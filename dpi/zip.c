/*
 * File: zip.c :)
 *
 * Copyright (C) 2000-2007 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

/*
 * Directory scanning is no longer streamed, but it gets sorted instead!
 * Directory entries on top, files next.
 * With new HTML layout.
 */

#include <ctype.h>           /* for isspace */
#include <errno.h>           /* for errno */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <netinet/in.h>

#include "../dpip/dpip.h"
#include "dpiutil.h"
#include "d_size.h"
#include "fileutil.h"

/*
 * Debugging macros
 */
#define _MSG(...)
#define MSG(...)  printf("[zip dpi]: " __VA_ARGS__)
#define _MSG_RAW(...)
#define MSG_RAW(...)  printf(__VA_ARGS__)

#define HIDE_DOTFILES TRUE

/*
 * Communication flags
 */
#define ZIP_AUTH_OK     1     /* Authentication done */
#define ZIP_READ        2     /* Waiting data */
#define ZIP_WRITE       4     /* Sending data */
#define ZIP_DONE        8     /* Operation done */
#define ZIP_ERR        16     /* Operation error */

typedef enum {
   st_start = 10,
   st_dpip,
   st_http,
   st_content,
   st_done,
   st_err
} FileState;

typedef struct {
   Dsh *sh;
   char *orig_url;
   char *archive_filename;
   char *inner_filename;
   FILE *zip;
   DilloDir *d_dir;
   FileState state;
   int err_code;
   int flags;
   int old_style;
} ClientInfo;

typedef struct {
   char *date;
   char *time;
   char *attr;
   char *size;
   char *compr;
   char *name;
} ZipFileInfo;

/*
 * Forward references
 */
static const char *Zip_content_type(const char *archive_filename, const char *inner_filename);

/*
 * Global variables
 */
static int DPIBYE = 0;
static int OLD_STYLE = 0;
/* A list for the clients we are serving */
static Dlist *Clients;
/* Set of filedescriptors we're working on */
fd_set read_set, write_set;

/*
 * Popen() an unzip process with the specified cmdline arguments
 */
FILE *Zip_open(const char *cmd_prefix, const char *archive_filename,
               const char *cmd_postfix, const char *inner_filename,
               const char *cmd_closing) {
   char cmd[1024] = { 0 };

   size_t len = strlen(cmd_prefix) + strlen(archive_filename) +
                (cmd_postfix ? strlen(cmd_postfix) : 0) +
                (inner_filename ? strlen(inner_filename) : 0) + 
                (cmd_closing ? strlen(cmd_closing) : 0);
	
   /* Check command length */
   if(len > sizeof(cmd) - 1) {
      MSG("error: zip_list(): cmd too long\n");
      return NULL;
   }

   /* Note: we assume the arguments here have already been validated
      by Zip_parse_path(), otherwise a shell command injection is possible */

   /* Assemble command */
   strncat(cmd, cmd_prefix, sizeof(cmd) - 1);
   strncat(cmd, archive_filename, sizeof(cmd) - strlen(cmd) - 1);
   if(cmd_postfix) strncat(cmd, cmd_postfix, sizeof(cmd) - strlen(cmd) - 1);
   if(inner_filename) strncat(cmd, inner_filename, sizeof(cmd) - strlen(cmd) - 1);
   if(cmd_closing) strncat(cmd, cmd_closing, sizeof(cmd) - strlen(cmd) - 1);

   return popen(cmd, "r");
}

/*
 * Close a popen()-ed file but handling EINTR
 */
static void Zip_close(FILE *fp)
{
   if (fp != NULL) pclose(fp);
}

/*
 * Open a zip listing process
 */
FILE *Zip_open_listing(const char *archive_filename) {
#if(ZIP_USE_7Z==1)
   return Zip_open("7z l \"", archive_filename, "\"", NULL, NULL);
#else
   return Zip_open("unzip -l \"", archive_filename, "\"", NULL, NULL);
#endif
}

/*
 * Open a zip extracting process
 */
FILE *Zip_open_extract(const char *archive_filename, const char *inner_filename) {
#if(ZIP_USE_7Z==1)
   return Zip_open("7z x -so \"", archive_filename, "\" \"",
                   inner_filename[0] == '/' ? inner_filename + 1 : inner_filename,
                   "\"");
#else
   return Zip_open("unzip -p \"", archive_filename, "\" \"",
                   inner_filename[0] == '/' ? inner_filename + 1 : inner_filename,
                   "\"");
#endif
}

/*
 * Parse a zip file listing line
 */
int Zip_parse_list_line(char *line, ZipFileInfo *zfi) {
   char *end;

#define ZIP_NEXT_LINE_TOKEN {            \
   line = strchr(line, ' ');             \
   if(!line) goto ZIP_PARSE_LINE_ERROR;  \
   *line='\0';                           \
   line++;                               \
   while(*line==' ') line++;             \
}

   while(*line==' ') line++;

#if(ZIP_USE_7Z==1)
   zfi->date = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->time = line;

   ZIP_NEXT_LINE_TOKEN;	
   zfi->attr = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->size = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->compr = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->name = line;
#else
   zfi->size = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->date = line;

   ZIP_NEXT_LINE_TOKEN;	
   zfi->time = line;

   ZIP_NEXT_LINE_TOKEN;
   zfi->name = line;
#endif
	
#undef ZIP_NEXT_LINE_TOKEN

   end = strchr(line, '\r');
   if(!end) end = strchr(line, '\n');
   if(end) *end='\0';

   return 1;
	
ZIP_PARSE_LINE_ERROR:
   MSG("error: zip_parse_list_line(): "
       "could not parse line: %s\n", zfi->date);
   return 0;
}

/*
 * Allocate a DilloDir structure, set safe values in it and sort the entries.
 */
static DilloDir *Zip_dillodir_fs_new(const char *archive_filename)
{
   struct stat sb;
   struct tm tm;
   FILE *zip;
   DilloDir *Ddir;
   char *full_path, *timestamp;
   ZipFileInfo zfi;
   int in_file_listing;
   char line[1024];

   if (!(zip = Zip_open_listing(archive_filename)))
      return NULL;

   Ddir = FileUtil_dillodir_new(archive_filename);

   in_file_listing = 0;

   memset(&tm, 0, sizeof(tm));

   while(fgets(line, sizeof(line) - 1, zip)) {
      if(!strncmp(line, "-----", 5)) {
         in_file_listing = !in_file_listing;
         continue;
      }

      if(in_file_listing) {
         if(Zip_parse_list_line(line, &zfi)) {
            /* Add file to archive listing */
            full_path = dStrconcat(archive_filename, "/", zfi.name, NULL);

            sb.st_size = atoi(zfi.size);
            sb.st_mode = S_IFREG;

            timestamp = dStrconcat(zfi.date, " ", zfi.time, NULL);
#if(ZIP_USE_7Z == 1)
            strptime(timestamp, "%Y-%m-%d %H:%M:%S", &tm);
#else
            strptime(timestamp, "%m-%d-%Y %H:%M", &tm);
#endif
            sb.st_mtime = mktime(&tm);
            free(timestamp);

            FileUtil_dillodir_add(Ddir, full_path, sb);
         }
      }
   }

   Zip_close(zip);

   /* sort the entries */
   dList_sort(Ddir->flist, (dCompareFunc)FileUtil_comp);

   return Ddir;
}

/*
 * Based on the extension, return the content_type for the file.
 * (if there's no extension, analyze the data and try to figure it out)
 */
static const char *Zip_content_type(const char *archive_filename, const char *inner_filename)
{
   FILE *zip;
   const char *ct;
   char buf[256];
   ssize_t buf_size;

   if (!(ct = FileUtil_ext(inner_filename))) {
      /* everything failed, let's analyze the data... */
      zip = Zip_open_extract(archive_filename, inner_filename);
      if (zip) {
         if ((buf_size = fread(buf, 1, 256, zip)) > 0) {
            ct = FileUtil_get_content_type_from_data(buf, (size_t)buf_size);
         }
         Zip_close(zip);
      }
   }
   _MSG("Zip_content_type: archive_filename=%s inner_filename=%s ct=%s\n", archive_filename, inner_filename, ct);
   return ct;
}

/*
 * Send the HTML directory page in HTTP.
 */
static void Zip_send_dir(ClientInfo *client)
{
   int n;
   char *d_cmd;
   const char *filecont;
   FileInfo *finfo;
   DilloDir *Ddir = client->d_dir;

   if (client->state == st_start) {
      /* Send DPI command */
      d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page",
                               client->orig_url);
      a_Dpip_dsh_write_str(client->sh, 1, d_cmd);
      dFree(d_cmd);
      client->state = st_dpip;

   } else if (client->state == st_dpip) {
      /* send HTTP header and HTML top part */

      /* Send page title */
      FileUtil_print_page_header(client->sh, "zip", Ddir->dirname, client->old_style);

      /* Output the parent directory */
      FileUtil_print_parent_dir(client->sh, "file", Ddir->dirname); /* handled with file dpi */

      /* HTML style toggle */
      a_Dpip_dsh_write_str(client->sh, 0,
         "&nbsp;&nbsp;<a href='dpi:/zip/toggle'>%</a>\n");
	 
      /* Output the file listing table */
      FileUtil_print_table_header(client->sh, dList_length(Ddir->flist), client->old_style);

      client->state = st_http;

   } else if (client->state == st_http) {
      /* send directories as HTML contents */
      for (n = 0; n < dList_length(Ddir->flist); ++n) {
         finfo = dList_nth_data(Ddir->flist,n);
         filecont = Zip_content_type(client->archive_filename, finfo->filename);
         FileUtil_print_info(client->sh, finfo, n+1, filecont, client->old_style);
      }

      FileUtil_print_table_footer(client->sh, dList_length(Ddir->flist), client->old_style);

      FileUtil_print_page_footer(client->sh, client->old_style);

      client->state = st_content;
      client->flags |= ZIP_DONE;
   }
}

/*
 * Send an error page
 */
static void Zip_prepare_send_error_page(ClientInfo *client, int res,
                                         const char *orig_url)
{
   client->state = st_err;
   client->err_code = res;
   client->orig_url = dStrdup(orig_url);
   client->flags &= ~ZIP_READ;
   client->flags |= ZIP_WRITE;
}

/*
 * Send an error page
 */
static void Zip_send_error_page(ClientInfo *client)
{
   const char *status;
   char *d_cmd;
   Dstr *body = dStr_sized_new(128);

   if (client->err_code == EACCES) {
      status = "403 Forbidden";
   } else if (client->err_code == ENOENT) {
      status = "404 Not Found";
   } else {
      /* good enough */
      status = "500 Internal Server Error";
   }
   dStr_append(body, status);
   dStr_append(body, "\n");
   dStr_append(body, dStrerror(client->err_code));

   /* Send DPI command */
   d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page",
                            client->orig_url);
   a_Dpip_dsh_write_str(client->sh, 0, d_cmd);
   dFree(d_cmd);

   a_Dpip_dsh_printf(client->sh, 0,
                     "HTTP/1.1 %s\r\n"
                     "Content-Type: text/plain\r\n"
                     "Content-Length: %d\r\n"
                     "\r\n"
                     "%s",
                     status, body->len, body->str);
   dStr_free(body, TRUE);

   client->flags |= ZIP_DONE;
}

/*
 * Scan the directory, sort and prepare to send it enclosed in HTTP.
 */
static int Zip_prepare_send_dir(ClientInfo *client,
                                 const char *archive_filename, const char *orig_url)
{
   Dstr *ds_dirname;
   DilloDir *Ddir;

   /* Let's make sure this directory url has a trailing slash */
   ds_dirname = dStr_new(archive_filename);
   if (ds_dirname->str[ds_dirname->len - 1] != '/')
      dStr_append(ds_dirname, "/");

   /* Let's get a structure ready for transfer */
   Ddir = Zip_dillodir_fs_new(archive_filename);
   dStr_free(ds_dirname, TRUE);
   if (Ddir) {
      /* looks ok, set things accordingly */
      client->orig_url = dStrdup(orig_url);
      client->d_dir = Ddir;
      client->state = st_start;
      client->flags &= ~ZIP_READ;
      client->flags |= ZIP_WRITE;
      return 0;
   } else
      return EACCES;
}

/*
 * Prepare to send HTTP headers and then the file itself.
 */
static int Zip_prepare_send_file(ClientInfo *client,
                                  const char *archive_filename,
                                  const char *inner_filename,
                                  const char *orig_url)
{
   FILE *zip;
   int res = -1;

   if (!(zip = Zip_open_extract(archive_filename, inner_filename))) {
      /* prepare an error message */
      res = errno;
   } else {
      /* looks ok, set things accordingly */
      client->zip = zip;
      client->d_dir = NULL;
      client->state = st_start;
      client->orig_url = dStrdup(orig_url);
      client->flags &= ~ZIP_READ;
      client->flags |= ZIP_WRITE;
      res = 0;
   }
   return res;
}

/*
 * Parse path to get archive filename and inner filename (if any)
 */
static int Zip_parse_path(char *path, char **archive_filename,
                            char **inner_filename) {
   char c;
   int i, len=(int) strlen(path);
	
   struct stat sb;
	
   /* Exclude some dangerous chars in path,
      to avoid shell commands injection on popen() */
   for(i=0; i<len; i++) {
      c=path[i];
         if(c == '"' || c == '$' || c == '`' || c == '#' ||
            c == '<' || c == '>' || c == '|' || c == '\\') {
            MSG("error: zip_parse_path(): invalid chars in path\n");
            return 0;
         }
   }

   /* Check each piece of the path to find the first existing file
      (it should be the archive file) */
   for(i=len; i>=0; i--) {
      if(path[i] != '/' && path[i] != '\0') continue;

      c = path[i];
      path[i]='\0';

      if(!stat(path, &sb)) {
         if(S_ISDIR(sb.st_mode)) {
            MSG("error: zip_parse_path(): directory found instead of file\n");
            return 0;
         }
      
         *archive_filename = path;
         *inner_filename = i<len-1 ? path+i+1 : NULL;
         MSG("archive=%s, inner=%s\n", *archive_filename, *inner_filename);
         return 1;
      }

      path[i]=c;
   }
	
   return 0;
}

/*
 * Try to stat the file and determine if it's readable.
 */
static void Zip_get(ClientInfo *client, char *filename,
                     const char *orig_url)
{
   int res;
   char *archive_filename, *inner_filename;

   if (!Zip_parse_path(filename, &archive_filename, &inner_filename)) {
      /* parse and stat failed, prepare a file-not-found error. */
      res = ENOENT;
   } else {
      client->archive_filename = dStrdup(archive_filename);
      client->inner_filename = dStrdup(inner_filename);

      if (!inner_filename) {
         /* set up for reading archive listing */
         res = Zip_prepare_send_dir(client, archive_filename, orig_url);
      } else {
         /* set up for reading an inner archive file */
         res = Zip_prepare_send_file(client, archive_filename,
                                     inner_filename, orig_url);
      }
   }
   if (res != 0) {
      Zip_prepare_send_error_page(client, res, orig_url);
   }
}

/*
 * Send HTTP headers and then the file itself.
 */
static int Zip_send_file(ClientInfo *client)
{
//#define LBUF 1
#define LBUF 16*1024

   const char *ct;
   const char *unknown_type = "application/octet-stream";
   char buf[LBUF], *d_cmd, *name;
   int st, st2, namelen;
   bool_t gzipped = FALSE;

   if (client->state == st_start) {
      /* Send DPI command */
      d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page",
                               client->orig_url);
      a_Dpip_dsh_write_str(client->sh, 1, d_cmd);
      dFree(d_cmd);
      client->state = st_dpip;

   } else if (client->state == st_dpip) {
      /* send HTTP header */

      /* Check for gzipped file */
      namelen = strlen(client->inner_filename);
      if (namelen > 3 &&
          !dStrAsciiCasecmp(client->inner_filename + namelen - 3, ".gz")) {
         gzipped = TRUE;
         namelen -= 3;
      }
      /* Content-Type info is based on filename extension (with ".gz" removed).
       * If there's no known extension, perform data sniffing.
       * If this doesn't lead to a conclusion, use "application/octet-stream".
       */
      name = dStrndup(client->inner_filename, namelen);
      if (!(ct = Zip_content_type(client->archive_filename, name)))
         ct = unknown_type;
      dFree(name);

      /* Send HTTP headers */
      a_Dpip_dsh_write_str(client->sh, 0, "HTTP/1.1 200 OK\r\n");
      if (gzipped) {
         a_Dpip_dsh_write_str(client->sh, 0, "Content-Encoding: gzip\r\n");
      }
      if (!gzipped || strcmp(ct, unknown_type)) {
         a_Dpip_dsh_printf(client->sh, 0, "Content-Type: %s\r\n", ct);
      } else {
         /* If we don't know type for gzipped data, let dillo figure it out. */
      }
      /* Todo: determine content length */
      /*
      a_Dpip_dsh_printf(client->sh, 1,
                        "Content-Length: %ld\r\n"
                        client->file_sz); */
      a_Dpip_dsh_printf(client->sh, 1, "\r\n");
      client->state = st_http;

   } else if (client->state == st_http) {
      /* Send body -- raw file contents */
      if ((st = a_Dpip_dsh_tryflush(client->sh)) < 0) {
         client->flags |= (st == -3) ? ZIP_ERR : 0;
      } else {
         /* no pending data, let's send new data */
         do {
            st2 = fread(buf, 1, LBUF, client->zip);
         } while (!feof(client->zip));
         if (st2 < 0) {
            MSG("\nERROR while reading from file '%s': %s\n\n",
                client->inner_filename, dStrerror(errno));
            client->flags |= ZIP_ERR;
         } else if (st2 == 0) {
            client->state = st_content;
            client->flags |= ZIP_DONE;
         } else {
            /* ok to write */
            st = a_Dpip_dsh_trywrite(client->sh, buf, st2);
            client->flags |= (st == -3) ? ZIP_ERR : 0;
         }
      }
   }

   return 0;
}

/*
 * Set the style flag and ask for a reload, so it shows immediately.
 */
static void Zip_toggle_html_style(ClientInfo *client)
{
   char *d_cmd;

   OLD_STYLE = !OLD_STYLE;
   d_cmd = a_Dpip_build_cmd("cmd=%s", "reload_request");
   a_Dpip_dsh_write_str(client->sh, 1, d_cmd);
   dFree(d_cmd);
}

/*
 * Perform any necessary cleanups upon abnormal termination
 */
static void termination_handler(int signum)
{
  MSG("\nexit(signum), signum=%d\n\n", signum);
  exit(signum);
}


/* Client handling ----------------------------------------------------------*/

/*
 * Add a new client to the list.
 */
static ClientInfo *Zip_add_client(int sock_fd)
{
   ClientInfo *new_client;

   new_client = dNew(ClientInfo, 1);
   new_client->sh = a_Dpip_dsh_new(sock_fd, sock_fd, 8*1024);
   new_client->orig_url = NULL;
   new_client->archive_filename = NULL;
   new_client->inner_filename = NULL;
   new_client->zip = NULL;
   new_client->d_dir = NULL;
   new_client->state = 0;
   new_client->err_code = 0;
   new_client->flags = ZIP_READ;
   new_client->old_style = OLD_STYLE;

   dList_append(Clients, new_client);
   return new_client;
}

/*
 * Remove a client from the list.
 */
static void Zip_remove_client(ClientInfo *client)
{
   dList_remove(Clients, (void *)client);

   _MSG("Closing Socket Handler\n");
   a_Dpip_dsh_close(client->sh);
   a_Dpip_dsh_free(client->sh);
   Zip_close(client->zip);
   dFree(client->orig_url);
   if (client->archive_filename) dFree(client->archive_filename);
   if (client->inner_filename) dFree(client->inner_filename);
   FileUtil_dillodir_free(client->d_dir);

   dFree(client);
}

/*
 * Serve this client.
 */
static void Zip_serve_client(void *data, int f_write)
{
   char *dpip_tag = NULL, *cmd = NULL, *url = NULL, *path;
   ClientInfo *client = data;
   int st;

   while (1) {
      _MSG("Zip_serve_client %p, flags=%d state=%d\n",
          client, client->flags, client->state);
      if (client->flags & (ZIP_DONE | ZIP_ERR))
         break;
      if (client->flags & ZIP_READ) {
         dpip_tag = a_Dpip_dsh_read_token(client->sh, 0);
         _MSG("dpip_tag={%s}\n", dpip_tag);
         if (!dpip_tag)
            break;
      }

      if (client->flags & ZIP_READ) {
         if (!(client->flags & ZIP_AUTH_OK)) {
            /* Authenticate our client... */
            st = a_Dpip_check_auth(dpip_tag);
            _MSG("a_Dpip_check_auth returned %d\n", st);
            client->flags |= (st == 1) ? ZIP_AUTH_OK : ZIP_ERR;
         } else {
            /* Get file request */
            cmd = a_Dpip_get_attr(dpip_tag, "cmd");
            url = a_Dpip_get_attr(dpip_tag, "url");
            path = FileUtil_normalize_path("zip", url);
            if (cmd) {
               if (strcmp(cmd, "DpiBye") == 0) {
                  DPIBYE = 1;
                  MSG("(pid %d): Got DpiBye.\n", (int)getpid());
                  client->flags |= ZIP_DONE;
               } else if (url && dStrnAsciiCasecmp(url, "dpi:", 4) == 0 &&
                          strcmp(url+4, "/zip/toggle") == 0) {
                  Zip_toggle_html_style(client);
               } else if (path) {
                  Zip_get(client, path, url);
               } else {
                  client->flags |= ZIP_ERR;
                  MSG("ERROR: URL was %s\n", url);
               }
            }
            dFree(path);
            dFree(url);
            dFree(cmd);
            dFree(dpip_tag);
            break;
         }
         dFree(dpip_tag);

      } else if (f_write) {
         /* send our answer */
         if (client->state == st_err)
            Zip_send_error_page(client);
         else if (client->d_dir)
            Zip_send_dir(client);
         else
            Zip_send_file(client);
         break;
      }
   } /*while*/

   client->flags |= (client->sh->status & DPIP_ERROR) ? ZIP_ERR : 0;
   client->flags |= (client->sh->status & DPIP_EOF) ? ZIP_DONE : 0;
}

/*
 * Serve the client queue.
 */
static void Zip_serve_clients()
{
   int i, f_read, f_write;
   ClientInfo *client;

   for (i = 0; (client = dList_nth_data(Clients, i)); ++i) {
      f_read = FD_ISSET(client->sh->fd_in, &read_set);
      f_write = FD_ISSET(client->sh->fd_out, &write_set);
      if (!f_read && !f_write)
         continue;
      Zip_serve_client(client, f_write);
      if (client->flags & (ZIP_DONE | ZIP_ERR)) {
         Zip_remove_client(client);
         --i;
      }
   }
}

/* --------------------------------------------------------------------------*/

/*
 * Check the fd sets for activity, with a max timeout.
 * return value: 0 if timeout, 1 if input available, -1 if error.
 */
static int Zip_check_fds(uint_t seconds)
{
   int i, st;
   ClientInfo *client;
   struct timeval timeout;

   /* initialize observed file descriptors */
   FD_ZERO (&read_set);
   FD_ZERO (&write_set);
   FD_SET (STDIN_FILENO, &read_set);
   for (i = 0; (client = dList_nth_data(Clients, i)); ++i) {
      if (client->flags & ZIP_READ)
         FD_SET (client->sh->fd_in, &read_set);
      if (client->flags & ZIP_WRITE)
         FD_SET (client->sh->fd_out, &write_set);
   }
   _MSG("Watching %d fds\n", dList_length(Clients) + 1);

   /* Initialize the timeout data structure. */
   timeout.tv_sec = seconds;
   timeout.tv_usec = 0;

   do {
      st = select(FD_SETSIZE, &read_set, &write_set, NULL, &timeout);
   } while (st == -1 && errno == EINTR);
/*
   MSG_RAW(" (%d%s%s)", STDIN_FILENO,
           FD_ISSET(STDIN_FILENO, &read_set) ? "R" : "",
           FD_ISSET(STDIN_FILENO, &write_set) ? "W" : "");
   for (i = 0; (client = dList_nth_data(Clients, i)); ++i) {
      MSG_RAW(" (%d%s%s)", client->sh->fd_in,
              FD_ISSET(client->sh->fd_in, &read_set) ? "R" : "",
              FD_ISSET(client->sh->fd_out, &write_set) ? "W" : "");
   }
   MSG_RAW("\n");
*/
   return st;
}


int main(void)
{
   struct sockaddr_in sin;
   socklen_t sin_sz;
   int sock_fd, c_st, st = 1;

   /* Arrange the cleanup function for abnormal terminations */
   if (signal (SIGINT, termination_handler) == SIG_IGN)
     signal (SIGINT, SIG_IGN);
   if (signal (SIGHUP, termination_handler) == SIG_IGN)
     signal (SIGHUP, SIG_IGN);
   if (signal (SIGTERM, termination_handler) == SIG_IGN)
     signal (SIGTERM, SIG_IGN);

   MSG("(v.2) accepting connections...\n");
   //sleep(20);

   /* initialize observed file descriptors */
   FD_ZERO (&read_set);
   FD_ZERO (&write_set);
   FD_SET (STDIN_FILENO, &read_set);

   /* Set STDIN socket nonblocking (to ensure accept() never blocks) */
   fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK | fcntl(STDIN_FILENO, F_GETFL));

   /* initialize Clients list */
   Clients = dList_new(512);

   /* some OSes may need this... */
   sin_sz = sizeof(sin);

   /* start the service loop */
   while (!DPIBYE) {
      /* wait for activity */
      do {
         c_st = Zip_check_fds(10);
      } while (c_st == 0 && !DPIBYE);
      if (c_st < 0) {
         MSG(" select() %s\n", dStrerror(errno));
         break;
      }
      if (DPIBYE)
         break;

      if (FD_ISSET(STDIN_FILENO, &read_set)) {
         /* accept the incoming connection */
         do {
            sock_fd = accept(STDIN_FILENO, (struct sockaddr *)&sin, &sin_sz);
         } while (sock_fd < 0 && errno == EINTR);
         if (sock_fd == -1) {
            if (errno == EAGAIN)
               continue;
            MSG(" accept() %s\n", dStrerror(errno));
            break;
         } else {
            _MSG(" accept() fd=%d\n", sock_fd);
            /* Set nonblocking */
            fcntl(sock_fd, F_SETFL, O_NONBLOCK | fcntl(sock_fd, F_GETFL));
            /* Create and initialize a new client */
            Zip_add_client(sock_fd);
         }
         continue;
      }

      Zip_serve_clients();
   }

   if (DPIBYE)
      st = 0;
   return st;
}

