CXXCOMPILE = $(CXX) $(DEFS) $(INCLUDES) $(CXXFLAGS)
CXX = g++
CXXFLAGS = -g -O2 -DD_DNS_THREADED -D_REENTRANT -D_THREAD_SAFE -Wall -W -Wno-unused-parameter -Waggregate-return -fno-rtti -fno-exceptions
INCLUDES = -I. -I.. -I/usr/local/include
LIBFLTK_CXXFLAGS = -I/usr/local/include -I/usr/X11R6/include/freetype2 -O2 -pipe -I/usr/local/include -fvisibility-inlines-hidden -I/usr/X11R6/include -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE -D_THREAD_SAFE -D_REENTRANT

DILLO_LIBDIR = $(PREFIX)/lib/dillo/
CXXFLAGS_EXTRA = -DDILLO_LIBDIR='"$(DILLO_LIBDIR)"'

all: libDw-core.a libDw-fltk.a libDw-widgets.a

libDw-core.a: findtext.o imgrenderer.o iterator.o layout.o selection.o style.o types.o ui.o widget.o
	$(AR) $(ARFLAGS) libDw-core.a findtext.o imgrenderer.o iterator.o layout.o selection.o style.o types.o ui.o widget.o
	$(RANLIB) libDw-core.a

findtext.o: findtext.hh findtext.cc
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c findtext.cc

imgrenderer.o: imgrenderer.hh imgrenderer.cc
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c imgrenderer.cc

iterator.o: iterator.cc iterator.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c iterator.cc

layout.o: layout.cc layout.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c layout.cc

selection.o: selection.cc selection.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c selection.cc

style.o: style.cc style.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c style.cc

types.o: types.cc types.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c types.cc

ui.o: ui.cc ui.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c ui.cc

widget.o: widget.cc widget.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c widget.cc

libDw-fltk.a: libDw_fltk_a-fltkcomplexbutton.o libDw_fltk_a-fltkflatview.o libDw_fltk_a-fltkimgbuf.o libDw_fltk_a-fltkmisc.o libDw_fltk_a-fltkplatform.o libDw_fltk_a-fltkpreview.o libDw_fltk_a-fltkui.o libDw_fltk_a-fltkviewbase.o  libDw_fltk_a-fltkviewport.o
	$(AR) $(ARFLAGS) libDw_fltk_a-fltkcomplexbutton.o libDw_fltk_a-fltkflatview.o libDw_fltk_a-fltkimgbuf.o libDw_fltk_a-fltkmisc.o libDw_fltk_a-fltkplatform.o libDw_fltk_a-fltkpreview.o libDw_fltk_a-fltkui.o libDw_fltk_a-fltkviewbase.o  libDw_fltk_a-fltkviewport.o
	$(RANLIB) libDw-fltk.a

libDw_fltk_a-fltkcomplexbutton.o: fltkcomplexbutton.cc fltkcomplexbutton.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkcomplexbutton.o fltkcomplexbutton.cc

libDw_fltk_a-fltkflatview.o: fltkflatview.cc fltkflatview.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkflatview.o fltkflatview.cc

libDw_fltk_a-fltkimgbuf.o: fltkimgbuf.cc fltkimgbuf.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkimgbuf.o fltkimgbuf.cc

libDw_fltk_a-fltkmisc.o: fltkmisc.cc fltkmisc.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkmisc.o fltkmisc.cc

libDw_fltk_a-fltkplatform.o: fltkplatform.cc fltkplatform.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkplatform.o fltkplatform.cc

libDw_fltk_a-fltkpreview.o: fltkpreview.cc fltkpreview.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkpreview.o fltkpreview.cc

libDw_fltk_a-fltkui.o: fltkui.cc fltkui.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkui.o fltkui.cc

libDw_fltk_a-fltkviewbase.o: fltkviewbase.cc fltkviewbase.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkviewbase.o fltkviewbase.cc

libDw_fltk_a-fltkviewport.o: fltkviewport.cc fltkviewport.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c -o libDw_fltk_a-fltkviewport.o fltkviewport.cc

libDw-widgets.a: alignedtextblock.o bullet.o hyphenator.o image.o listitem.o ruler.o table.o tablecell.o textblock.o textblock_iterator.o textblock_linebreaking.o
	$(AR) $(ARFLAGS) libDw-widgets.a alignedtextblock.o bullet.o hyphenator.o image.o listitem.o ruler.o table.o tablecell.o textblock.o textblock_iterator.o textblock_linebreaking.o
	$(RANLIB) libDw-widgets.a

alignedtextblock.o: alignedtextblock.cc alignedtextblock.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c alignedtextblock.cc

bullet.o: bullet.cc bullet.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c bullet.cc

hyphenator.o: hyphenator.cc hyphenator.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c hyphenator.cc

image.o: image.cc image.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c image.cc

listitem.o: listitem.cc listitem.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c listitem.cc

ruler.o: ruler.cc ruler.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c ruler.cc

table.o: table.cc table.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c table.cc

tablecell.o: tablecell.cc tablecell.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c tablecell.cc

textblock.o: textblock.cc textblock.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c textblock.cc

textblock_iterator.o: textblock_iterator.cc textblock_iterator.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c textblock_iterator.cc

textblock_linebreaking.o: textblock_linebreaking.cc textblock_linebreaking.hh
	$(CXXCOMPILE) $(CXXFLAGS_EXTRA) -c textblock_linebreaking.cc

clean:
	rm -f *.a *.o

install:
