include ../Makefile.options

libDlib.a: dlib.o
	$(AR) $(ARFLAGS) libDlib.a dlib.o
	$(RANLIB) libDlib.a

dlib.o: dlib.h dlib.c
	$(COMPILE) -c dlib.c

all: libDlib.a

clean:
	rm -f *.o *.a

install:
uninstall:
