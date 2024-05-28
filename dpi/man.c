/*
 * File: man.c :)
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
#define MSG(...)  printf("[man dpi]: " __VA_ARGS__)
#define _MSG_RAW(...)
#define MSG_RAW(...)  printf(__VA_ARGS__)

#define HIDE_DOTFILES TRUE

/*
 * Communication flags
 */
#define MAN_AUTH_OK     1     /* Authentication done */
#define MAN_READ        2     /* Waiting data */
#define MAN_WRITE       4     /* Sending data */
#define MAN_DONE        8     /* Operation done */
#define MAN_ERR        16     /* Operation error */

typedef enum {
   st_start = 10,
   st_dpip,
   st_http,
   st_pre_content,
   st_content,
   st_post_content,
   st_done,
   st_err
} FileState;

typedef enum {
   fs_start = 10,
   fs_in_see_also,
} FormatState;

typedef struct {
   Dsh *sh;
   char *orig_url;
   char *manpage;
   FILE *man;
   FileState state;
   FormatState fstate;
   int err_code;
   int flags;
   int old_style;
} ClientInfo;

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
 * Open a pipe to a man process with the specified cmdline arguments
 */
FILE *Man_open(const char *args, const char *manpage) {
   int pid1 = 0, pid2 = 0;
   int pipe_1[2];
   int pipe_2[2];
   int pipe_3[2];

   if (pipe(pipe_1) < 0) {
      return NULL;
   }

   if (pipe(pipe_2) < 0) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      return NULL;
   }

   if (pipe(pipe_3) < 0) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      close(pipe_2[0]);
      close(pipe_2[1]);
      return NULL;
   }

   pid1 = fork();

   if (pid1 == -1) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      close(pipe_2[0]);
      close(pipe_2[1]);
      close(pipe_3[0]);
      close(pipe_3[1]);
      return NULL;
   }

   if(pid1 != 0) pid2 = fork();

   if (pid2 == -1) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      close(pipe_2[0]);
      close(pipe_2[1]);
      close(pipe_3[0]);
      close(pipe_3[1]);
      return NULL;
   }

   if (pid1 == 0) {
      // child 1

      close(fileno(stdin));
      close(fileno(stdout));
      
      close(pipe_1[1]);
      close(pipe_2[0]);

      close(pipe_3[1]);
      close(pipe_3[0]);

      // redirect stdout
      if (pipe_2[1] != STDOUT_FILENO) {
	dup2(pipe_2[1], STDOUT_FILENO);
	close(pipe_2[1]);
      }

      // redirect stdin
      if (pipe_1[0] != STDIN_FILENO) {
	dup2(pipe_1[0], STDIN_FILENO);
	close(pipe_1[0]);
      }

      // launch cmd
      const char* argp[] = {"man", args, manpage, NULL};
      execvp("man", (char**) argp);

      // if the function returns an error has occurred
      perror("execvp");
      exit(127);
   }

   if (pid2 == 0) {
      // child 2

      close(fileno(stdin));
      close(fileno(stdout));

      close(pipe_1[1]);
      close(pipe_1[0]);      

      close(pipe_2[1]);
      close(pipe_3[0]);

      // redirect stdout
      if (pipe_3[1] != STDOUT_FILENO) {
	dup2(pipe_3[1], STDOUT_FILENO);
	close(pipe_3[1]);
      }

      // redirect stdin
      if (pipe_2[0] != STDIN_FILENO) {
	dup2(pipe_2[0], STDIN_FILENO);
	close(pipe_2[0]);
      }

      // launch cmd
      const char* argp[] = {"col", "-b", NULL};
      execvp("col", (char**) argp);

      // if the function returns an error has occurred
      perror("execvp");
      exit(127);
   }

   // parent
   
   close(pipe_1[0]);
   close(pipe_2[1]);
   close(pipe_2[0]);
   close(pipe_1[1]); // do not write to child1 stdin
   close(pipe_3[1]);
   
   return fdopen(pipe_3[0], "r");
}

/*
 * Close an open pipe to man process but handling EINTR
 */
static void Man_close(FILE *fp)
{
   if (fp != NULL) fclose(fp);
}

/*
 * Send an error page
 */
static void Man_prepare_send_error_page(ClientInfo *client, int res,
                                         const char *orig_url)
{
   client->state = st_err;
   client->err_code = res;
   client->orig_url = dStrdup(orig_url);
   client->flags &= ~MAN_READ;
   client->flags |= MAN_WRITE;
}

/*
 * Send an error page
 */
static void Man_send_error_page(ClientInfo *client)
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

   client->flags |= MAN_DONE;
}

/*
 * Prepare to send HTTP headers and then the file itself.
 */
static int Man_prepare_send_file(ClientInfo *client,
                                  const char *manpage,
                                  const char *orig_url)
{
   FILE *man;
   int res = -1;

   if (!(man = Man_open("--", manpage))) {
      /* prepare an error message */
      res = errno;
   } else {
      /* looks ok, set things accordingly */
      client->man = man;
      client->state = st_start;
      client->fstate = fs_start;
      client->orig_url = dStrdup(orig_url);
      client->flags &= ~MAN_READ;
      client->flags |= MAN_WRITE;
      res = 0;
   }
   return res;
}

/*
 * Parse path to get archive filename and inner filename (if any)
 */
static int Man_parse_path(char *path, char **manpage) {
   char c;
   int i, len=(int) strlen(path);
	
   struct stat sb;
	
   /* Exclude some dangerous chars in path,
      to avoid shell commands injection on popen() */
   for(i=0; i<len; i++) {
      c=path[i];
         if(c == '"' || c == '$' || c == '`' || c == '#' ||
            c == '<' || c == '>' || c == '|' || c == '\\') {
            MSG("error: man_parse_path(): invalid chars in path\n");
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
            MSG("error: man_parse_path(): directory found instead of file\n");
            return 0;
         }
      
         *manpage = path;
         MSG("manpage=%s\n", *manpage);
         return 1;
      }

      path[i]=c;
   }
	
   return 0;
}

/*
 * Try to stat the file and determine if it's readable.
 */
static void Man_get(ClientInfo *client, char *filename,
                     const char *orig_url)
{
   int res;
   char *manpage;

   if (!Man_parse_path(filename, &manpage)) {
      /* parse and stat failed, prepare a file-not-found error. */
      res = ENOENT;
   } else {
      client->manpage = dStrdup(manpage);

      /* set up for reading an inner archive file */
      res = Man_prepare_send_file(client, manpage, orig_url);
   }
   if (res != 0) {
      Man_prepare_send_error_page(client, res, orig_url);
   }
}

/*
 * Send HTTP headers and then the file itself.
 */
static int Man_send_file(ClientInfo *client)
{
//#define LBUF 1
#define LBUF 16*1024

   const char *ct = "text/html";
   char buf[LBUF], *d_cmd;
   int st, st2;
   char *lr;

   int lstart, i;

   if (client->state == st_start) {
      /* Send DPI command */
      d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page",
                               client->orig_url);
      a_Dpip_dsh_write_str(client->sh, 1, d_cmd);
      dFree(d_cmd);
      client->state = st_dpip;

   } else if (client->state == st_dpip) {
      /* send HTTP header */

      /* Send HTTP headers */
      a_Dpip_dsh_write_str(client->sh, 0, "HTTP/1.1 200 OK\r\n");
      a_Dpip_dsh_printf(client->sh, 0, "Content-Type: %s\r\n", ct);
      /* Todo: determine content length */
      /*
      a_Dpip_dsh_printf(client->sh, 1,
                        "Content-Length: %ld\r\n"
                        client->file_sz); */
      a_Dpip_dsh_printf(client->sh, 1, "\r\n");
      client->state = st_http;

   } else if (client->state == st_http) {
      a_Dpip_dsh_printf(client->sh, 1, "<pre>");
      client->state = st_pre_content;
   } else if (client->state == st_pre_content) {
      /* Send body -- raw file contents */
      if ((st = a_Dpip_dsh_tryflush(client->sh)) < 0) {
         client->flags |= (st == -3) ? MAN_ERR : 0;
      } else {
         /* no pending data, let's send new data */
         lr = fgets(buf, LBUF, client->man);
         if (ferror(client->man) != 0) {
            MSG("\nERROR while reading from file '%s': %s\n\n",
                client->manpage, dStrerror(errno));
            client->flags |= MAN_ERR;
         } else if (lr == NULL) {
            client->state = st_content;
         } else {
            /* ok to write */
            st2 = strlen(buf);

            /* preprocessing line */
            if(isupper(buf[0]) && isupper(buf[1])) {
               a_Dpip_dsh_printf(client->sh, 1, "<strong>");
            }

            /* write line content */
            if(client->fstate == fs_in_see_also) {

               /* skip initial spaces */
               for(i = 0; buf[i] == ' ' && i < st2; i++) ;

               st = a_Dpip_dsh_trywrite(client->sh, buf, i);
               client->flags |= (st == -3) ? MAN_ERR : 0;

               lstart = i;

               while(buf[lstart] != '\n' && lstart < st2) {

                  a_Dpip_dsh_printf(client->sh, 1, "<a href=\"man:");

                  for(i = lstart; buf[i] != ',' && buf[i] != '\n' && i < st2; i++) ;

                  st = a_Dpip_dsh_trywrite(client->sh, buf + lstart, i - lstart);
                  client->flags |= (st == -3) ? MAN_ERR : 0;

                  a_Dpip_dsh_printf(client->sh, 1, "\">");

                  st = a_Dpip_dsh_trywrite(client->sh, buf + lstart, i - lstart);
                  client->flags |= (st == -3) ? MAN_ERR : 0;

                  a_Dpip_dsh_printf(client->sh, 1, "</a>");

                  lstart = i;

                  for(i = lstart; (buf[i] == ',' || buf[i] == ' ') && i < st2; i++) ;
                  st = a_Dpip_dsh_trywrite(client->sh, buf + lstart, i - lstart);

                  lstart = i;
               }

            } else {
               st = a_Dpip_dsh_trywrite(client->sh, buf, st2);
               client->flags |= (st == -3) ? MAN_ERR : 0;
            }

            /* post processing line */
            if(isupper(buf[0]) && isupper(buf[1])) {
               a_Dpip_dsh_printf(client->sh, 1, "</strong>");
            }
            if(!strncmp(buf, "SEE ALSO", 8)) {
               client->fstate = fs_in_see_also;
            } else if(client->fstate == fs_in_see_also) {
               client->fstate = fs_start;
            }

         }
      }
   } else if (client->state == st_content) {
      a_Dpip_dsh_printf(client->sh, 1, "</pre>");
      client->state = st_post_content;
      client->flags |= MAN_DONE;
   }

   return 0;
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
static ClientInfo *Man_add_client(int sock_fd)
{
   ClientInfo *new_client;

   new_client = dNew(ClientInfo, 1);
   new_client->sh = a_Dpip_dsh_new(sock_fd, sock_fd, 8*1024);
   new_client->orig_url = NULL;
   new_client->manpage = NULL;
   new_client->man = NULL;
   new_client->state = 0;
   new_client->err_code = 0;
   new_client->flags = MAN_READ;
   new_client->old_style = OLD_STYLE;

   dList_append(Clients, new_client);
   return new_client;
}

/*
 * Remove a client from the list.
 */
static void Man_remove_client(ClientInfo *client)
{
   dList_remove(Clients, (void *)client);

   _MSG("Closing Socket Handler\n");
   a_Dpip_dsh_close(client->sh);
   a_Dpip_dsh_free(client->sh);
   Man_close(client->man);
   dFree(client->orig_url);
   if (client->manpage) dFree(client->manpage);

   dFree(client);
}

/*
 * Serve this client.
 */
static void Man_serve_client(void *data, int f_write)
{
   char *dpip_tag = NULL, *cmd = NULL, *url = NULL, *path;
   ClientInfo *client = data;
   int st;

   while (1) {
      _MSG("Man_serve_client %p, flags=%d state=%d\n",
          client, client->flags, client->state);
      if (client->flags & (MAN_DONE | MAN_ERR))
         break;
      if (client->flags & MAN_READ) {
         dpip_tag = a_Dpip_dsh_read_token(client->sh, 0);
         _MSG("dpip_tag={%s}\n", dpip_tag);
         if (!dpip_tag)
            break;
      }

      if (client->flags & MAN_READ) {
         if (!(client->flags & MAN_AUTH_OK)) {
            /* Authenticate our client... */
            st = a_Dpip_check_auth(dpip_tag);
            _MSG("a_Dpip_check_auth returned %d\n", st);
            client->flags |= (st == 1) ? MAN_AUTH_OK : MAN_ERR;
         } else {
            /* Get file request */
            cmd = a_Dpip_get_attr(dpip_tag, "cmd");
            url = a_Dpip_get_attr(dpip_tag, "url");
            path = FileUtil_normalize_path("man", url);
            if (cmd) {
               if (strcmp(cmd, "DpiBye") == 0) {
                  DPIBYE = 1;
                  MSG("(pid %d): Got DpiBye.\n", (int)getpid());
                  client->flags |= MAN_DONE;
               } else if (path) {
                  Man_get(client, path, url);
               } else {
                  client->flags |= MAN_ERR;
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
            Man_send_error_page(client);
         else
            Man_send_file(client);
         break;
      }
   } /*while*/

   client->flags |= (client->sh->status & DPIP_ERROR) ? MAN_ERR : 0;
   client->flags |= (client->sh->status & DPIP_EOF) ? MAN_DONE : 0;
}

/*
 * Serve the client queue.
 */
static void Man_serve_clients()
{
   int i, f_read, f_write;
   ClientInfo *client;

   for (i = 0; (client = dList_nth_data(Clients, i)); ++i) {
      f_read = FD_ISSET(client->sh->fd_in, &read_set);
      f_write = FD_ISSET(client->sh->fd_out, &write_set);
      if (!f_read && !f_write)
         continue;
      Man_serve_client(client, f_write);
      if (client->flags & (MAN_DONE | MAN_ERR)) {
         Man_remove_client(client);
         --i;
      }
   }
}

/* --------------------------------------------------------------------------*/

/*
 * Check the fd sets for activity, with a max timeout.
 * return value: 0 if timeout, 1 if input available, -1 if error.
 */
static int Man_check_fds(uint_t seconds)
{
   int i, st;
   ClientInfo *client;
   struct timeval timeout;

   /* initialize observed file descriptors */
   FD_ZERO (&read_set);
   FD_ZERO (&write_set);
   FD_SET (STDIN_FILENO, &read_set);
   for (i = 0; (client = dList_nth_data(Clients, i)); ++i) {
      if (client->flags & MAN_READ)
         FD_SET (client->sh->fd_in, &read_set);
      if (client->flags & MAN_WRITE)
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
         c_st = Man_check_fds(10);
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
            Man_add_client(sock_fd);
         }
         continue;
      }

      Man_serve_clients();
   }

   if (DPIBYE)
      st = 0;
   return st;
}

