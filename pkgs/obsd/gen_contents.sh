#!/bin/sh

function get_sha {
	sha256 -b "$1" | cut -d ' ' -f 4
}

function get_size {
	stat "$1" | cut -d ' ' -f 8
}

function gen_file {
	local prefix="$1"
	local path="$2"
	echo "$path"
	echo -n "@sha "
	echo $(get_sha "$prefix$path")
	echo -n "@size "
	echo $(get_size "$prefix$path")
}

cat  >+CONTENTS <<EOF
@name dillo-plus-3.3.0-custom
@version 10
@arch amd64
$(gen_file "" +DESC)
@depend converters/libiconv:libiconv-*:libiconv-1.17
@depend graphics/jpeg:jpeg-*:jpeg-2.1.5.1v0
@depend graphics/png:png-*:png-1.6.39
@depend x11/fltk:fltk-*:fltk-1.3.3p3
@wantlib X11.18.0
@wantlib Xau.10.0
@wantlib Xcursor.5.0
@wantlib Xdmcp.11.0
@wantlib Xext.13.0
@wantlib Xfixes.6.1
@wantlib Xft.12.0
@wantlib Xinerama.6.0
@wantlib c++.9.0
@wantlib c++abi.6.0
@wantlib c.97.1
@wantlib crypto.52.0
@wantlib fltk.8.0
@wantlib fontconfig.13.1
@wantlib iconv.7.1
@wantlib jpeg.70.1
@wantlib m.10.1
@wantlib png.18.0
@wantlib pthread.27.1
@wantlib ssl.55.0
@wantlib z.7.0
@cwd /usr/local
@bin $(gen_file "/usr/local/" bin/dillo)
@bin $(gen_file "/usr/local/" bin/dillo-install-hyphenation)
@bin $(gen_file "/usr/local/" bin/dpid)
@bin $(gen_file "/usr/local/" bin/dpidc)
lib/dillo/
lib/dillo/dpi/
lib/dillo/dpi/bookmarks/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/bookmarks/bookmarks.dpi)
lib/dillo/dpi/cookies/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/cookies/cookies.dpi)
lib/dillo/dpi/datauri/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/datauri/datauri.filter.dpi)
lib/dillo/dpi/downloads/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/downloads/downloads.dpi)
lib/dillo/dpi/file/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/file/file.dpi)
lib/dillo/dpi/zip/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/zip/zip.dpi)
lib/dillo/dpi/man/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/man/man.dpi)
lib/dillo/dpi/ftp/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/ftp/ftp.filter.dpi)
lib/dillo/dpi/gemini/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/gemini/gemini.filter.dpi)
lib/dillo/dpi/gopher/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/gopher/gopher.filter.dpi)
lib/dillo/dpi/hello/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/hello/hello.filter.dpi)
lib/dillo/dpi/vsource/
@bin $(gen_file "/usr/local/" lib/dillo/dpi/vsource/vsource.filter.dpi)
@man $(gen_file "/usr/local/" man/man1/dillo.1)
share/doc/dillo/
$(gen_file "/usr/local/" share/doc/dillo/user_help.html)
$(gen_file "/usr/local/" share/doc/dillo/Cookies.txt)
etc/dillo/
$(gen_file "/usr/local/" etc/dillo/dpidrc)
$(gen_file "/usr/local/" etc/dillo/dillorc)
$(gen_file "/usr/local/" etc/dillo/style.css)
$(gen_file "/usr/local/" etc/dillo/style_reader_mode.css)
EOF

