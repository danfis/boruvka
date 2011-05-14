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
CXXFLAGS += -I.
LDFLAGS += -L. -lfermat -lm -lrt

ifeq '$(USE_OPENCL)' 'yes'
  CFLAGS += $(OPENCL_CFLAGS)
endif
ifeq '$(USE_RAPID)' 'yes'
  CFLAGS += $(RAPID_CFLAGS)
endif

BIN_TARGETS  = fer-gsrm fer-qdelaunay
BIN_TARGETS += fer-gng-2d fer-gng-3d fer-plan-2d
BIN_TARGETS += fer-gngp2 fer-gngp3 fer-prm-2d fer-rrt-2d

TARGETS = libfermat.a
OBJS  = alloc.o timer.o parse.o
OBJS += vec4.o vec3.o vec2.o vec.o
OBJS += mat4.o mat3.o
OBJS += pc2.o pc3.o pc4.o pc-internal.o
OBJS += predicates.o
OBJS += cubes2.o cubes3.o nncells.o nearest-linear.o
OBJS += mesh3.o qhull.o net.o chull3.o
OBJS += fibo.o pairheap.o
OBJS += dij.o
OBJS += gsrm.o
OBJS += rand-mt.o
OBJS += gng.o gng2.o gng3.o
OBJS += gng-plan.o prm.o rrt.o
OBJS += tasks.o
OBJS += cd-box.o cd-sphere.o cd-cyl.o cd-trimesh.o cd-shape.o
OBJS += cd-obb.o cd-geom.o cd-collide.o cd-cd.o cd-parse.o
OBJS += cd-sphere-grid.o
OBJS += ccd.o ccd-polytope.o ccd-mpr.o ccd-support.o

ifeq '$(USE_OPENCL)' 'yes'
  OBJS += opencl.o
  OBJS += surf-matching.o
endif
ifeq '$(USE_RAPID)' 'yes'
  OBJSPP = trimesh.cpp.o
  BIN_TARGETS += fer-gngp-alpha fer-print-alpha
endif

# header files that must be generated
HEADERS  = pc2.h pc3.h pc4.h
HEADERS += cubes2.h cubes3.h
HEADERS += gng2.h gng3.h

OBJS 		:= $(foreach obj,$(OBJS),.objs/$(obj))
OBJSPP 		:= $(foreach obj,$(OBJSPP),.objs/$(obj))
HEADERS     := $(foreach h,$(HEADERS),fermat/$(h))
BIN_TARGETS := $(foreach target,$(BIN_TARGETS),bin/$(target))

all: $(TARGETS) $(BIN_TARGETS) $(HEADERS)

libfermat.a: $(OBJS) $(OBJSPP)
	ar cr $@ $(OBJS) $(OBJSPP)
	ranlib $@

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

bin/fer-gngp-alpha: bin/gngp-alpha-main.c libfermat.a
	$(CC) $(CFLAGS) $(RAPID_CFLAGS) -o $@ $< $(LDFLAGS) $(RAPID_LDFLAGS) -lstdc++
bin/fer-print-alpha: bin/print-alpha-main.c libfermat.a
	$(CC) $(CFLAGS) $(RAPID_CFLAGS) -o $@ $< $(LDFLAGS) $(RAPID_LDFLAGS) -lstdc++

bin/fer-%: bin/%-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

src/surf-matching.c: src/surf-matching-cl.c
	touch $@

.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<

.objs/%.cpp.o: src/%.c fermat/%.h fermat/config.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<
.objs/%.cpp.o: src/%.c fermat/config.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<


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

%-cl.c: %.cl
	$(PYTHON) ./scripts/cl-to-c.py opencl_program <$< >$@


install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/fermat
	mkdir -p $(PREFIX)/$(LIBDIR)
	cp -r fermat/* $(PREFIX)/$(INCLUDEDIR)/fermat/
	cp libfermat.a $(PREFIX)/$(LIBDIR)

clean:
	rm -f $(OBJS)
	rm -f .objs/*.o
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	rm -f fermat/config.h
	rm -f fermat/pc{2,3,4}.h
	rm -f fermat/gng{2,3}.h
	rm -f src/pc{2,3,4}.c
	rm -f fermat/cubes{2,3,4}.h
	rm -f src/cubes{2,3,4}.c
	rm -f src/*-cl.c
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	if [ -d doc ]; then $(MAKE) -C doc clean; fi;
	if [ -d cython ]; then $(MAKE) -C cython clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

doc:
	$(MAKE) -C doc

cython:
	$(MAKE) -C cython


help:
	@echo "Targets:"
	@echo "    all            - Build library"
	@echo "    cython         - Build python bindings using cython"
	@echo "    doc            - Build documentation"
	@echo "    check          - Build & Run automated tests"
	@echo "    check-valgrind - Build & Run automated tests in valgrind(1)"
	@echo "    clean          - Remove all generated files"
	@echo "    install        - Install library into system"
	@echo "Options:"
	@echo "    CC      - Path to C compiler          (=$(CC))"
	@echo "    CXX     - Path to C++ compiler        (=$(CXX))"
	@echo "    M4      - Path to m4 macro processor  (=$(M4))"
	@echo "    SED     - Path to sed(1)              (=$(SED))"
	@echo "    PYTHON  - Path to python interpret    (=$(PYTHON))"
	@echo "    PYTHON2 - Path to python interpret v2 (=$(PYTHON2))"
	@echo "    PYTHON3 - Path to python interpret v3 (=$(PYTHON3))"
	@echo "    CYTHON  - Path to cython              (=$(CYTHON))"
	@echo ""
	@echo "    CC_NOT_GCC 'yes'/'no' - If set to 'yes' no gcc specific options will be used (=$(CC_NOT_GCC))"
	@echo ""
	@echo "    DEBUG      'yes'/'no' - Turn on/off debugging          (=$(DEBUG))"
	@echo "    PROFIL     'yes'/'no' - Compiles profiling info        (=$(PROFIL))"
	@echo "    NOWALL     'yes'/'no' - Turns off -Wall gcc option     (=$(NOWALL))"
	@echo "    NOPEDANTIC 'yes'/'no' - Turns off -pedantic gcc option (=$(NOPEDANTIC))"
	@echo ""
	@echo "    USE_SINGLE 'yes'      - Use single precision (=$(USE_SINGLE))"
	@echo "    USE_DOUBLE 'yes'      - Use double precision (=$(USE_DOUBLE))"
	@echo "    USE_SSE    'yes'/'no' - Use SSE instructions (=$(USE_SSE))"
	@echo "    USE_OPENCL 'yes'/'no' - Use OpenCL library   (=$(USE_OPENCL))"
	@echo "                            By default, auto detection is used."
	@echo "                            This option depends on USE_SINGLE set to 'yes'"
	@echo "    USE_RAPID  'yes'/'no' - Use RAPID library    (=$(USE_RAPID))"
	@echo "                            By default, auto detection is used."
	@echo ""
	@echo "    PREFIX     - Prefix where library will be installed                             (=$(PREFIX))"
	@echo "    INCLUDEDIR - Directory where header files will be installed (PREFIX/INCLUDEDIR) (=$(INCLUDEDIR))"
	@echo "    LIBDIR     - Directory where library will be installed (PREFIX/LIBDIR)          (=$(LIBDIR))"
	@echo ""
	@echo "Variables:"
	@echo "  Note that most of can be preset or changed by user"
	@echo "    SYSTEM         = "$(SYSTEM)
	@echo "    CFLAGS         = $(CFLAGS)"
	@echo "    CXXFLAGS       = $(CXXFLAGS)"
	@echo "    LDFLAGS        = $(LDFLAGS)"
	@echo "    CONFIG_FLAGS   = $(CONFIG_FLAGS)"
	@echo "    PYTHON_CFLAGS  = $(PYTHON_CFLAGS)"
	@echo "    PYTHON_LDFLAGS = $(PYTHON_LDFLAGS)"
	@echo "    RAPID_CFLAGS   = $(RAPID_CFLAGS)"
	@echo "    RAPID_LDFLAGS  = $(RAPID_LDFLAGS)"
	@echo "    OPENCL_CFLAGS  = $(OPENCL_CFLAGS)"
	@echo "    OPENCL_LDFLAGS = $(OPENCL_LDFLAGS)"

.PHONY: all clean check check-valgrind help doc install cython
