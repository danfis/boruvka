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

CFLAGS += -I..
CFLAGS += $(PYTHON_CFLAGS)
CFLAGS += -fPIC
LDFLAGS += -shared
LDFLAGS += -L.. -lfermat -lm -lrt
LDFLAGS += $(PYTHON_LDFLAGS)

CYTHON_FLAGS = -3


ifeq '$(USE_OPENCL)' 'yes'
  CFLAGS  += $(OPENCL_CFLAGS)
  LDFLAGS += $(OPENCL_LDFLAGS)
endif
ifeq '$(USE_RAPID)' 'yes'
  CFLAGS  += $(RAPID_CFLAGS)
  LDFLAGS += $(RAPID_LDFLAGS)
endif


TARGETS = fermat.so
MODULES = fermat

OBJS := $(foreach m,$(MODULES),.objs/$(m).o)
SRCS := $(foreach m,$(MODULES),$(m).c)

all: $(TARGETS)

fermat.so: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)


.objs/%.o: %.c src/%.c ../src/%.c ../fermat/%.h ../fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.c: %.pyx
	$(CYTHON) $(CYTHON_FLAGS) -o $@ $<

fermat.pyx: core.pxd rand.pxd rand_mt.pxd timer.pxd
	touch $@


install:
	echo "TODO"

clean:
	rm -f .objs/*.o
	rm -f $(SRCS)
	rm -f $(TARGETS)

	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind


.PHONY: all clean
