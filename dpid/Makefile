include ../Makefile.options

CFLAGS_EXTRA = -DDPIDRC_SYS='"$(DPIDRC_SYS)"' -DBINNAME='"$(BINNAME)"'

all: dpid-plus dpidc-plus dpidrc

dpid-plus: dpi.o dpi_socket_dir.o  dpid-plus.o dpid_common.o main.o  misc_new.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(LINK) dpi.o dpi_socket_dir.o  dpid-plus.o dpid_common.o main.o  misc_new.o ../dpip/libDpip.a ../dlib/libDlib.a

dpidc-plus: dpidc-plus.o ../dpip/libDpip.a  ../dlib/libDlib.a 
	$(LINK) dpidc-plus.o ../dpip/libDpip.a  ../dlib/libDlib.a

dpidrc: dpidrc.in
	sed -e "s|[@]libdir[@]|$(LIBDIR)|" -e "s|[@]binname[@]|$(BINNAME)|" ./dpidrc.in > dpidrc

dpi.o: dpi.h dpi.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpi.c

dpi_socket_dir.o: dpi_socket_dir.h dpi_socket_dir.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpi_socket_dir.c

dpid-plus.o: dpid-plus.h dpid-plus.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpid-plus.c

dpid_common.o: dpid_common.h dpid_common.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpid_common.c

main.o: main.c
	$(COMPILE) $(CFLAGS_EXTRA) -c main.c

misc_new.o: misc_new.h misc_new.c
	$(COMPILE) $(CFLAGS_EXTRA) -c misc_new.c

dpidc-plus.o: dpidc-plus.c
	$(COMPILE) $(CFLAGS_EXTRA) -c dpidc-plus.c

clean:
	rm -f *.o dpid-plus dpidc-plus dpidrc

install: all
	$(INSTALL_SH) -c -d "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 dpidrc "$(DPIDRC_SYS)"
	$(INSTALL_SH) -c -d "$(DILLO_BINDIR)"
	$(INSTALL) -c dpid-plus "$(DILLO_BINDIR)"
	$(INSTALL) -c dpidc-plus "$(DILLO_BINDIR)"

uninstall:
	rm -f "$(DPIDRC_SYS)"
	rm -f "$(DILLO_BINDIR)/dpid-plus"
	rm -f "$(DILLO_BINDIR)/dpidc-plus"
