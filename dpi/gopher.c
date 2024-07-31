/*
 * Dpi for Gopher.
 *
 *
 *
 *                            W A R N I N G
 *
 * One of the important things to have in mind is about whether
 * unix domain sockets (UDS) are secure for gopher. I mean, root can always
 * snoop on sockets (regardless of permissions) so he'd be able to "see" all
 * the traffic. OTOH, if someone has root access on a machine he can do
 * anything, and that includes modifying the binaries, peeking-up in
 * memory space, installing a key-grabber, ...
 *
 *
 * Copyright 2003, 2004 Jorge Arellano Cid <jcid@dillo.org>
 * Copyright 2004 Garrett Kajmowicz <gkajmowi@tbaytel.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */

/*
 * TODO: a lot of things, this is just a bare bones example.
 *
 * For instance:
 * - Certificate authentication (asking the user in case it can't be verified)
 * - Certificate management.
 * - Session caching ...
 *
 */

#include <config.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/stat.h>

#include "../dpip/dpip.h"
#include "dpiutil.h"

/*
 * Debugging macros
 */
#define SILENT 0
#define _MSG(...)
#if SILENT
 #define MSG(...)
#else
 #define MSG(...)  fprintf(stderr, "[gopher dpi]: " __VA_ARGS__)
#endif

char servername[1024];

/*---------------------------------------------------------------------------*/
/*
 * Global variables
 */
static char *root_url = NULL;  /*Holds the URL we are connecting to*/
static Dsh *sh;

/*
 * The following function attempts to open up a connection to the
 * remote server and return the file descriptor number of the
 * socket.  Returns -1 in the event of an error
 */
static int get_network_connection(char * url)
{
   struct sockaddr_in address;
   struct hostent *hp;

   int s;
   int url_offset = 0;
   int portnum = 70;
   uint_t url_look_up_length = 0;
   char * url_look_up = NULL;

   MSG("{get_network_connection}\n");

   /*Determine how much of url we chop off as unneeded*/
   if (dStrnAsciiCasecmp(url, "gopher://", 9) == 0){
      url_offset = 9;
   }

   /*Find end of URL*/

   if (strpbrk(url+url_offset, ":/") != NULL){
      url_look_up_length = strpbrk(url+url_offset, ":/") - (url+url_offset);
      url_look_up = dStrndup(url+url_offset, url_look_up_length);

      /*Check for port number*/
      if (strchr(url+url_offset, ':') ==
          (url + url_offset + url_look_up_length)){
         portnum = strtol(url + url_offset + url_look_up_length + 1, NULL, 10);
      }
   } else {
      url_look_up = url + url_offset;
   }

   root_url = dStrdup(url_look_up);

   memset(servername, 0, sizeof(servername));
   strncpy(servername, root_url, sizeof(servername) - 1);
   
   hp=gethostbyname(url_look_up);

   /*url_look_uip no longer needed, so free if necessary*/
   if (url_look_up_length != 0){
      dFree(url_look_up);
   }

   if (hp == NULL){
      MSG("gethostbyname() failed\n");
      return -1;
   }

   memset(&address,0,sizeof(address));
   memcpy((char *)&address.sin_addr, hp->h_addr, (size_t)hp->h_length);
   address.sin_family=hp->h_addrtype;
   address.sin_port= htons((u_short)portnum);

   s = socket(hp->h_addrtype, SOCK_STREAM, 0);
   if (connect(s, (struct sockaddr *)&address, sizeof(address)) != 0){
      dClose(s);
      s = -1;
      MSG("errno: %i\n", errno);
   }
   return s;
}

/*
 * This function parses the Gopher URL format used by dillo
 */
static int gopher_parse_url(char *url, char *type, int *url_proto_len,
                            char *query, size_t max_query_size)
{
   *type = '1';
   *url_proto_len = strlen("gopher://");

   /* Example of Gopher URL to parse:
      gopher://1::gopher.club:70/phlogs/ */

   if(strlen(url) + 10 >= max_query_size) {
      MSG("Error: URL too long\n");
      return 0;
   }

   /* Isolate Gopher query */
   if(!strchr(url+strlen("gopher://"), '/')) {
      /* add trailing backslash if missing */
      snprintf(query, max_query_size, "%s/\r\n", url);
   } else {
      snprintf(query, max_query_size, "%s\r\n", url);
   }

   /* Parse Gopher type (if present) */
   if(strstr(query, "::")) {
      *type = strstr(query, "::")[-1];
      *url_proto_len += 3;
   }

   /* Parse query input (if present) */
   if(strstr(query, "?q=")) { // convert extra piece to tab "?q=" -> "\t"
      char *repl;

      for(repl=query; *repl != '?'; repl++) {}
      *repl = '\t';
      repl++;
      for(; *repl; repl++) {
         *repl = *(repl+2);
      }
     
   }
   
   return 1;
}

/*
 * This function opens a connection to a remote server
 */
static int gopher_open_connection(char *url, int url_proto_len,
                                  char *proxy_url, char *proxy_connect)
{
   /* The following variable will be set to 1 in the event of
    * an error and skip any further processing
    */
   int exit_error = 0;
   int network_socket = -1;
   
   /* Open the socket */
   network_socket = get_network_connection(proxy_url ? proxy_url : (url + url_proto_len));
   if (network_socket<0){
      MSG("Network socket create error\n");
      exit_error = 1;
   }
   
   /* Setup the proxy */
   if (exit_error == 0 && proxy_connect != NULL) {
      ssize_t St;
      const char *p = proxy_connect;
      int writelen = strlen(proxy_connect);

      while (writelen > 0) {
         St = write(network_socket, p, writelen);
         if (St < 0) {
            /* Error */
            if (errno != EINTR) {
               MSG("Error writing to proxy.\n");
               exit_error = 1;
               break;
            }
         } else {
            p += St;
            writelen -= St;
         }
      }
      if (exit_error == 0) {
         const size_t buflen = 200;
         char buf[buflen];
         Dstr *reply = dStr_new("");

         while (1) {
            St = read(network_socket, buf, buflen);
            if (St > 0) {
               dStr_append_l(reply, buf, St);
               if (strstr(reply->str, "\r\n\r\n")) {
                  /* have whole reply header */
                  if (reply->len >= 12 && reply->str[9] == '2') {
                     /* e.g. "HTTP/1.1 200 Connection established[...]" */
                     MSG("CONNECT through proxy succeeded.\n");
                  } else {
                     /* TODO: send reply body to dillo */
                     exit_error = 1;
                     MSG("CONNECT through proxy failed.\n");
                  }
                  break;
               }
            } else if (St < 0) {
               if (errno != EINTR) {
                  exit_error = 1;
                  MSG("Error reading from proxy.\n");
                  break;
               }
            }
         }
         dStr_free(reply, 1);
      }
   }
   
   return exit_error ? -1 : network_socket;
}

static void gopher_main(void)
{
   /* The following variable will be set to 1 in the event of
    * an error and skip any further processing
    */
   int exit_error = 0;

   char *dpip_tag = NULL, *cmd = NULL, *url = NULL,
        *proxy_url = NULL, *proxy_connect = NULL;
   char buf[4096], buf2[4096*4];
   int ret = 0;
   int network_socket = -1;

   char gopher_query[2048];
   char gopher_type = '1';
   int url_proto_len = 0;

   MSG("{In gopher.filter.dpi}\n");

   /*Get the network address and command to be used*/
   dpip_tag = a_Dpip_dsh_read_token(sh, 1);

   MSG("dpip_tag = %s\n", dpip_tag);
       
   cmd = a_Dpip_get_attr(dpip_tag, "cmd");
   proxy_url = a_Dpip_get_attr(dpip_tag, "proxy_url");
   proxy_connect =
      a_Dpip_get_attr(dpip_tag, "proxy_connect");
   url = a_Dpip_get_attr(dpip_tag, "url");
   
   if (!url) {
      MSG("***Value of url is NULL"
          " - cannot continue\n");
      exit_error = 1;
   }

   /* Parse Gopher URL */
   if (exit_error == 0){
      ret = gopher_parse_url(url, &gopher_type, &url_proto_len,
                             gopher_query, sizeof(gopher_query));

      if (ret == 0){
         MSG("Error parsing URL\n");
         exit_error = 1;
      }
   }

   /* Open Gopher connection */
   if (exit_error == 0){
      network_socket = gopher_open_connection(url, url_proto_len,
                                              proxy_url, proxy_connect);

      if (network_socket<0){
         MSG("Error opening the connection\n");
         exit_error = 1;
      }
   }

   /* Send Gopher query */
   if (exit_error == 0){
      char *query_piece;

      fprintf(stderr, "Gopher Request = %s\n", gopher_query);

      query_piece = strchr(gopher_query+url_proto_len, '/');
      write(network_socket, query_piece, (int)strlen(query_piece));
   }

   /* Read Gopher response */
   if (exit_error == 0) {
      char *d_cmd;

      /*Send dpi command*/
      d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page", url);
      a_Dpip_dsh_write_str(sh, 1, d_cmd);
      dFree(d_cmd);

      /*Send HTTP OK header*/

      switch (gopher_type) {

      case '0':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/plain; charset=UTF-8\r\n\r\n");
         break;

      case 'g':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: image/gif\r\n\r\n");
         break;

      case 'p':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: image/png\r\n\r\n");
         break;

      case 'h':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/html; charset=UTF-8\r\n\r\n");
         break;

      case 'X':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/xml; charset=UTF-8\r\n\r\n");
         break;

      case '1':
      case '7':
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/gopher; charset=UTF-8\r\n\r\n");
         break;

      default:
         snprintf(buf2, sizeof(buf2),
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/octet-stream\r\n\r\n");
      }
      
      a_Dpip_dsh_write(sh, 1, buf2, strlen(buf2));

      /*Send data*/

      while ((ret = read(network_socket, buf, 4096)) > 0 ){
         a_Dpip_dsh_write(sh, 1, buf, (size_t)ret);
      }

   }

   /*Begin cleanup of all resources used*/
   dFree(dpip_tag);
   dFree(cmd);
   dFree(url);
   dFree(proxy_url);
   dFree(proxy_connect);

   if (network_socket != -1){
      dClose(network_socket);
      network_socket = -1;
   }

}

/*---------------------------------------------------------------------------*/

int download(char *url, char *output_filename,
             char *proxy_url, char *proxy_connect, int check_cert) {
   /* The following variable will be set to 1 in the event of
    * an error and skip any further processing
    */
   int exit_error = 0;

   char buf[4096];
   int ret = 0;
   int network_socket = -1;

   char gopher_query[2048];
   char gopher_type = '1';
   int url_proto_len = 0;

   FILE *outfile = NULL;

   /* Open output file */
   if((outfile = fopen(output_filename, "w")) == NULL) {
      MSG("Error opening output file\n");
      exit_error = 1;
   }

   /* Parse Gopher URL */
   if (exit_error == 0){
      ret = gopher_parse_url(url, &gopher_type, &url_proto_len,
                             gopher_query, sizeof(gopher_query));

      if (ret == 0){
         MSG("Error parsing URL\n");
         exit_error = 1;
      }
   }

   /* Open Gopher connection */
   if (exit_error == 0){
      network_socket = gopher_open_connection(url, url_proto_len,
                                              proxy_url, proxy_connect);

      if (network_socket<0){
         MSG("Error opening the connection\n");
         exit_error = 1;
      }
   }

   /* Send Gopher query */
   if (exit_error == 0){
      char *query_piece;

      fprintf(stderr, "Gopher Request = %s\n", gopher_query);

      query_piece = strchr(gopher_query+url_proto_len, '/');
      write(network_socket, query_piece, (int)strlen(query_piece));
   }

   /* Read Gopher response */
   if (exit_error == 0) {

      /*Send data*/

      while ((ret = read(network_socket, buf, 4096)) > 0 ){
         fwrite(buf, 1, (size_t)ret, outfile);
      }

   }

   /* Begin cleanup of all resources used */
   if (network_socket != -1){
      dClose(network_socket);
      network_socket = -1;
   }
   if(outfile != NULL){
      fclose(outfile);
      outfile = NULL;
   }

   return 0;
}

int main(int argc, char *argv[])
{
   char *dpip_tag;
   int i = 0;
   
   if(argc > 1) {
      /* Downloader behaviour */

      for(i=1; i<argc; i++) {
         if(!strncmp(argv[i], "gopher://", sizeof("gopher://")-1)) break;
      }

      if(i+1<argc) {

         /* Found URL to download */
         return download(argv[i], argv[i+1], NULL, NULL, 0);

      } else {
         fprintf(stderr, "usage: %s gopher_url output_filename\n", argv[0]);
         return 1;
      }
      
   } else {
      /* Standard DPI behaviour */

      /* Initialize the SockHandler for this filter dpi */
      sh = a_Dpip_dsh_new(STDIN_FILENO, STDOUT_FILENO, 8*1024);

      /* Authenticate our client... */
      if (!(dpip_tag = a_Dpip_dsh_read_token(sh, 1)) ||
          a_Dpip_check_auth(dpip_tag) < 0) {
         MSG("can't authenticate request: %s\n", dStrerror(errno));
         a_Dpip_dsh_close(sh);
         return 1;
      }
      dFree(dpip_tag);

      gopher_main();

      /* Finish the SockHandler */
      a_Dpip_dsh_close(sh);
      a_Dpip_dsh_free(sh);

      dFree(root_url);

      MSG("{ exiting gopher.dpi}\n");

      return 0;
   }
}

