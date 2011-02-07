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

CFLAGS += -I. -I../fermat
LDFLAGS += -L. -lgann -L../fermat -lfermat -lm -lrt

BIN_TARGETS = gann-gng-3d

TARGETS = libgann.a
OBJS  = net.o
OBJS += gng.o gng3.o

OBJS := $(foreach obj,$(OBJS),.objs/$(obj))
BIN_TARGETS := $(foreach target,$(BIN_TARGETS),bin/$(target))

all: $(TARGETS) $(BIN_TARGETS)

libgann.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

bin/gann-%: bin/%-main.c libgann.a ../fermat/libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

.objs/%.o: src/%.c gann/%.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<


install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/gann
	mkdir -p $(PREFIX)/$(LIBDIR)
	cp -r gann/* $(PREFIX)/$(INCLUDEDIR)/gann/
	cp libgann.a $(PREFIX)/$(LIBDIR)

clean:
	rm -f $(OBJS)
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

python:
	$(MAKE) -C python

.PHONY: all clean check check-valgrind help
