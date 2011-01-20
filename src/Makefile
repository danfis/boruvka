###
# fermat
# -------
# Copyright (c)2010 Daniel Fiser <danfis@danfis.cz>
#
#  This file is part of fermat.
#
#  Distributed under the OSI-approved BSD License (the "License");
#  see accompanying file BDS-LICENSE for details or see
#  <http://www.opensource.org/licenses/bsd-license.php>.
#
#  This software is distributed WITHOUT ANY WARRANTY; without even the
#  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#  See the License for more information.
##

-include Makefile.include

CFLAGS += -I. -g

TARGETS = libfermat.a
OBJS = alloc.o vec4.o vec3.o vec2.o parse.o \
       point_cloud.o point_cloud_internal.o \
       mat3.o mat4.o

ifeq '$(USE_GSL)' 'yes'
  OBJS += mat.o
endif

all: $(TARGETS)

libfermat.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

%.o: %.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
%.o: %.c fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
%.h: fermat/config.h
%.c: fermat/config.h

install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/fermat
	mkdir -p $(PREFIX)/$(LIBDIR)
	cp fermat/*.h $(PREFIX)/$(INCLUDEDIR)/fermat/
	cp libfermat.a $(PREFIX)/$(LIBDIR)

clean:
	rm -f $(OBJS)
	rm -f $(TARGETS)
	rm -f fermat/config.h
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

help:
	@echo "Targets:"
	@echo "    all     - Build library"
	@echo "    install - Install library into system"
	@echo ""
	@echo "Options:"
	@echo "    CC - Path to C compiler"
	@echo "    M4 - Path to m4 macro processor"
	@echo ""
	@echo "    DEBUG 'yes'/'no'      - Turn on/off debugging (default: 'no')"
	@echo "    PROFIL 'yes'/'no'     - Compiles profiling info (default: 'no')"
	@echo "    NOWALL 'yes'/'no'     - Turns off -Wall gcc option (default: 'no')"
	@echo "    NOPEDANTIC 'yes'/'no' - Turns off -pedantic gcc option (default: 'no')"
	@echo ""
	@echo "    USE_SINGLE 'yes' - Use single precision (default: 'no')"
	@echo "    USE_DOUBLE 'yes' - Use double precision (default: 'yes')"
	@echo ""
	@echo "    PREFIX     - Prefix where library will be installed (default: /usr/local)"
	@echo "    INCLUDEDIR - Directory where header files will be installed (PREFIX/INCLUDEDIR) (default: include)"
	@echo "    LIBDIR     - Directory where library will be installed (PREFIX/LIBDIR) (default: lib)"
	@echo ""

.PHONY: all clean check check-valgrind help
