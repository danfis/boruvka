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

OBJS += quat.o vec4.o vec3.o vec2.o vec.o
OBJS += mat4.o mat3.o
OBJS += predicates.o
OBJS += sort.o

OBJS += pc.o pc-internal.o

OBJS += gug.o nearest-linear.o
OBJS += vptree.o
OBJS += nn-linear.o

OBJS += mesh3.o net.o qhull.o chull3.o

OBJS += fibo.o pairheap.o dij.o

OBJS += tasks.o task-pool.o hmap.o hfunc.o barrier.o

OBJS += rand-mt.o timer.o parse.o

OBJS += image.o

ifeq '$(USE_OPENCL)' 'yes'
  OBJS += opencl.o
endif


BIN_TARGETS = fer-qdelaunay


OBJS 		    := $(foreach obj,$(OBJS),.objs/$(obj))
BIN_TARGETS     := $(foreach target,$(BIN_TARGETS),bin/$(target))


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
bin/%.o: bin/%.c bin/%.h
	$(CC) $(CFLAGS) -c -o $@ $<

examples/%: examples/%.c libfermat.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

src/cfg-lexer.c: src/cfg-lexer.l src/cfg-lexer.h
	$(FLEX) -f -t $< >$@
.objs/cfg.o: src/cfg.c fermat/cfg.h fermat/config.h src/cfg-lexer.c
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/%.h fermat/config.h
	$(CC) $(CFLAGS) -c -o $@ $<
.objs/%.o: src/%.c fermat/config.h
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
	@echo ""
	@echo "    PREFIX     - Prefix where library will be installed                             (=$(PREFIX))"
	@echo "    INCLUDEDIR - Directory where header files will be installed (PREFIX/INCLUDEDIR) (=$(INCLUDEDIR))"
	@echo "    LIBDIR     - Directory where library will be installed (PREFIX/LIBDIR)          (=$(LIBDIR))"
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
	@echo "    OPENCL_CFLAGS     = $(OPENCL_CFLAGS)"
	@echo "    OPENCL_LDFLAGS    = $(OPENCL_LDFLAGS)"

.PHONY: all clean check check-valgrind help doc install analyze examples
