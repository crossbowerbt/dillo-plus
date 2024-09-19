include ../Makefile.options

all:

clean:

install:
	$(INSTALL_SH) -c -d "$(APP_PATH)"
	$(INSTALL_SH) -c -d "$(ICON_PATH)"
	$(INSTALL) -c -m 644 dillo-plus.desktop "$(APP_PATH)"
	$(INSTALL) -c -m 644 icons/dillo-plus.png "$(ICON_PATH)"

uninstall:
	rm -f "$(APP_PATH)/dillo-plus.desktop"
	rm -f "$(ICON_PATH)/dillo-plus.png"
