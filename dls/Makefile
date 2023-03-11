include ../Makefile.options

all: default.dls.in
	sed -e "s|[@]libdir[@]|$(LIBDIR)|" default.dls.in > default.dls
	chmod +x default.dls

clean:
	rm -f default.dls

install: all
	$(INSTALL_SH) -c -d "$(DILLO_LIBDIR)/dls/"
	$(INSTALL) -c default.dls "$(DILLO_LIBDIR)/dls/"
	$(INSTALL) -c test.dls "$(DILLO_LIBDIR)/dls/"
	$(INSTALL) -c not_executable.dls "$(DILLO_LIBDIR)/dls/"
	chmod -x "$(DILLO_LIBDIR)/dls/not_executable.dls"
	$(INSTALL) -c zip.dls "$(DILLO_LIBDIR)/dls/"
	$(INSTALL) -c epub.dls "$(DILLO_LIBDIR)/dls/"
	$(INSTALL) -c rss.dls "$(DILLO_LIBDIR)/dls/"

uninstall:
	rm -f "$(DILLO_LIBDIR)/dls/default.dls"
	rm -f "$(DILLO_LIBDIR)/dls/test.dls"
	rm -f "$(DILLO_LIBDIR)/dls/not_executable.dls"
	rm -f "$(DILLO_LIBDIR)/dls/zip.dls"
	rm -f "$(DILLO_LIBDIR)/dls/epub.dls"
	rm -f "$(DILLO_LIBDIR)/dls/rss.dls"
