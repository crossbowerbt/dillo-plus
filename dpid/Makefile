include ../Makefile.options

CFLAGS_EXTRA = -DDPIDRC_SYS='"$(DPIDRC_SYS)"' -DBINNAME='"$(BINNAME)"'

all: dpid dpidc dpidrc

dpid: dpi.o dpi_socket_dir.o  dpid.o dpid_common.o main.o  misc_new.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(LINK) dpi.o dpi_socket_dir.o  dpid.o dpid_common.o main.o  misc_new.o ../dpip/libDpip.a ../dlib/libDlib.a

dpidc: dpidc.o ../dpip/libDpip.a  ../dlib/libDlib.a 
	$(LINK) dpidc.o ../dpip/libDpip.a  ../dlib/libDlib.a

dpidrc: dpidrc.in
	sed -e "s|[@]libdir[@]|$(LIBDIR)|" -e "s|[@]binname[@]|$(BINNAME)|" ./dpidrc.in > dpidrc

dpi.o: dpi.h dpi.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpi.c

dpi_socket_dir.o: dpi_socket_dir.h dpi_socket_dir.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpi_socket_dir.c

dpid.o: dpid.h dpid.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpid.c

dpid_common.o: dpid_common.h dpid_common.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpid_common.c

main.o: main.c
	$(COMPILE) $(CFLAGS_EXTRA) -c main.c

misc_new.o: misc_new.h misc_new.c
	$(COMPILE) $(CFLAGS_EXTRA) -c misc_new.c

dpidc.o: dpidc.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpidc.c

clean:
	rm -f *.o dpid dpidc dpidrc

install: all
	$(INSTALL_SH) -c -d "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 dpidrc "$(DPIDRC_SYS)"
	$(INSTALL_SH) -c -d "$(DILLO_BINDIR)"
	$(INSTALL) -c dpid "$(DILLO_BINDIR)"
	$(INSTALL) -c dpidc "$(DILLO_BINDIR)"

uninstall:
	rm -f "$(DPIDRC_SYS)"
	rm -f "$(DILLO_BINDIR)/dpid"
	rm -f "$(DILLO_BINDIR)/dpidc"
