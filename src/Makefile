include ../Makefile.options

CXXFLAGS_EXTRA = -DDILLO_SYSCONF='"$(DILLO_ETCDIR)"' -DDOC_PATH='"$(DOC_PATH)"' -DBINNAME='"$(BINNAME)"'

all: recurse_into_IO $(BINNAME)

recurse_into_IO:
	@echo "Making all in IO"
	@(cd IO; make)

$(BINNAME).o: $(BINNAME).cc
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c $(BINNAME).cc

paths.o: paths.cc paths.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c paths.cc

tipwin.o: tipwin.cc tipwin.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c tipwin.cc

ui.o: ui.cc ui.hh pixmaps.h
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c -o ui.o ui.cc

uicmd.o: uicmd.cc uicmd.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c uicmd.cc

bw.o: bw.c bw.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c bw.c

cookies.o: cookies.c cookies.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c cookies.c

auth.o: auth.c auth.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c auth.c

md5.o: md5.c md5.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c md5.c

digest.o: digest.c digest.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c digest.c

colors.o: colors.c colors.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c colors.c

misc.o: misc.c misc.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c misc.c

history.o: history.c history.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c history.c

hsts.o: hsts.c hsts.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c hsts.c

prefs.o: prefs.c
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c prefs.c

prefsparser.o: prefsparser.cc prefsparser.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c prefsparser.cc

keys.o: keys.cc keys.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c keys.cc

url.o: url.c url.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c url.c

bitvec.o: bitvec.c bitvec.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c bitvec.c

klist.o: klist.c klist.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c klist.c

chain.o: chain.c chain.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c chain.c

utf8.o: utf8.cc utf8.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c utf8.cc

timeout.o: timeout.cc timeout.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c timeout.cc

dialog.o: dialog.cc dialog.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c dialog.cc

web.o: web.cc web.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c web.cc

nav.o: nav.c nav.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c nav.c

cache.o: cache.c cache.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c cache.c

decode.o: decode.c decode.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c decode.c

dicache.o: dicache.c dicache.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c dicache.c

capi.o: capi.c capi.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c capi.c

domain.o: domain.c domain.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c domain.c

css.o: css.cc css.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c css.cc

cssparser.o: cssparser.cc cssparser.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c cssparser.cc

styleengine.o: styleengine.cc styleengine.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c styleengine.cc

plain.o: plain.cc
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c plain.cc

html.o: html.cc html.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c html.cc

form.o: form.cc form.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c form.cc

table.o: table.cc table.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c table.cc

bookmark.o: bookmark.c bookmark.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c bookmark.c

dns.o: dns.c dns.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c dns.c

gif.o: gif.c
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c gif.c

jpeg.o: jpeg.c
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c jpeg.c

png.o: png.c
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c png.c

imgbuf.o: imgbuf.cc imgbuf.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c imgbuf.cc

image.o: image.cc image.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c image.cc

menu.o: menu.cc menu.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c menu.cc

dpiapi.o: dpiapi.c dpiapi.h
	$(COMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) $(LIBPNG16_CXXFLAGS) -c dpiapi.c

findbar.o: findbar.cc findbar.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c findbar.cc

xembed.o: xembed.cc xembed.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) -c xembed.cc


$(BINNAME): $(BINNAME).o paths.o tipwin.o ui.o uicmd.o bw.o cookies.o auth.o md5.o digest.o colors.o misc.o history.o hsts.o prefs.o prefsparser.o keys.o url.o bitvec.o klist.o chain.o utf8.o timeout.o dialog.o web.o nav.o cache.o decode.o dicache.o capi.o domain.o css.o cssparser.o styleengine.o plain.o html.o form.o table.o bookmark.o dns.o gif.o jpeg.o png.o imgbuf.o image.o menu.o dpiapi.o findbar.o xembed.o ../dlib/libDlib.a ../dpip/libDpip.a IO/libDiof.a ../dw/libDw-widgets.a ../dw/libDw-fltk.a ../dw/libDw-core.a ../lout/liblout.a
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) $(LIBPNG16_CXXFLAGS) $(LDFLAGS) $(DILLO_LDFLAGS) $(HTTPS_LDFLAGS) -o $(BINNAME) $(BINNAME).o paths.o tipwin.o ui.o uicmd.o bw.o cookies.o auth.o md5.o digest.o colors.o misc.o history.o hsts.o prefs.o prefsparser.o keys.o url.o bitvec.o klist.o chain.o utf8.o timeout.o dialog.o web.o nav.o cache.o decode.o dicache.o capi.o domain.o css.o cssparser.o styleengine.o plain.o html.o form.o table.o bookmark.o dns.o gif.o jpeg.o png.o imgbuf.o image.o menu.o dpiapi.o findbar.o xembed.o ../dlib/libDlib.a ../dpip/libDpip.a IO/libDiof.a ../dw/libDw-widgets.a ../dw/libDw-fltk.a ../dw/libDw-core.a ../lout/liblout.a

clean:
	rm -f *.o *.a $(BINNAME)
	@echo "Making clean in IO"
	@(cd IO; make clean)

install: all
	$(INSTALL_SH) -c -d "$(DILLO_BINDIR)"
	$(INSTALL) -c $(BINNAME) "$(DILLO_BINDIR)"

uninstall:
	rm -f "$(DILLO_BINDIR)/$(BINNAME)"
