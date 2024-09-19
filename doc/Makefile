include ../Makefile.options

all: manpage.1.in
	sed "s/%BINNAME%/$(BINNAME)/g" manpage.1.in > $(BINNAME).1

clean:
	rm -f $(BINNAME).1

install: all
	$(INSTALL_SH) -c -d "$(DOC_PATH)"
	$(INSTALL) -c -m 644 user_help.html "$(DOC_PATH)"
	$(INSTALL) -c -m 644 Cookies.txt "$(DOC_PATH)"
	$(INSTALL_SH) -c -d "$(MAN_PATH)"
	$(INSTALL) -c -m 644 $(BINNAME).1 "$(MAN_PATH)"

uninstall:
	rm -f "$(DOC_PATH)/*.html"
	rm -f "$(DOC_PATH)/*.txt"
	rm -f "$(MAN_PATH)/$(BINNAME).1"
