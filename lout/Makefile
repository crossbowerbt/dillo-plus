include ../Makefile.options

all: liblout.a

liblout.a: container.o identity.o misc.o object.o signal.o unicode.o
	$(AR) $(ARFLAGS) liblout.a container.o identity.o misc.o object.o signal.o unicode.o
	$(RANLIB) liblout.a

container.o: container.cc container.hh
	$(CXXCOMPILE) -c container.cc

identity.o: identity.cc identity.hh
	$(CXXCOMPILE) -c identity.cc

misc.o: misc.cc misc.hh
	$(CXXCOMPILE) -c misc.cc

object.o: object.cc object.hh
	$(CXXCOMPILE) -c object.cc

signal.o: signal.cc signal.hh
	$(CXXCOMPILE) -c signal.cc

unicode.o: unicode.cc unicode.hh
	$(CXXCOMPILE) -c unicode.cc

clean:
	rm -f *.o *.a

install:
uninstall:
