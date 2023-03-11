/*
 * Dpi for Dillo Local Scripts (DLS).
 *
 *
 *
 *                            W A R N I N G
 *
 * Copyright 2003, 2004 Jorge Arellano Cid <jcid@dillo.org>
 * Copyright 2004 Garrett Kajmowicz <gkajmowi@tbaytel.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * As a special exception permission is granted to link the code of
 * the dls dillo plugin with the OpenSSL project's OpenSSL library
 * (or a modified version of that library), and distribute the linked
 * executables, without including the source code for the SSL library
 * in the source distribution. You must obey the GNU General Public
 * License, version 3, in all respects for all of the code used other
 * than the SSL library.
 *
 */

#include <config.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/stat.h>
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
 #define MSG(...)  fprintf(stderr, "[dls dpi]: " __VA_ARGS__)
#endif

char servername[1024];

/*---------------------------------------------------------------------------*/
/*
 * Global variables
 */
static char *root_url = NULL;  /*Holds the URL we are connecting to*/
static Dsh *sh;

/*
 * This function compares end of strings.
 */
static int ends_with(const char *str, const char *suffix)
{
   if (!str || !suffix)
     return 0;
   size_t lenstr = strlen(str);
   size_t lensuffix = strlen(suffix);
   if (lensuffix >  lenstr)
     return 0;
   return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

pid_t my_popen(const char* cmd, const char* arg, int *pipe_2) {
   int pid = 0;
   int pipe_1[2];

   if (pipe(pipe_1) < 0) {
      return 0;
   }

   if (pipe(pipe_2) < 0) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      return 0;
   }

   pid = fork();

   if (pid == -1) {
      close(pipe_1[0]);
      close(pipe_1[1]);
      close(pipe_2[0]);
      close(pipe_2[1]);
      return 0;
   }

   else if (pid == 0) {
      // child

      close(fileno(stdin));
      close(fileno(stdout));
      
      close(pipe_1[1]);
      close(pipe_2[0]);

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
      const char* argp[] = {cmd, arg, NULL};
      execv(cmd, (char**) argp);

      // if the function returns an error has occurred
      printf("HTTP/1.1 500 Execution Error\r\n"
	     "Content-Type: text/html\r\n\r\n"
	     "<pre>Error during execv(): %s</pre>",
	     strerror(errno));
      
      exit(127);
   }

   // parent
   
   close(pipe_1[0]);
   close(pipe_2[1]);
   close(pipe_1[1]); // do not write to child stdin
   
   return pid;
}

/*
 * This function execute a command and save its output.
 */
static char* pipe_exec(const char* cmd, const char* arg, int *result_size_ext) {
   int last_read = 0;
   char buffer[1024];

   int child_pipe[2];

   int result_size = 1024 * 8, result_pos = 0;
   char *result = NULL;

   int pstat;
   pid_t pid;

   // open child process
   
   pid = my_popen(cmd, arg, child_pipe);

   if (!pid) {
      return NULL;
   }

   FILE *child_stdout = fdopen(child_pipe[0], "r");

   result = (char *) malloc(result_size);

   // read child output
   
   while ((last_read = fread(buffer, 1, sizeof(buffer), child_stdout))) {
      if((result_size - result_pos) <= (last_read + 1)) {
	result_size *= 2;
	result = realloc(result, result_size);
      }
      memcpy(result+result_pos, buffer, last_read);
      result_pos += last_read;
   }

   // close and wait child
   
   fclose(child_stdout);
   
   do {
      pid = waitpid(pid, &pstat, 0);
   } while (pid == -1 && errno == EINTR);

   // finalize result
   
   result[result_pos] = '\0';
   *result_size_ext = result_pos;
   
   return result;
}

/*
 *  This handles the execution of a local script and returns its output to dillo
 */
static void handle_local_script(void)
{
   /* The following variable will be set to 1 in the event of
    * an error and skip any further processing
    */
   int exit_error = 0;
   char error_msg[1024] = { 0 };

   char *dpip_tag = NULL, *cmd = NULL, *url = NULL,
     *dls_path = NULL, *dls_arg = NULL, *result = NULL;

   char dls_path_real[2048] = DILLO_LIBDIR "dls/";

   int result_size = 0;
   
   struct stat sb;

   char *d_cmd;

   MSG("{ In dls.filter.dpi}\n");

   /*Get script and the command to be used*/
   dpip_tag = a_Dpip_dsh_read_token(sh, 1);

   MSG("dpip_tag = %s\n", dpip_tag);
	       
   cmd = a_Dpip_get_attr(dpip_tag, "cmd");
   url = a_Dpip_get_attr(dpip_tag, "url");

   if (!url) {
     MSG("***Value of url is NULL"
	 " - cannot continue\n");
     exit_error = 1;
   }

   if (strncmp(url, "dls:", sizeof("dls:") - 1)) {
     MSG("***Invalid URL scheme"
	 " - cannot continue\n");
     exit_error = 1;
   }

   dls_path = url + sizeof("dls:") - 1; // get DLS file path

   dls_arg = strchr(dls_path, '?'); // get DLS cmd arg

   if(dls_arg) {
      *dls_arg = '\0';
      dls_arg++;
   }

   /* if not path is provided fallback to default DLS */

   if(!strlen(dls_path))
     dls_path = "default";

   /* check file name, path and permissions */

   if(exit_error == 0) {
     if(strlen(dls_path) > (sizeof(dls_path_real)/2)) {
         MSG("DLS name too long\n");
         strncpy(error_msg, "DLS name too long", sizeof(error_msg)-1);
         exit_error = 1;
      }
   }

   strncat(dls_path_real, dls_path, sizeof(dls_path_real));
   strncat(dls_path_real, ".dls", sizeof(dls_path_real));

   fprintf(stderr, "DLS Script = %s\n", dls_path_real);
   
   if(exit_error == 0) {
      if(!ends_with(dls_path_real, ".dls")) {
         MSG("DLS file do not end with .dls\n");
         strncpy(error_msg, "DLS file do not end with .dls", sizeof(error_msg)-1);
         exit_error = 1;
      }
   }

   if(exit_error == 0) {
      if(stat(dls_path_real, &sb) != 0) {
         MSG("DLS file not found\n");
         strncpy(error_msg, "DLS file not found", sizeof(error_msg)-1);
         exit_error = 1;
      }
   }

   if(exit_error == 0) {
      if(!(sb.st_mode & S_IXUSR)) {
         MSG("DLS file is not executable\n");
         strncpy(error_msg, "DLS file is not executable", sizeof(error_msg)-1);
         exit_error = 1;
      }
   }

   /* execute dls */

   if(exit_error == 0) {
     result = pipe_exec(dls_path_real, dls_arg, &result_size);

      if(!result) {
         MSG("DLS execution error\n");
	 strncpy(error_msg, "DLS execution error", sizeof(error_msg)-1);
	 exit_error = 1;
      }
   }

   /*Send dpi command*/
   d_cmd = a_Dpip_build_cmd("cmd=%s url=%s", "start_send_page", url);
   a_Dpip_dsh_write_str(sh, 1, d_cmd);
   dFree(d_cmd);

   if(exit_error == 0) {
     
      a_Dpip_dsh_write(sh, 1, result, result_size);

   }
      
   else {

      char error[1024];

      snprintf(error, sizeof(error)-1, "HTTP/1.1 500 Execution Error\r\n"
                                       "Content-Type: text/html\r\n"
                                       "Content-Length: %zu\r\n\r\n", strlen(error_msg));
      
      a_Dpip_dsh_write(sh, 1, error, strlen(error));
      a_Dpip_dsh_write_str(sh, 1, error_msg);

   }
   
   /*Begin cleanup of all resources used*/
   dFree(dpip_tag);
   dFree(cmd);
   dFree(url);

}


/*---------------------------------------------------------------------------*/
int main(void)
{
   char *dpip_tag;

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

   /* Run and get output from a local script */
   handle_local_script();

   /* Finish the SockHandler */
   a_Dpip_dsh_close(sh);
   a_Dpip_dsh_free(sh);

   dFree(root_url);

   MSG("{ exiting dls.dpi}\n");

   return 0;
}
