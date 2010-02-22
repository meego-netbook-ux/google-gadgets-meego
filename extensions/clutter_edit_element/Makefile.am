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

INCLUDES		= -I$(top_builddir) \
			  -I$(top_srcdir)

noinst_HEADERS		= clutter_edit_element.h \
			  clutter_edit_impl.h

extension_LTLIBRARIES	= clutter-edit-element.la
extensiondir		= $(GGL_MODULE_DIR)

clutter_edit_element_la_SOURCES = \
			  clutter_edit_element.cc \
			  clutter_edit_impl.cc

clutter_edit_element_la_CXXFLAGS = \
			  $(DEFAULT_COMPILE_FLAGS)

clutter_edit_element_la_CPPFLAGS = \
			  @CLUTTER_CFLAGS@ \
			  $(PREDEFINED_MACROS)

clutter_edit_element_la_LIBADD = \
			  @CLUTTER_LIBS@ \
			  $(top_builddir)/ggadget/libggadget@GGL_EPOCH@.la \
			  $(top_builddir)/ggadget/clutter/libggadget-clutter-1.0.la

clutter_edit_element_la_LDFLAGS = \
			  -no-undefined \
			  -avoid-version \
			  -module
