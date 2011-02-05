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

-include ../Makefile.include

CFLAGS += -I.
LDFLAGS += -L. -lfermat -lm -lrt

BIN_TARGETS = fer-gsrm fer-qdelaunay

TARGETS = libfermat.a
OBJS  = alloc.o timer.o parse.o
OBJS += vec4.o vec3.o vec2.o
OBJS += mat4.o mat3.o
OBJS += pc.o pc-internal.o
OBJS += predicates.o
OBJS += cubes3.o nearest-linear.o
OBJS += mesh3.o qhull.o
OBJS += gsrm.o

ifeq '$(USE_GSL)' 'yes'
  OBJS += mat.o vec.o
  CFLAGS += $(GSL_CFLAGS)
endif

OBJS := $(foreach obj,$(OBJS),.objs/$(obj))
BIN_TARGETS := $(foreach target,$(BIN_TARGETS),bin/$(target))

all: $(TARGETS) $(BIN_TARGETS)

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
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

python:
	$(MAKE) -C python

.PHONY: all clean check check-valgrind help
