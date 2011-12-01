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

-include Makefile.local
-include Makefile.include

CFLAGS += -I.
CXXFLAGS += -I.
LDFLAGS += -L. -lfermat -lm -lrt

ifeq '$(USE_OPENCL)' 'yes'
  CFLAGS += $(OPENCL_CFLAGS)
  LDFLAGS += $(OPENCL_LDFLAGS)
endif

#BIN_TARGETS  = fer-gsrm fer-qdelaunay
#BIN_TARGETS += fer-gng-eu fer-plan-2d
#BIN_TARGETS += fer-gng-t
#BIN_TARGETS += fer-gngp2 fer-gngp3 fer-prm-2d fer-prm-6d
#BIN_TARGETS += fer-rrt-2d fer-rrt-6d
#BIN_TARGETS += fer-gngp fer-gngp-2-3 fer-gngp-6d
BIN_TARGETS += fer-gnnp
BIN_TARGETS += fer-plan
BIN_TARGETS += fer-cfg-scale

TARGETS = libfermat.a
OBJS  = alloc.o
OBJS += cfg.o cfg-lexer.o
OBJS += opts.o
OBJS += varr.o
OBJS += sort.o

OBJS += vec4.o vec3.o vec2.o vec.o
OBJS += mat4.o mat3.o
OBJS += predicates.o

OBJS += pc.o pc-internal.o

OBJS += gug.o nearest-linear.o
OBJS += cubes2.o cubes3.o
OBJS += vptree.o
OBJS += nn-linear.o

OBJS += mesh3.o net.o qhull.o chull3.o

OBJS += fibo.o pairheap.o dij.o

OBJS += gng.o gng-eu.o gsrm.o
OBJS += gng-t.o
OBJS += gng-plan.o gng-plan2.o prm.o rrt.o
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

FERMAT_CD_H  = config.h.m4 core.h
FERMAT_CD_H += vec3.h mat3.h quat.h chull3.h
FERMAT_CD_H += list.h hmap.h tasks.h barrier.h alloc.h
FERMAT_CD_H += cd-const.h cd-shape.h
FERMAT_CD_H += cd-sphere.h cd-box.h cd-cyl.h cd-cap.h cd-plane.h cd-trimesh.h
FERMAT_CD_H += cd-obb.h cd-collide.h cd-separate.h cd-ccd.h
FERMAT_CD_H += cd-cd.h cd-geom.h cd-sap.h cd-cp.h
FERMAT_CD_C  = vec3.c mat3.c chull3.c
FERMAT_CD_C += hmap.c tasks.c barrier.c alloc.c
FERMAT_CD_C += cd-shape.c cd-sphere.c cd-box.c cd-cyl.c cd-cap.c cd-plane.c cd-trimesh.c
FERMAT_CD_C += cd-obb.c cd-collide.c cd-separate.c cd-ccd.c
FERMAT_CD_C += cd-cd.c cd-geom.c cd-sap.c cd-cp.c

FERMAT_CD_H := $(foreach file,$(FERMAT_CD_H),fermat/$(file))
FERMAT_CD_C := $(foreach file,$(FERMAT_CD_C),src/$(file))


# header files that must be generated
HEADERS = cubes2.h cubes3.h

OBJS 		:= $(foreach obj,$(OBJS),.objs/$(obj))
OBJSPP 		:= $(foreach obj,$(OBJSPP),.objs/$(obj))
HEADERS     := $(foreach h,$(HEADERS),fermat/$(h))
BIN_TARGETS := $(foreach target,$(BIN_TARGETS),bin/$(target))


ifeq '$(EXAMPLES)' 'yes'
  TARGETS += examples/cd-simple-collision
  TARGETS += examples/cd-trimesh-collision
  TARGETS += examples/cd-sep
  ifeq '$(USE_ODE)' 'yes'
    TARGETS += examples/cd-ode
  endif

  TARGETS += examples/kohonen-simple
endif

all: $(TARGETS) $(BIN_TARGETS) $(HEADERS)

libfermat.a: $(OBJS) $(OBJSPP)
	ar cr $@ $(OBJS) $(OBJSPP)
	ranlib $@

fermat/config.h: fermat/config.h.m4
	$(M4) $(CONFIG_FLAGS) $< >$@

bin/fer-%: bin/%-main.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
bin/fer-gnnp: bin/gnnp-main.o bin/cfg-map.o libfermat.a
	$(CC) $(CFLAGS) -o $@ $< bin/cfg-map.o $(LDFLAGS)
bin/fer-plan: bin/plan-main.o bin/cfg-map.o libfermat.a
	$(CC) $(CFLAGS) -o $@ $< bin/cfg-map.o $(LDFLAGS)
bin/%.o: bin/%.c bin/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/%: examples/%.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
examples/cd-ode: examples/cd-ode.c libfermat.a
	$(CC) $(CFLAGS) $(ODE_CFLAGS) -o $@ $< $(LDFLAGS) $(ODE_LDFLAGS) $(ODE_DRAWSTUFF)

src/surf-matching.c: src/surf-matching-cl.c
	touch $@
src/cd-sap-gpu.c: src/cd-sap-gpu-cl.c
	touch $@

src/cfg-lexer.c: src/cfg-lexer.l src/cfg-lexer.h
	$(FLEX) -o $@ $<
.objs/cfg.o: src/cfg.c fermat/cfg.h fermat/config.h src/cfg-lexer.c
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/cd-sap.o: src/cd-sap.c src/cd-sap-1.c src/cd-sap-threads.c src/cd-sap-gpu.c fermat/cd-sap.h fermat/config.h
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
	if [ -d cython ]; then $(MAKE) -C cython clean; fi;
	
check:
	$(MAKE) -C testsuites check
check-valgrind:
	$(MAKE) -C testsuites check-valgrind

doc:
	$(MAKE) -C doc

cython:
	$(MAKE) -C cython

analyze: clean
	$(SCAN_BUILD) $(MAKE)



fermat-cd:
	rm -rf fermat-cd*/
	mkdir -p fermat-cd-$(FERMAT_CD_VER)/{fermat,src}
	@echo "FERMAT_CD_H:" $(FERMAT_CD_H)
	@echo "FERMAT_CD_C:" $(FERMAT_CD_C)
	cp $(FERMAT_CD_H) fermat-cd-$(FERMAT_CD_VER)/fermat/
	cp $(FERMAT_CD_C) fermat-cd-$(FERMAT_CD_VER)/src/

help:
	@echo "Targets:"
	@echo "    all            - Build library"
	@echo "    cython         - Build python bindings using cython"
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
	@echo "    CYTHON     - Path to cython              (=$(CYTHON))"
	@echo "    SCAN_BUILD - Path to scan-build          (=$(SCAN_BUILD))"
	@echo ""
	@echo "    EXAMPLES  'yes'/'no' - Set to 'yes' if examples should be build (=$(EXAMPLES))"
	@echo ""
	@echo "    CC_NOT_GCC 'yes'/'no' - If set to 'yes' no gcc specific options will be used (=$(CC_NOT_GCC))"
	@echo ""
	@echo "    DEBUG      'yes'/'no' - Turn on/off debugging          (=$(DEBUG))"
	@echo "    PROFIL     'yes'/'no' - Compiles profiling info        (=$(PROFIL))"
	@echo "    NOWALL     'yes'/'no' - Turns off -Wall gcc option     (=$(NOWALL))"
	@echo "    NOPEDANTIC 'yes'/'no' - Turns off -pedantic gcc option (=$(NOPEDANTIC))"
	@echo ""
	@echo "    USE_SINGLE   'yes'      - Use single precision (=$(USE_SINGLE))"
	@echo "    USE_DOUBLE   'yes'      - Use double precision (=$(USE_DOUBLE))"
	@echo "    USE_MEMCHECK 'yes'/'no' - Use memory checking during allocation (=$(USE_MEMCHECK))"
	@echo "    USE_SSE      'yes'/'no' - Use SSE instructions (=$(USE_SSE))"
	@echo "    USE_OPENCL   'yes'/'no' - Use OpenCL library   (=$(USE_OPENCL))"
	@echo "                              By default, auto detection is used."
	@echo "                              This option depends on USE_SINGLE set to 'yes'"
	@echo "    USE_RAPID    'yes'/'no' - Use RAPID library    (=$(USE_RAPID))"
	@echo "                              By default, auto detection is used."
	@echo "    USE_ODE      'yes'/'no' - Use ODE library      (=$(USE_ODE))"
	@echo "                              By default, auto detection is used."
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
	@echo "    SYSTEM         = "$(SYSTEM)
	@echo "    CFLAGS         = $(CFLAGS)"
	@echo "    CXXFLAGS       = $(CXXFLAGS)"
	@echo "    LDFLAGS        = $(LDFLAGS)"
	@echo "    CONFIG_FLAGS   = $(CONFIG_FLAGS)"
	@echo "    PYTHON_CFLAGS  = $(PYTHON_CFLAGS)"
	@echo "    PYTHON_LDFLAGS = $(PYTHON_LDFLAGS)"
	@echo "    RAPID_CFLAGS   = $(RAPID_CFLAGS)"
	@echo "    RAPID_LDFLAGS  = $(RAPID_LDFLAGS)"
	@echo "    ODE_CFLAGS     = $(ODE_CFLAGS)"
	@echo "    ODE_LDFLAGS    = $(ODE_LDFLAGS)"
	@echo "    OPENCL_CFLAGS  = $(OPENCL_CFLAGS)"
	@echo "    OPENCL_LDFLAGS = $(OPENCL_LDFLAGS)"

.PHONY: all clean check check-valgrind help doc install cython analyze examples
