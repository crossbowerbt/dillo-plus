/*
 * File: fileutil.h
 *
 * Copyright 2004-2005 Jorge Arellano Cid <jcid@dillo.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 */

/*
 * This file contains common functions used by file-like dpi programs.
 * (i.e. a convenience library).
 */

#ifndef __FILEUTIL_H__
#define __FILEUTIL_H__

#include <stdio.h>
#include "d_size.h"
#include "../dlib/dlib.h"

#define MAXNAMESIZE 30

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct {
   char *full_path;
   const char *filename;
   off_t size;
   mode_t mode;
   time_t mtime;
} FileInfo;

/*
 * Detects 'Content-Type' when the server does not supply one.
 * It uses the magic(5) logic from file(1). Currently, it
 * only checks the few mime types that Dillo supports.
 *
 * 'Data' is a pointer to the first bytes of the raw data.
 * (this is based on a_Misc_get_content_type_from_data())
 */
const char *File_get_content_type_from_data(void *Data, size_t Size);

/*
 * Return a content type based on the extension of the filename.
 */
const char *File_ext(const char *filename);

/*
 * Given a timestamp, output an HTML-formatted date string.
 */
void File_print_mtime(Dsh *sh, time_t mtime, int old_style);

/*
 * Output the HTML page header and title.
 */
void File_print_page_header(Dsh *sh, const char *dpiname, const char *dirname, int old_style);

/*
 * Output the string for parent directory.
 */
void File_print_parent_dir(Dsh *sh, const char *dpiname, const char *dirname);

/*
 * Output the header for the file listing table.
 */
void File_print_table_header(Dsh *sh, int dirlen, int old_style);

/*
 * Output a HTML-line from file info.
 */
void File_print_info(Dsh *sh, FileInfo *finfo, int n, const char *filecont, int old_style);

/*
 * Output the footer for the file listing table.
 */
void File_print_table_footer(Dsh *sh, int dirlen, int old_style);

/*
 * Output the HTML page footer.
 */
void File_print_page_footer(Dsh *sh, int old_style);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __FILEUTIL_H__ */

