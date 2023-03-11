include ../../Makefile.options

INCLUDES_EXTRA = -I../..
CXXFLAGS_EXTRA = -DDILLO_BINDIR='"$(DILLO_BINDIR)"'

all: libDiof.a

libDiof.a: mime.o about.o http.o dpi.o IO.o iowatch.o
	$(AR) $(ARFLAGS) libDiof.a mime.o about.o http.o dpi.o IO.o iowatch.o
	$(RANLIB) libDiof.a

mime.o: mime.c mime.h
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c mime.c

about.o: about.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c about.c

http.o: http.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c http.c

dpi.o: dpi.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c dpi.c

IO.o: IO.c IO.h
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c IO.c

iowatch.o: iowatch.cc iowatch.hh
	$(CXXCOMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c iowatch.cc

clean:
	rm -f *.o *.a

install:
