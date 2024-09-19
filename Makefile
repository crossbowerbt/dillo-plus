include Makefile.options

all: config.h
	@echo Making all in lout
	@(cd lout; make all)
	@echo Making all in dw
	@(cd dw; make all)
	@echo Making all in dlib
	@(cd dlib; make all)
	@echo Making all in dpip
	@(cd dpip; make all)
	@echo Making all in src
	@(cd src; make all)
	@echo Making all in doc
	@(cd doc; make all)
	@echo Making all in dist
	@(cd dist; make all)
	@echo Making all in dpid
	@(cd dpid; make all)
	@echo Making all in dpi
	@(cd dpi; make all)

test:
	@echo Making all in test
	@(cd test; make all)

clean:
	@echo Cleaning in lout
	@(cd lout; make clean)
	@echo Cleaning in dw
	@(cd dw; make clean)
	@echo Cleaning in dlib
	@(cd dlib; make clean)
	@echo Cleaning in dpip
	@(cd dpip; make clean)
	@echo Cleaning in src
	@(cd src; make clean)
	@echo Cleaning in doc
	@(cd doc; make clean)
	@echo Cleaning in dist
	@(cd dist; make clean)
	@echo Cleaning in dpid
	@(cd dpid; make clean)
	@echo Cleaning in dpi
	@(cd dpi; make clean)
	@echo Cleaning in test
	@(cd test; make clean)

install: all
	@echo Making install in lout
	@(cd lout; make install)
	@echo Making install in dw
	@(cd dw; make install)
	@echo Making install in dlib
	@(cd dlib; make install)
	@echo Making install in dpip
	@(cd dpip; make install)
	@echo Making install in src
	@(cd src; make install)
	@echo Making install in doc
	@(cd doc; make install)
	@echo Making install in dist
	@(cd dist; make install)
	@echo Making install in dpid
	@(cd dpid; make install)
	@echo Making install in dpi
	@(cd dpi; make install)
	#@echo Making install in test
	#@(cd test; make install)
	./install-sh -c -d "$(DILLO_BINDIR)"
	$(INSTALL) -c dillo-install-hyphenation "$(DILLO_BINDIR)"
	./install-sh -c -d "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 dillorc "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 bm.txt "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 style.css "$(DILLO_ETCDIR)"
	$(INSTALL) -c -m 644 style_reader_mode.css "$(DILLO_ETCDIR)"

uninstall:
	@echo Making uninstall in lout
	@(cd lout; make uninstall)
	@echo Making uninstall in dw
	@(cd dw; make uninstall)
	@echo Making uninstall in dlib
	@(cd dlib; make uninstall)
	@echo Making uninstall in dpip
	@(cd dpip; make uninstall)
	@echo Making uninstall in src
	@(cd src; make uninstall)
	@echo Making uninstall in doc
	@(cd doc; make uninstall)
	@echo Making uninstall in dist
	@(cd dist; make uninstall)
	@echo Making uninstall in dpid
	@(cd dpid; make uninstall)
	@echo Making uninstall in dpi
	@(cd dpi; make uninstall)
	#@echo Making uninstall in test
	#@(cd test; make uninstall)
	rm -f "$(DILLO_BINDIR)/dillo-install-hyphenation"
	rm -f "$(DILLO_ETCDIR)/dillorc"
	rm -f "$(DILLO_ETCDIR)/bm.txt"
	rm -f "$(DILLO_ETCDIR)/style.css"
	rm -f "$(DILLO_ETCDIR)/style_reader_mode.css"
