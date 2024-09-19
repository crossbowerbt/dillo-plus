/*
 * File: paths.cc
 *
 * Copyright 2006-2009 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "msg.h"
#include "../dlib/dlib.h"
#include "paths.hh"

#include   <fcntl.h>

/*
 * Local data
 */

// Dillo works from an unmounted directory (/tmp)
static char* oldWorkingDir = NULL;

/*
 * Changes current working directory to /tmp and creates home config dir
 * if not exists.
 */
void Paths::init(void)
{
   char *path;
   struct stat st;
   int rc = 0;

   dFree(oldWorkingDir);
   oldWorkingDir = dGetcwd();
   rc = chdir("/tmp");
   if (rc == -1) {
      MSG("paths: Error changing directory to /tmp: %s\n",
          dStrerror(errno));
   }

   path = dStrconcat(dGethomedir(), "/." BINNAME, NULL);
   if (stat(path, &st) == -1) {
      if (errno == ENOENT) {
         MSG("paths: Creating directory '%s/'\n", path);
         if (mkdir(path, 0700) < 0) {
            MSG("paths: Error creating directory %s: %s\n",
                path, dStrerror(errno));
         }
      } else {
         MSG("Dillo: error reading %s: %s\n", path, dStrerror(errno));
      }
   }

   dFree(path);
}

/*
 * Return the initial current working directory in a string.
 */
char *Paths::getOldWorkingDir(void)
{
   return oldWorkingDir;
}

/*
 * Free memory
 */
void Paths::free(void)
{
   dFree(oldWorkingDir);
}

/*
 * Examines the path for "rcFile" and assign its file pointer to "fp".
 */
FILE *Paths::getPrefsFP(const char *rcFile)
{
   FILE *fp;
   char *path = dStrconcat(dGethomedir(), "/." BINNAME "/", rcFile, NULL);
   char *path2 = dStrconcat(DILLO_SYSCONF, rcFile, NULL);

   if (!(fp = fopen(path, "r"))) {
      copy_file(path2, path);
        if (!(fp = fopen(path, "r"))) {
         MSG("paths: Using internal defaults because could not make '%s': %s\n", path, dStrerror(errno));
      }
   }

   dFree(path);
   dFree(path2);

   return fp;
}

void Paths::copy_file( char *filename, char *outfile){
   int     in_fd, out_fd, n_chars;
   char    buf[BUFFERSIZE];

   if ( (in_fd=open(filename, O_RDONLY)) == -1 )
      MSG("Cannot open %s: %s", filename,dStrerror(errno));

   if ( (out_fd=creat( outfile, COPYMODE)) == -1 )
      MSG( "Cannot create %s: %s", outfile, dStrerror(errno));

   while ( (n_chars = read(in_fd , buf, BUFFERSIZE)) > 0 )

   if ( write( out_fd, buf, n_chars ) != n_chars )
      MSG("Write error to %s: %s", outfile, dStrerror(errno));
   if ( n_chars == -1 )
      MSG("Read error from %s: %s", outfile, dStrerror(errno));
   if ( close(in_fd) == -1 || close(out_fd) == -1 )
      MSG("Error closing files %s", dStrerror(errno));
}
