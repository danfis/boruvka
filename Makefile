###
# fermat
# -------
# Copyright (c)2010-2011 Daniel Fiser <danfis@danfis.cz>
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

CFLAGS += -I.
LDFLAGS += -L. -lfermat -lm -lrt

BIN_TARGETS  = fer-gsrm fer-qdelaunay
BIN_TARGETS += fer-gng-2d fer-gng-3d fer-plan-2d
BIN_TARGETS += fer-gngp

TARGETS = libfermat.a
OBJS  = alloc.o timer.o parse.o
OBJS += vec4.o vec3.o vec2.o vec.o
OBJS += mat4.o mat3.o
OBJS += pc2.o pc3.o pc4.o pc-internal.o
OBJS += predicates.o
OBJS += cubes2.o cubes3.o nncells.o nearest-linear.o
OBJS += mesh3.o qhull.o net.o
OBJS += fibo.o pairheap.o
OBJS += dij.o
OBJS += gsrm.o
OBJS += rand-mt.o
OBJS += gng.o gng2.o gng3.o
OBJS += gng-plan.o

# header files that must be generated
HEADERS  = pc2.h pc3.h pc4.h
HEADERS += cubes2.h cubes3.h
HEADERS += gng2.h gng3.h

OBJS 		:= $(foreach obj,$(OBJS),.objs/$(obj))
HEADERS     := $(foreach h,$(HEADERS),fermat/$(h))
BIN_TARGETS := $(foreach target,$(BIN_TARGETS),bin/$(target))

all: $(TARGETS) $(BIN_TARGETS) $(HEADERS)

libfermat.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

bin/fer-%: bin/%-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

%2.c: %N.c
	$(SED) 's|`N`|2|g' <$< >$@
%3.c: %N.c
	$(SED) 's|`N`|3|g' <$< >$@
%4.c: %N.c
	$(SED) 's|`N`|4|g' <$< >$@

%2.h: %N.h
	$(SED) 's|`N`|2|g' <$< >$@
%3.h: %N.h
	$(SED) 's|`N`|3|g' <$< >$@
%4.h: %N.h
	$(SED) 's|`N`|4|g' <$< >$@

%2d-main.c: %Nd-main.c
	$(SED) 's|`N`|2|g' <$< >$@
%3d-main.c: %Nd-main.c
	$(SED) 's|`N`|3|g' <$< >$@

%.h: fermat/config.h
%.c: fermat/config.h


install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/fermat
	mkdir -p $(PREFIX)/$(LIBDIR)
	cp -r fermat/* $(PREFIX)/$(INCLUDEDIR)/fermat/
	cp libfermat.a $(PREFIX)/$(LIBDIR)

clean:
	rm -f $(OBJS)
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	rm -f fermat/config.h
	rm -f fermat/pc{2,3,4}.h
	rm -f src/pc{2,3,4}.c
	rm -f fermat/cubes{2,3,4}.h
	rm -f src/cubes{2,3,4}.c
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

python:
	$(MAKE) -C python


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
