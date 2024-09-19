include ../Makefile.options

all: dw-anchors-test dw-example dw-find-test dw-float-test dw-links dw-links2 dw-image-background dw-images-simple dw-images-scaled dw-images-scaled2 dw-lists dw-simple-container-test dw-table-aligned dw-table dw-border-test dw-imgbuf-mem-test identity dw-ui-test dw-resource-test containers shapes cookies liang trie notsosimplevector unicode-test

dw_anchors_test.o: dw_anchors_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_anchors_test.cc

dw-anchors-test: dw_anchors_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-anchors-test dw_anchors_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_example.o: dw_example.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_example.cc

dw-example: dw_example.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-example dw_example.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_find_test.o: dw_find_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_find_test.cc

dw-find-test: dw_find_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-find-test dw_find_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_float_test.o: dw_float_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_float_test.cc

dw-float-test: dw_float_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-float-test dw_float_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_links.o: dw_links.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_links.cc

dw-links: dw_links.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-links dw_links.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_links2.o: dw_links2.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_links2.cc

dw-links2: dw_links2.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-links2 dw_links2.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_image_background.o: dw_image_background.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_image_background.cc

dw-image-background: dw_image_background.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-image-background dw_image_background.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_images_simple.o: dw_images_simple.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_images_simple.cc

dw-images-simple: dw_images_simple.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-images-simple dw_images_simple.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_images_scaled.o: dw_images_scaled.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_images_scaled.cc

dw-images-scaled: dw_images_scaled.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-images-scaled dw_images_scaled.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_images_scaled2.o: dw_images_scaled2.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_images_scaled2.cc

dw-images-scaled2: dw_images_scaled2.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-images-scaled2 dw_images_scaled2.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_lists.o: dw_lists.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_lists.cc

dw-lists: dw_lists.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-lists dw_lists.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_simple_container.o: dw_simple_container.cc dw_simple_container.hh
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_simple_container.cc

dw_simple_container_test.o: dw_simple_container_test.cc dw_simple_container.hh
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_simple_container_test.cc

dw-simple-container-test: dw_simple_container_test.o dw_simple_container.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-simple-container-test dw_simple_container_test.o dw_simple_container.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_table_aligned.o: dw_table_aligned.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_table_aligned.cc

dw-table-aligned: dw_table_aligned.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-table-aligned dw_table_aligned.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_table.o: dw_table.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_table.cc

dw-table: dw_table.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-table dw_table.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_border_test.o: dw_border_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_border_test.cc

dw-border-test: dw_border_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-border-test dw_border_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_imgbuf_mem_test.o: dw_imgbuf_mem_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_imgbuf_mem_test.cc

dw-imgbuf-mem-test: dw_imgbuf_mem_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-imgbuf-mem-test dw_imgbuf_mem_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_resource_test.o: dw_resource_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_resource_test.cc

dw-resource-test: dw_resource_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-resource-test dw_resource_test.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw_ui_test.o: dw_ui_test.cc form.hh
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c dw_ui_test.cc

form.o: form.cc form.hh
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c form.cc

identity.o: identity.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c identity.cc

identity: identity.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o identity identity.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

dw-ui-test: dw_ui_test.o form.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o dw-ui-test dw_ui_test.o form.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

containers.o: containers.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c containers.cc

containers: containers.o ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o containers containers.o ../lout/liblout.a

shapes.o: shapes.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c shapes.cc

shapes: shapes.o ../dw/libDw-core.a ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o shapes shapes.o ../dw/libDw-core.a ../lout/liblout.a

cookies.o: cookies.c
	$(COMPILE) $(LIBFLTK_CXXFLAGS) -DBINNAME='"$(BINNAME)"' -c cookies.c

cookies: cookies.o ../dpip/libDpip.a ../dlib/libDlib.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o cookies cookies.o ../dpip/libDpip.a ../dlib/libDlib.a

liang.o: liang.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c liang.cc

liang: liang.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o liang liang.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

trie.o: trie.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c trie.cc

trie: trie.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o trie trie.o ../dw/libDw-widgets.a  ../dw/libDw-fltk.a  ../dw/libDw-core.a  ../lout/liblout.a

notsosimplevector.o: notsosimplevector.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c notsosimplevector.cc

notsosimplevector: notsosimplevector.o ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o notsosimplevector notsosimplevector.o ../lout/liblout.a

unicode_test.o: unicode_test.cc
	$(CXXCOMPILE) $(LIBFLTK_CXXFLAGS) -c unicode_test.cc

unicode-test: unicode_test.o ../lout/liblout.a
	$(CXXCOMPILE) $(LIBFLTK_LDFLAGS) -o unicode-test unicode_test.o ../lout/liblout.a

clean:
	rm -f *.o
	rm -f dw-anchors-test dw-example dw-find-test dw-float-test dw-links dw-links2 dw-image-background dw-images-simple dw-images-scaled dw-images-scaled2 dw-lists dw-simple-container-test dw-table-aligned dw-table dw-border-test dw-imgbuf-mem-test identity dw-ui-test dw-resource-test containers shapes cookies liang trie notsosimplevector unicode-test

install:
uninstall:
