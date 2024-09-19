include ../Makefile.options

libDpip.a: dpip.o
	$(AR) $(ARFLAGS) libDpip.a dpip.o
	$(RANLIB) libDpip.a

dpip.o: dpip.h dpip.c
	$(COMPILE) -DBINNAME='"$(BINNAME)"' -c dpip.c

all: libDpip.a

clean:
	rm -f *.o *.a

install:
uninstall:
