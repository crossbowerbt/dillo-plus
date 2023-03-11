include ../Makefile.options

all:

clean:

install:
	$(INSTALL_SH) -c -d "$(DOC_PATH)"
	$(INSTALL) -c -m 644 user_help.html "$(DOC_PATH)"
	$(INSTALL_SH) -c -d "$(MAN_PATH)"
	$(INSTALL) -c -m 644 dillo.1 "$(MAN_PATH)"

uninstall:
	rm -f "$(DOC_PATH)/user_help.html"
	rm -f "$(MAN_PATH)/dillo.1"
