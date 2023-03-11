include ../Makefile.options

all:

clean:

install:
	$(INSTALL_SH) -c -d "$(DOC_PATH)"
	$(INSTALL) -c -m 644 user_help.html "$(DOC_PATH)"
	$(INSTALL) -c -m 644 Cookies.txt "$(DOC_PATH)"
	$(INSTALL) -c -m 644 speeddial.html "$(DOC_PATH)"
	$(INSTALL_SH) -c -d "$(DOC_PATH)"/speeddial_icons/
	$(INSTALL) -c -m 644 speeddial_icons/*.png "$(DOC_PATH)"/speeddial_icons/
	$(INSTALL_SH) -c -d "$(MAN_PATH)"
	$(INSTALL) -c -m 644 dillo.1 "$(MAN_PATH)"

uninstall:
	rm -f "$(DOC_PATH)/*.html"
	rm -f "$(DOC_PATH)/*.txt"
	rm -f "$(DOC_PATH)"/speeddial_icons/*.png
	rm -f "$(MAN_PATH)/dillo.1"
