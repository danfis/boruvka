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

TARGETS = libfermat.a gsrm
TARGETS += fer-qdelaunay
OBJS = alloc.o vec4.o vec3.o vec2.o parse.o \
       point_cloud.o point_cloud_internal.o \
       mat3.o mat4.o predicates.o timer.o
OBJS += nearest/cubes3.o
OBJS += mesh/mesh3.o mesh/qhull.o
OBJS += surface/gsrm.o
#OBJS += gann/gann.o
OBJS += gann/net.o

ifeq '$(USE_GSL)' 'yes'
  OBJS += mat.o vec.o
  CFLAGS += $(GSL_CFLAGS)
endif

all: $(TARGETS)

libfermat.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

gsrm: surface/gsrm-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
fer-qdelaunay: mesh/qdelaunay-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

%.o: %.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
nearest/%.o: nearest/%.c fermat/nearest/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
mesh/%.o: mesh/%.c fermat/mesh/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
surface/%.o: surface/%.c fermat/surface/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
gann/%.o: gann/%.c fermat/gann/%.h fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
%.o: %.c fermat/config.h
	$(CC) $(CFLAGS) $(DEFS) -c -o $@ $<
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
	rm -f fermat/config.h
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

.PHONY: all clean check check-valgrind help
