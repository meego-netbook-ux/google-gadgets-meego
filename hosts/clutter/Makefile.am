#
# Copyright 2008 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

INCLUDES			= -I$(top_builddir) \
			  	-I$(top_srcdir)

bin_PROGRAMS			= ggl-clutter

ggl_clutter_SOURCES		= main.cc \
				simple_clutter_host.cc \
				simple_clutter_host.h \
				gadget_browser_host.h

ggl_clutter_CPPFLAGS 		= @CLUTTER_CFLAGS@ \
			  	$(PREDEFINED_MACROS) \
			  	-DPIXMAP_DIR=\"$(GGL_RESOURCE_DIR)/\" \
			  	-DGGL_APP_NAME=\"ggl-clutter\"

ggl_clutter_CXXFLAGS		= $(DEFAULT_COMPILE_FLAGS)

ggl_clutter_LDADD		= @CLUTTER_LIBS@ \
				$(top_builddir)/ggadget/libggadget@GGL_EPOCH@.la \
		  		$(top_builddir)/ggadget/clutter/libggadget-clutter@GGL_EPOCH@.la

if GGL_HAVE_MPL
ggl_clutter_CPPFLAGS += $(MPL_CFLAGS)
ggl_clutter_LDADD +=  $(MPL_LIBS)
endif

ggl_clutter_LDFLAGS = -R$(libdir)

pixmapdir = $(GGL_RESOURCE_DIR)
BITMAPS = control.png \
	  control-hover.png \
	  trash-normal.png \
	  trash-colored.png \
	  gadget-normal.png \
	  gadget-colored.png \
	  gadgets-background.png

pixmap_DATA = $(BITMAPS) gadgets-panel.css

%.css: %.css.in $(top_builddir)/config.log
	$(QUIET_GEN)sed -e "s|\@gglresdir\@|$(GGL_RESOURCE_DIR)|" $< > $@

%.service: %.service.in
	$(QUIET_GEN)sed -e "s|\@bindir\@|$(bindir)|" $< > $@

desktop_in_files = gadgets-panel.desktop.in.in
desktop_data = $(desktop_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

paneldir = @PANEL_DIR@
panel_DATA = $(desktop_data)

servicedir = $(datadir)/dbus-1/services
service_in_files = org.moblin.UX.Shell.Panels.gadgets.service.in
service_DATA = $(service_in_files:.service.in=.service)

EXTRA_DIST = gadgets-panel.css.in $(service_in_files) $(BITMAPS)
CLEANFILES = gadgets-panel.css $(desktop_DATA) $(service_DATA)

# TODO: install these files into correct path.
#all-local:
#	[ ! -f $(top_builddir)/extensions/gtkmoz_browser_element/gtkmoz-browser-child ] || cp $(top_builddir)/extensions/gtkmoz_browser_element/gtkmoz-browser-child .
#	[ ! -f $(top_builddir)/resources/resources.gg ] || cp $(top_builddir)/resources/resources.gg .

noinst_SCRIPTS=ggl-clutter.sh

.PHONY: ggl-clutter.sh

ggl-clutter.sh: ggl-clutter
	(echo '#!/bin/sh' > $@; \
	 echo 'GGL_MODULE_PATH=`pwd`' >> $@; \
	 echo 'for i in $(abs_top_builddir)/extensions/*; do' >> $@; \
	 echo '  if test -d $$i; then' >> $@; \
	 echo '    GGL_MODULE_PATH=$$GGL_MODULE_PATH:$$i' >> $@; \
	 echo '  fi' >> $@; \
	 echo 'done' >> $@; \
	 echo 'export GGL_MODULE_PATH' >> $@; \
	 echo '$(LIBTOOL) --mode=execute $$MEMCHECK_COMMAND $(abs_builddir)/ggl-clutter $$@' >> $@; \
	 chmod 0755 $@)

# post-install hook to remove the .la and .a files we are not interested in
# (There is no way to stop libtool generating static libs locally, and we
# cannot do this globally because of libmetacity-private.so).
install-exec-hook:
	rm -f $(DESTDIR)$(pkglibdir)/*.a
	rm -f $(DESTDIR)$(pkglibdir)/*.la
