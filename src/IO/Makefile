include ../../Makefile.options

INCLUDES_EXTRA = -I../..
CXXFLAGS_EXTRA = -DDILLO_BINDIR='"$(DILLO_BINDIR)"' -DCA_CERTS_FILE='"$(CA_CERTS_FILE)"' -DCA_CERTS_DIR='"$(CA_CERTS_DIR)"' -DDOC_PATH='"$(DOC_PATH)"' -DBINNAME='"$(BINNAME)"'

all: libDiof.a

libDiof.a: mime.o about.o http.o dpi.o IO.o iowatch.o proto.o tls.o
	$(AR) $(ARFLAGS) libDiof.a mime.o about.o http.o dpi.o IO.o iowatch.o proto.o tls.o
	$(RANLIB) libDiof.a

mime.o: mime.c mime.h
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c mime.c

about.o: about.c ../../config.h
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c about.c

http.o: http.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c http.c

proto.o: proto.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c proto.c

tls.o: tls.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c tls.c

dpi.o: dpi.c
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c dpi.c

IO.o: IO.c IO.h
	$(COMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CFLAGS) -c IO.c

iowatch.o: iowatch.cc iowatch.hh
	$(CXXCOMPILE) $(INCLUDES_EXTRA) $(CXXFLAGS_EXTRA) $(LIBFLTK_CXXFLAGS) -c iowatch.cc

clean:
	rm -f *.o *.a

install:
