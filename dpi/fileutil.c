/*
 * File: fileutils.c :)
 *
 * Copyright (C) 2000-2007 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#include <ctype.h>           /* for isspace */
#include <errno.h>           /* for errno */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>

#define __USE_XOPEN
#include <time.h>

#include "../dpip/dpip.h"
#include "dpiutil.h"
#include "d_size.h"
#include "fileutil.h"

/*
 * Debugging macros
 */
#define _MSG(...)
#define MSG(...)  printf("[file dpi]: " __VA_ARGS__)
#define _MSG_RAW(...)
#define MSG_RAW(...)  printf(__VA_ARGS__)

/*
 * Compare two FileInfo pointers
 * This function is used for sorting directories
 */
int FileUtil_comp(const FileInfo *f1, const FileInfo *f2)
{
   if (S_ISDIR(f1->mode)) {
      if (S_ISDIR(f2->mode)) {
         return strcmp(f1->filename, f2->filename);
      } else {
         return -1;
      }
   } else {
      if (S_ISDIR(f2->mode)) {
         return 1;
      } else {
         return strcmp(f1->filename, f2->filename);
      }
   }
}

/*
 * Allocate an empty DilloDir structure.
 */
DilloDir *FileUtil_dillodir_new(const char *dirname)
{
   DilloDir *Ddir;

   Ddir = dNew(DilloDir, 1);
   Ddir->dirname = dStrdup(dirname);
   Ddir->flist = dList_new(512);

   return Ddir;
}

/*
 * Add a FileInfo to DilloDir structure.
 */
void FileUtil_dillodir_add(DilloDir *Ddir, char *full_path, struct stat sb)
{
   int dirname_len;
   FileInfo *finfo;

   dirname_len = strlen(Ddir->dirname);
   
   finfo = dNew(FileInfo, 1);
   finfo->full_path = full_path;
   finfo->filename = full_path + dirname_len;
   finfo->size = sb.st_size;
   finfo->mode = sb.st_mode;
   finfo->mtime = sb.st_mtime;

   dList_append(Ddir->flist, finfo);      
}

/*
 * Deallocate a DilloDir structure.
 */
void FileUtil_dillodir_free(DilloDir *Ddir)
{
   int i;
   FileInfo *finfo;

   dReturn_if (Ddir == NULL);

   for (i = 0; i < dList_length(Ddir->flist); ++i) {
      finfo = dList_nth_data(Ddir->flist, i);
      dFree(finfo->full_path);
      dFree(finfo);
   }

   dList_free(Ddir->flist);
   dFree(Ddir->dirname);
   dFree(Ddir);
}


/*
 * Given a hex octet (e3, 2F, 20), return the corresponding
 * character if the octet is valid, and -1 otherwise
 */
static int FileUtil_parse_hex_octet(const char *s)
{
   int hex_value;
   char *tail, hex[3];

   if ((hex[0] = s[0]) && (hex[1] = s[1])) {
      hex[2] = 0;
      hex_value = strtol(hex, &tail, 16);
      if (tail - hex == 2)
        return hex_value;
   }

   return -1;
}

/*
 * Extract the resource part of an URI.
 */
char *FileUtil_get_resource(const char *dpiname, const char *orig)
{
   char *str = (char *) orig;

   dReturn_val_if (orig == NULL, NULL);

   /* Make sure the string starts with dpiname + ":" */
   if (dStrnAsciiCasecmp(str, dpiname, strlen(dpiname)) != 0)
      return NULL;
   str += strlen(dpiname);
   
   if (dStrnAsciiCasecmp(str, ":", 1) != 0)
      return NULL;

   str += 1;
   
   return dStrdup(str);
}

/*
 * Make a file URL into a human (and machine) readable path.
 * The idea is to always have a path that starts with only one slash.
 * Embedded slashes are ignored.
 */
char *FileUtil_normalize_path(const char *dpiname, const char *orig)
{
   char *str = (char *) orig, *basename = NULL, *ret = NULL, *p;

   dReturn_val_if (orig == NULL, ret);

   /* Make sure the string starts with dpiname + ":/" */
   if (dStrnAsciiCasecmp(str, dpiname, strlen(dpiname)) != 0)
      return ret;
   str += strlen(dpiname);
   
   if (dStrnAsciiCasecmp(str, ":/", 2) != 0)
      return ret;
   str += 1;

   /* Skip "localhost" */
   if (dStrnAsciiCasecmp(str, "//localhost/", 12) == 0)
      str += 11;

   /* Skip packed slashes, and leave just one */
   while (str[0] == '/' && str[1] == '/')
      str++;

   {
      int i, val;
      Dstr *ds = dStr_sized_new(32);
      dStr_sprintf(ds, "%s%s%s",
                   basename ? basename : "",
                   basename ? "/" : "",
                   str);
      dFree(basename);

      /* Parse possible hexadecimal octets in the URI path */
      for (i = 0; ds->str[i]; ++i) {
         if (ds->str[i] == '%' &&
             (val = FileUtil_parse_hex_octet(ds->str + i+1)) != -1) {
            ds->str[i] = val;
            dStr_erase(ds, i+1, 2);
         }
      }
      /* Remove the fragment if not a part of the filename */
      if ((p = strrchr(ds->str, '#')) != NULL && access(ds->str, F_OK) != 0)
         dStr_truncate(ds, p - ds->str);
      ret = ds->str;
      dStr_free(ds, 0);
   }

   return ret;
}
 
/*
 * Detects 'Content-Type' when the server does not supply one.
 * It uses the magic(5) logic from file(1). Currently, it
 * only checks the few mime types that Dillo supports.
 *
 * 'Data' is a pointer to the first bytes of the raw data.
 * (this is based on a_Misc_get_content_type_from_data())
 */
const char *FileUtil_get_content_type_from_data(void *Data, size_t Size)
{
   static const char *Types[] = {
      "application/octet-stream",
      "text/html", "text/plain",
      "image/gif", "image/png", "image/jpeg",
      "application/zip"
   };
   int Type = 0;
   char *p = Data;
   size_t i, non_ascci;

   _MSG("FileUtil_get_content_type_from_data:: Size = %d\n", Size);

   /* HTML try */
   for (i = 0; i < Size && dIsspace(p[i]); ++i);
   if ((Size - i >= 5  && !dStrnAsciiCasecmp(p+i, "<html", 5)) ||
       (Size - i >= 5  && !dStrnAsciiCasecmp(p+i, "<head", 5)) ||
       (Size - i >= 6  && !dStrnAsciiCasecmp(p+i, "<title", 6)) ||
       (Size - i >= 14 && !dStrnAsciiCasecmp(p+i, "<!doctype html", 14)) ||
       /* this line is workaround for FTP through the Squid proxy */
       (Size - i >= 17 && !dStrnAsciiCasecmp(p+i, "<!-- HTML listing", 17))) {

      Type = 1;

   /* Images */
   } else if (Size >= 4 && !strncmp(p, "GIF8", 4)) {
      Type = 3;
   } else if (Size >= 4 && !strncmp(p, "\x89PNG", 4)) {
      Type = 4;
   } else if (Size >= 2 && !strncmp(p, "\xff\xd8", 2)) {
      /* JPEG has the first 2 bytes set to 0xffd8 in BigEndian - looking
       * at the character representation should be machine independent. */
      Type = 5;

   /* Compressed */
   } else if (Size >= 4 && (!strncmp(p, "PK\x03\x04", 4) ||
                            !strncmp(p, "PK\x05\x06", 4) ||
                            !strncmp(p, "PK\x07\x08", 4))) {
      Type = 6;

   /* Text */
   } else {
      /* We'll assume "text/plain" if the set of chars above 127 is <= 10
       * in a 256-bytes sample.  Better heuristics are welcomed! :-) */
      non_ascci = 0;
      Size = MIN (Size, 256);
      for (i = 0; i < Size; i++)
         if ((uchar_t) p[i] > 127)
            ++non_ascci;
      if (Size == 256) {
         Type = (non_ascci > 10) ? 0 : 2;
      } else {
         Type = (non_ascci > 0) ? 0 : 2;
      }
   }

   return (Types[Type]);
}

/*
 * Return a content type based on the extension of the filename.
 */
const char *FileUtil_ext(const char *filename)
{
   char *e;

   if (!(e = strrchr(filename, '.')))
      return NULL;

   e++;

   if (!dStrAsciiCasecmp(e, "gif")) {
      return "image/gif";
   } else if (!dStrAsciiCasecmp(e, "jpg") ||
              !dStrAsciiCasecmp(e, "jpeg")) {
      return "image/jpeg";
   } else if (!dStrAsciiCasecmp(e, "png")) {
      return "image/png";
   } else if (!dStrAsciiCasecmp(e, "html") ||
              !dStrAsciiCasecmp(e, "htm") ||
              !dStrAsciiCasecmp(e, "shtml") ||
              !dStrAsciiCasecmp(e, "xhtml")) {
      return "text/html";
   } else if (!dStrAsciiCasecmp(e, "xml") ||
              !dStrAsciiCasecmp(e, "ncx") ||
              !dStrAsciiCasecmp(e, "opf")) {
      return "text/xml";
   } else if (!dStrAsciiCasecmp(e, "pdf")) {
      return "application/pdf";
   } else if (!dStrAsciiCasecmp(e, "zip")) {
      return "application/zip";
   } else if (!dStrAsciiCasecmp(e, "epub")) {
      return "application/epub";
   } else if (!dStrAsciiCasecmp(e, "rss")) {
      return "application/rss+xml";
   } else if (!dStrAsciiCasecmp(e, "gmi")) {
      return "text/gemini";
   } else if (!dStrAsciiCasecmp(e, "gophermap")) {
      return "text/gopher";
   } else if (!dStrAsciiCasecmp(e, "md")) {
      return "text/markdown";
   } else if (!dStrAsciiCasecmp(e, "js")) {
      return "text/javascript";
   } else if (!dStrAsciiCasecmp(e, "css")) {
      return "text/css";
   } else if (!dStrAsciiCasecmp(e, "txt")) {
      return "text/plain";
   } else {
      return NULL;
   }
}

/*
 * Given a timestamp, output an HTML-formatted date string.
 */
void FileUtil_print_mtime(Dsh *sh, time_t mtime, int old_style)
{
   char *ds = ctime(&mtime);

   /* Month, day and {hour or year} */
   if (old_style) {
      a_Dpip_dsh_printf(sh, 0, " %.3s %.2s", ds + 4, ds + 8);
      if (time(NULL) - mtime > 15811200) {
         a_Dpip_dsh_printf(sh, 0, "  %.4s", ds + 20);
      } else {
         a_Dpip_dsh_printf(sh, 0, " %.5s", ds + 11);
      }
   } else {
      a_Dpip_dsh_printf(sh, 0,
         "<td>%.3s&nbsp;%.2s&nbsp;%.5s", ds + 4, ds + 8,
         /* (more than 6 months old) ? year : hour; */
         (time(NULL) - mtime > 15811200) ? ds + 20 : ds + 11);
   }
}

/*
 * Output the HTML page header and title.
 */
void FileUtil_print_page_header(Dsh *sh, const char *dpiname, const char *dirname, int old_style)
{
   char *Hdirname, *Udirname, *HUdirname;

   /* Send page title */
   Udirname = Escape_uri_str(dirname, NULL);
   HUdirname = Escape_html_str(Udirname);
   Hdirname = Escape_html_str(dirname);

   a_Dpip_dsh_printf(sh, 0,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/html\r\n"
      "\r\n"
      "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01 Transitional//EN'>\n"
      "<HTML>\n<HEAD>\n <BASE href='%s:%s'>\n"
      " <TITLE>%s:%s</TITLE>\n</HEAD>\n"
      "<BODY><H1>Directory listing of %s</H1>\n",
      dpiname, HUdirname, dpiname, Hdirname, Hdirname);
   dFree(Hdirname);
   dFree(HUdirname);
   dFree(Udirname);
   
   if (old_style) {
      a_Dpip_dsh_write_str(sh, 0, "<pre>\n");
   }
}

/*
 * Output the string for parent directory.
 */
void FileUtil_print_parent_dir(Dsh *sh, const char *dpiname, const char *dirname)
{
   if (strcmp(dirname, "/") != 0) {        /* Not the root dir */
      char *p, *parent, *HUparent, *Uparent;

      parent = dStrdup(dirname);
      /* cut trailing slash */
      parent[strlen(parent) - 1] = '\0';
      /* make 'parent' have the parent dir path */
      if ((p = strrchr(parent, '/')))
         *(p + 1) = '\0';

      Uparent = Escape_uri_str(parent, NULL);
      HUparent = Escape_html_str(Uparent);
      a_Dpip_dsh_printf(sh, 0,
         "<a href='%s:%s'>Parent directory</a>", dpiname, HUparent);
      dFree(HUparent);
      dFree(Uparent);
      dFree(parent);
   }
}

/*
 * Output the header for the file listing table.
 */
void FileUtil_print_table_header(Dsh *sh, int dirlen, int old_style)
{
   if (dirlen) {
      if (old_style) {
         a_Dpip_dsh_write_str(sh, 0, "\n\n");
      } else {
         a_Dpip_dsh_write_str(sh, 0,
            "<br><br>\n"
            "<table border=0 cellpadding=1 cellspacing=0"
            " bgcolor=#E0E0E0 width=100%>\n"
            "<tr align=center>\n"
            "<td>\n"
            "<td width=60%><b>Filename</b>"
            "<td><b>Type</b>"
            "<td><b>Size</b>"
            "<td><b>Modified&nbsp;at</b>\n");
      }
   } else {
      a_Dpip_dsh_write_str(sh, 0, "<br><br>Directory is empty...");
   }
}

/*
 * Output a HTML-line from file info.
 */
void FileUtil_print_info(Dsh *sh, FileInfo *finfo, int n, const char *filecont, int old_style)
{
   int size;
   char *sizeunits;
   char namebuf[MAXNAMESIZE + 1];
   char *Uref, *HUref, *Hname;
   const char *ref, *name = finfo->filename;
   const char *HHandler = "";

   if (finfo->size <= 9999) {
      size = finfo->size;
      sizeunits = "bytes";
   } else if (finfo->size / 1024 <= 9999) {
      size = finfo->size / 1024 + (finfo->size % 1024 >= 1024 / 2);
      sizeunits = "KB";
   } else {
      size = finfo->size / 1048576 + (finfo->size % 1048576 >= 1048576 / 2);
      sizeunits = "MB";
   }

   /* we could note if it's a symlink... */
   if (S_ISDIR (finfo->mode)) {
      filecont = "Directory";
   } else if (finfo->mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
      filecont = "Executable";
   } else {
      if (!filecont || !strcmp(filecont, "application/octet-stream"))
         filecont = "unknown";
   }
   
   /* dirty trick to open the file with the correct dpi */
   if(!strcmp(filecont, "application/zip") ||
      !strcmp(filecont, "application/epub")) {
      HHandler = "zip:";
   }

   ref = finfo->full_path;

   if (strlen(name) > MAXNAMESIZE) {
      memcpy(namebuf, name, MAXNAMESIZE - 3);
      strcpy(namebuf + (MAXNAMESIZE - 3), "...");
      name = namebuf;
   }

   /* escape problematic filenames */
   Uref = Escape_uri_str(ref, NULL);
   HUref = Escape_html_str(Uref);
   Hname = Escape_html_str(name);

   if (old_style) {
      char *dots = ".. .. .. .. .. .. .. .. .. .. .. .. .. .. .. .. ..";
      int ndots = MAXNAMESIZE - strlen(name);
      a_Dpip_dsh_printf(sh, 0,
         "%s<a href='%s%s'>%s</a>"
         " %s"
         " %-11s%4d %-5s",
         S_ISDIR (finfo->mode) ? ">" : " ", HHandler, HUref, Hname[0] == '/' ? Hname + 1 : Hname,
         dots + 50 - (ndots > 0 ? ndots : 0),
         filecont, size, sizeunits);

   } else {
      a_Dpip_dsh_printf(sh, 0,
         "<tr align=center %s><td>%s<td align=left><a href='%s%s'>%s</a>"
         "<td>%s<td>%d&nbsp;%s",
         (n & 1) ? "bgcolor=#dcdcdc" : "",
         S_ISDIR (finfo->mode) ? ">" : " ", HHandler, HUref, Hname[0] == '/' ? Hname + 1 : Hname,
         filecont, size, sizeunits);
   }
   FileUtil_print_mtime(sh, finfo->mtime, old_style);
   a_Dpip_dsh_write_str(sh, 0, "\n");

   dFree(Hname);
   dFree(HUref);
   dFree(Uref);
}

/*
 * Output the footer for the file listing table.
 */
void FileUtil_print_table_footer(Dsh *sh, int dirlen, int old_style)
{
   if (!old_style && dirlen) {
      a_Dpip_dsh_write_str(sh, 0, "</table>\n");
   }
}

/*
 * Output the HTML page footer.
 */
void FileUtil_print_page_footer(Dsh *sh, int old_style)
{
   if (old_style) {
      a_Dpip_dsh_write_str(sh, 0, "</pre>\n");
   }

   a_Dpip_dsh_write_str(sh, 1, "</BODY></HTML>\n");
}
