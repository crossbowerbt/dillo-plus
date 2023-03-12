include ../Makefile.options

CXXFLAGS_EXTRA = -DDILLO_BINDIR='"$(DILLO_BINDIR)"' -DDILLO_LIBDIR='"$(DILLO_LIBDIR)"'

all: bookmarks.dpi cookies.dpi datauri.filter.dpi downloads.dpi file.dpi ftp.filter.dpi dls.filter.dpi gemini.filter.dpi gopher.filter.dpi hello.filter.dpi vsource.filter.dpi

bookmarks.o: bookmarks.c
	$(COMPILE) -c bookmarks.c

dpiutil.o: dpiutil.c dpiutil.h
	$(COMPILE) -c dpiutil.c

bookmarks.dpi: bookmarks.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o bookmarks.dpi bookmarks.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

cookies.o: cookies.c
	$(COMPILE) -c cookies.c

cookies.dpi: cookies.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o cookies.dpi cookies.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

datauri.o: datauri.c
	$(COMPILE) -c datauri.c

datauri.filter.dpi: datauri.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o datauri.filter.dpi datauri.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

downloads_dpi-downloads.o: downloads.cc
	$(CXXCOMPILE) $(LIBFLTK_CFLAGS) -o downloads_dpi-downloads.o -c downloads.cc

downloads.dpi: downloads_dpi-downloads.o  dpiutil.o ../dpip/libDpip.a  ../dlib/libDlib.a
	$(CXXCOMPILE) $(LIBFLTK_CFLAGS) $(LIBFLTK_LDFLAGS) -o downloads.dpi downloads_dpi-downloads.o  dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

file.o: file.c
	$(COMPILE) -c file.c

file.dpi: file.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o file.dpi file.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

ftp.o: ftp.c
	$(COMPILE) -c ftp.c

ftp.filter.dpi: ftp.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o ftp.filter.dpi ftp.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

hello.o: hello.c
	$(COMPILE) -c hello.c

hello.filter.dpi: hello.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o hello.filter.dpi hello.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

dls.o: dls.c
	$(COMPILE) $(CXXFLAGS_EXTRA) -c dls.c

dls.filter.dpi: dls.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) $(HTTPS_LDFLAGS) -o dls.filter.dpi dls.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

gemini.o: gemini.c
	$(COMPILE) -c gemini.c

gemini.filter.dpi: gemini.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) $(HTTPS_LDFLAGS) -o gemini.filter.dpi gemini.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

gopher.o: gopher.c
	$(COMPILE) -c gopher.c

gopher.filter.dpi: gopher.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) $(HTTPS_LDFLAGS) -o gopher.filter.dpi gopher.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

vsource.o: vsource.c
	$(COMPILE) -c vsource.c

vsource.filter.dpi: vsource.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(COMPILE) -o vsource.filter.dpi vsource.o dpiutil.o ../dpip/libDpip.a ../dlib/libDlib.a

clean:
	rm -f *.o *.a *.dpi

install: all
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/bookmarks/"
	$(INSTALL) -c bookmarks.dpi "$(DILLO_LIBDIR)/dpi/bookmarks/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/cookies/"
	$(INSTALL) -c cookies.dpi "$(DILLO_LIBDIR)/dpi/cookies/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/datauri/"
	$(INSTALL) -c datauri.filter.dpi "$(DILLO_LIBDIR)/dpi/datauri/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/downloads/"
	$(INSTALL) -c downloads.dpi "$(DILLO_LIBDIR)/dpi/downloads/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/file/"
	$(INSTALL) -c file.dpi "$(DILLO_LIBDIR)/dpi/file/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/ftp/"
	$(INSTALL) -c ftp.filter.dpi "$(DILLO_LIBDIR)/dpi/ftp/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/dls/"
	$(INSTALL) -c dls.filter.dpi "$(DILLO_LIBDIR)/dpi/dls/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/gemini/"
	$(INSTALL) -c gemini.filter.dpi "$(DILLO_LIBDIR)/dpi/gemini/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/gopher/"
	$(INSTALL) -c gopher.filter.dpi "$(DILLO_LIBDIR)/dpi/gopher/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/hello/"
	$(INSTALL) -c hello.filter.dpi "$(DILLO_LIBDIR)/dpi/hello/"
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dpi/vsource/"
	$(INSTALL) -c vsource.filter.dpi "$(DILLO_LIBDIR)/dpi/vsource/"

uninstall:
	rm -f "$(DILLO_LIBDIR)/dpi/bookmarks/bookmarks.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/cookies/cookies.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/datauri/datauri.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/downloads/downloads.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/file/file.pdi"
	rm -f "$(DILLO_LIBDIR)/dpi/ftp/ftp.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/dls/dls.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/gemini/gemini.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/gopher/gopher.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/hello/hello.filter.dpi"
	rm -f "$(DILLO_LIBDIR)/dpi/vsource/vsource.filter.dpi"
