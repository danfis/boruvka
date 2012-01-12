###
# fermat
# -------
# Copyright (c)2010-2012 Daniel Fiser <danfis@danfis.cz>
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

-include Makefile.local
-include Makefile.include

CFLAGS += -I.
CXXFLAGS += -I.
LDFLAGS += -L. -lfermat -lm -lrt

TARGETS = libfermat.a
OBJS  = alloc.o
OBJS += cfg.o cfg-lexer.o
OBJS += opts.o
OBJS += varr.o
OBJS += sort.o

OBJS += vec4.o vec3.o vec2.o vec.o
OBJS += mat4.o mat3.o
OBJS += predicates.o
OBJS += sort.o

OBJS += pc.o pc-internal.o

OBJS += gug.o nearest-linear.o
OBJS += vptree.o
OBJS += nn-linear.o

OBJS += mesh3.o net.o qhull.o chull3.o

OBJS += fibo.o pairheap.o dij.o

OBJS += gng.o gng-eu.o gsrm.o
OBJS += gng-t.o
OBJS += prm.o rrt.o
OBJS += nnbp.o
OBJS += kohonen.o
OBJS += gnnp.o

OBJS += tasks.o task-pool.o hmap.o hfunc.o barrier.o

OBJS += rand-mt.o timer.o parse.o

OBJS += image.o

OBJS += cd-box.o cd-sphere.o cd-cyl.o cd-trimesh.o cd-shape.o cd-cap.o
OBJS += cd-plane.o
OBJS += cd-obb.o cd-geom.o cd-collide.o cd-cd.o cd-parse.o
OBJS += cd-sphere-grid.o cd-sap.o cd-separate.o cd-ccd.o
OBJS += cd-cp.o
OBJS += ccd.o ccd-polytope.o ccd-mpr.o ccd-gjk.o ccd-support.o
OBJS += cd-collide-box-tri.o

ifeq '$(USE_OPENCL)' 'yes'
  OBJS += opencl.o
  OBJS += surf-matching.o
endif

OBJS += ga.o


BIN_TARGETS  = fer-gsrm
BIN_TARGETS += fer-qdelaunay
BIN_TARGETS += fer-plan


EXAMPLE_TARGETS += cd-simple-collision
EXAMPLE_TARGETS += cd-trimesh-collision
EXAMPLE_TARGETS += cd-sep
ifeq '$(USE_ODE)' 'yes'
  EXAMPLE_TARGETS += cd-ode
endif

EXAMPLE_TARGETS += nnbp-simple
ifeq '$(USE_SDL)' 'yes'
  ifeq '$(USE_SDL_IMAGE)' 'yes'
    EXAMPLE_TARGETS += nnbp-img
  endif
endif

EXAMPLE_TARGETS += ga-knapsack
EXAMPLE_TARGETS += ga-knapsack2
EXAMPLE_TARGETS += kohonen-simple
EXAMPLE_TARGETS += gng-eu
EXAMPLE_TARGETS += gng-t
EXAMPLE_TARGETS += prm-6d


OBJS 		    := $(foreach obj,$(OBJS),.objs/$(obj))
BIN_TARGETS     := $(foreach target,$(BIN_TARGETS),bin/$(target))
EXAMPLE_TARGETS := $(foreach target,$(EXAMPLE_TARGETS),examples/$(target))


ifeq '$(EXAMPLES)' 'yes'
  TARGETS += $(EXAMPLE_TARGETS)
endif
ifeq '$(BINS)' 'yes'
  TARGETS += $(BIN_TARGETS)
endif


all: $(TARGETS)

libfermat.a: $(OBJS)
	ar cr $@ $(OBJS)
	ranlib $@

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

bin/fer-%: bin/%-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
bin/fer-plan: bin/plan-main.o bin/cfg-map.o libfermat.a
	$(CC) $(CFLAGS) $(OPENCL_CFLAGS) -o $@ $< bin/cfg-map.o $(LDFLAGS) $(OPENCL_LDFLAGS)
bin/%.o: bin/%.c bin/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/%: examples/%.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
examples/cd-%: examples/cd-%.c libfermat.a
	$(CC) $(CFLAGS) $(OPENCL_CFLAGS) -o $@ $< $(LDFLAGS) $(OPENCL_LDFLAGS)
examples/cd-ode: examples/cd-ode.c libfermat.a
	$(CC) $(CFLAGS) $(ODE_CFLAGS) $(OPENCL_CFLAGS) -o $@ $< $(LDFLAGS) $(ODE_LDFLAGS) $(ODE_DRAWSTUFF) $(OPENCL_LDFLAGS) -lstdc++
examples/nnbp-img: examples/nnbp-img.c libfermat.a
	$(CC) $(CFLAGS) $(SDL_CFLAGS) $(SDL_IMAGE_CFLAGS) -o $@ $< $(LDFLAGS) $(SDL_LDFLAGS) $(SDL_IMAGE_LDFLAGS)

src/surf-matching.c: src/surf-matching-cl.c
	touch $@
src/cd-sap-gpu.c: src/cd-sap-gpu-cl.c
	touch $@

src/cfg-lexer.c: src/cfg-lexer.l src/cfg-lexer.h
	$(FLEX) -f -t $< >$@
.objs/cfg.o: src/cfg.c fermat/cfg.h fermat/config.h src/cfg-lexer.c
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/cd-sap.o: src/cd-sap.c src/cd-sap-1.c src/cd-sap-threads.c src/cd-sap-gpu.c fermat/cd-sap.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<


%.h: fermat/config.h
%.c: fermat/config.h

%-cl.c: %.cl
	$(PYTHON) ./scripts/cl-to-c.py opencl_program <$< >$@
src/cd-sap-gpu-cl.c: src/cd-sap-gpu.cl
	$(PYTHON) ./scripts/cl-to-c.py opencl_program <$< >$@


install:
	mkdir -p $(PREFIX)/$(INCLUDEDIR)/fermat
	mkdir -p $(PREFIX)/$(LIBDIR)
	cp -r fermat/* $(PREFIX)/$(INCLUDEDIR)/fermat/
	cp libfermat.a $(PREFIX)/$(LIBDIR)

clean:
	rm -f $(OBJS)
	rm -f src/cfg-lexer.c src/cfg-lexer-gen.h
	rm -f .objs/*.o
	rm -f $(TARGETS)
	rm -f $(BIN_TARGETS)
	rm -f fermat/config.h
	rm -f fermat/pc{2,3,4}.h
	rm -f src/pc{2,3,4}.c
	rm -f fermat/cubes{2,3,4}.h
	rm -f src/cubes{2,3,4}.c
	rm -f src/*-cl.c
	if [ -d testsuites ]; then $(MAKE) -C testsuites clean; fi;
	if [ -d doc ]; then $(MAKE) -C doc clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

doc:
	$(MAKE) -C doc

analyze: clean
	$(SCAN_BUILD) $(MAKE)

help:
	@echo "Targets:"
	@echo "    all            - Build library"
	@echo "    doc            - Build documentation"
	@echo "    check          - Build & Run automated tests"
	@echo "    check-valgrind - Build & Run automated tests in valgrind(1)"
	@echo "    clean          - Remove all generated files"
	@echo "    install        - Install library into system"
	@echo "    analyze        - Performs static analysis using Clang Static Analyzer"
	@echo ""
	@echo "    fermat-cd      - Separate FermatCD library into tarball. See FERMAT_CD_VER option."
	@echo "Options:"
	@echo "    CC         - Path to C compiler          (=$(CC))"
	@echo "    CXX        - Path to C++ compiler        (=$(CXX))"
	@echo "    M4         - Path to m4 macro processor  (=$(M4))"
	@echo "    SED        - Path to sed(1)              (=$(SED))"
	@echo "    PYTHON     - Path to python interpret    (=$(PYTHON))"
	@echo "    PYTHON2    - Path to python interpret v2 (=$(PYTHON2))"
	@echo "    PYTHON3    - Path to python interpret v3 (=$(PYTHON3))"
	@echo "    SCAN_BUILD - Path to scan-build          (=$(SCAN_BUILD))"
	@echo ""
	@echo "    EXAMPLES  'yes'/'no' - Set to 'yes' if examples should be build (=$(EXAMPLES))"
	@echo "    BINS      'yes'/'no' - Set to 'yes' if binaries should be build (=$(BINS))"
	@echo ""
	@echo "    CC_NOT_GCC 'yes'/'no' - If set to 'yes' no gcc specific options will be used (=$(CC_NOT_GCC))"
	@echo ""
	@echo "    DEBUG      'yes'/'no' - Turn on/off debugging          (=$(DEBUG))"
	@echo "    PROFIL     'yes'/'no' - Compiles profiling info        (=$(PROFIL))"
	@echo "    NOWALL     'yes'/'no' - Turns off -Wall gcc option     (=$(NOWALL))"
	@echo "    NOPEDANTIC 'yes'/'no' - Turns off -pedantic gcc option (=$(NOPEDANTIC))"
	@echo ""
	@echo "    USE_SINGLE   'yes'       - Use single precision  (=$(USE_SINGLE))"
	@echo "    USE_DOUBLE   'yes'       - Use double precision  (=$(USE_DOUBLE))"
	@echo "    USE_MEMCHECK 'yes'/'no'  - Use memory checking during allocation (=$(USE_MEMCHECK))"
	@echo "    USE_SSE      'yes'/'no'  - Use SSE instructions  (=$(USE_SSE))"
	@echo "    USE_OPENCL   'yes'/'no'  - Use OpenCL library    (=$(USE_OPENCL))"
	@echo "                               By default, auto detection is used."
	@echo "                               This option depends on USE_SINGLE set to 'yes'"
	@echo "    USE_RAPID    'yes'/'no'  - Use RAPID library     (=$(USE_RAPID))"
	@echo "                               By default, auto detection is used."
	@echo "    USE_ODE      'yes'/'no'  - Use ODE library       (=$(USE_ODE))"
	@echo "                               By default, auto detection is used."
	@echo "    USE_SDL      'yes'/'no'  - Use SDL library       (=$(USE_SDL))"
	@echo "                               By default, auto detection is used."
	@echo "    USE_SDL_IMAGE 'yes'/'no' - Use SDL_image library (=$(USE_SDL_IMAGE))"
	@echo "                               By default, auto detection is used."
	@echo ""
	@echo "    CD_TIME_MEASURE 'yes'/'no' - Set to 'yes' if time measurement should be used in CD (=$(CD_TIME_MEASURE))"
	@echo ""
	@echo "    PREFIX     - Prefix where library will be installed                             (=$(PREFIX))"
	@echo "    INCLUDEDIR - Directory where header files will be installed (PREFIX/INCLUDEDIR) (=$(INCLUDEDIR))"
	@echo "    LIBDIR     - Directory where library will be installed (PREFIX/LIBDIR)          (=$(LIBDIR))"
	@echo ""
	@echo "    FERMAT_CD_VER - Stores version that should be used when creating separate fermat-cd tarball. (=$(FERMAT_CD_VER))"
	@echo ""
	@echo "Variables:"
	@echo "  Note that most of can be preset or changed by user"
	@echo "    SYSTEM            = $(SYSTEM)"
	@echo "    CFLAGS            = $(CFLAGS)"
	@echo "    CXXFLAGS          = $(CXXFLAGS)"
	@echo "    LDFLAGS           = $(LDFLAGS)"
	@echo "    CONFIG_FLAGS      = $(CONFIG_FLAGS)"
	@echo "    PYTHON_CFLAGS     = $(PYTHON_CFLAGS)"
	@echo "    PYTHON_LDFLAGS    = $(PYTHON_LDFLAGS)"
	@echo "    RAPID_CFLAGS      = $(RAPID_CFLAGS)"
	@echo "    RAPID_LDFLAGS     = $(RAPID_LDFLAGS)"
	@echo "    ODE_CFLAGS        = $(ODE_CFLAGS)"
	@echo "    ODE_LDFLAGS       = $(ODE_LDFLAGS)"
	@echo "    OPENCL_CFLAGS     = $(OPENCL_CFLAGS)"
	@echo "    OPENCL_LDFLAGS    = $(OPENCL_LDFLAGS)"
	@echo "    SDL_CFLAGS        = $(SDL_CFLAGS)"
	@echo "    SDL_LDFLAGS       = $(SDL_LDFLAGS)"
	@echo "    SDL_IMAGE_CFLAGS  = $(SDL_IMAGE_CFLAGS)"
	@echo "    SDL_IMAGE_LDFLAGS = $(SDL_IMAGE_LDFLAGS)"

.PHONY: all clean check check-valgrind help doc install analyze examples
