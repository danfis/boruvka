
ifneq '$(DEBUG)' 'no'
  DEBUG := yes
endif

-include ../Makefile.include

CFLAGS += -I./ -I../ -Icu/
CXXFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -ldata -Lcu/ -lcu -lrt -lm -L../ -lfermat -pthread
CFLAGS_BENCH += $(CFLAGS) -O3 -funroll-loops -funsafe-loop-optimizations -fira-loop-pressure

CHECK_REG=cu/check-regressions
CHECK_TS ?=

TARGETS = libdata.a test bench-heap test-rand-mt test-nn
ifeq '$(USE_OPENCL)' 'yes'
  TARGETS += test-surf-match
  LDFLAGS += $(OPENCL_LDFLAGS)
  CFLAGS  += $(OPENCL_CFLAGS)
endif

TARGETS += test-cd-spheres test-cd-trimesh

ifeq '$(USE_ODE)' 'yes'
  TARGETS += test-cd-ode
endif


USE_OZCOLLIDE = no
OZCOLLIDE_CFLAGS = -I/home/danfis/tmp/ozcollide
OZCOLLIDE_LDFLAGS = -L/home/danfis/tmp/ozcollide/ozcollide -lozcollide

ifeq '$(USE_OZCOLLIDE)' 'yes'
  OZCOLLIDE_CFLAGS += -DHAVE_OZCOLLIDE
else
  OZCOLLIDE_CFLAGS  :=
  OZCOLLIDE_LDFLAGS :=
endif




BENCH_HEAP = bench-heap-fibo bench-heap-pairheap
OBJS = vec4.o vec3.o vec2.o vec.o quat.o pc3.o pc.o \
       mat3.o mat4.o gug.o mesh3.o nearest.o \
       fibo.o pairheap.o dij.o cd.o cd-sphere-grid.o chull3.o \
       tasks.o task-pool.o vptree.o nn.o cfg.o opts.o
OBJS_DATA = data-vec2.o data-vec3.o data-quat.o data-vec4.o \
            data-mat3.o data-mat4.o data-bunny.o \
			data-protein-big.o data-protein-small.o
BENCH_OBJS =


all: $(TARGETS)

libdata.a: $(OBJS_DATA)
	ar cr $@ $^
	ranlib $@

test: cu libdata.a $(OBJS) main.c
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LDFLAGS)
bench: cu bench.c libdata.a
	$(CC) $(CFLAGS_BENCH) -o $@ bench.c $(LDFLAGS)
	$(CC) $(CFLAGS_BENCH) -S -c -o $@.s bench.c

test-rand-mt: test-rand-mt.c libdata.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-nn: test-nn.c libdata.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-surf-match: test-surf-match.c libdata.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-cd-spheres: test-cd-spheres.cpp libdata.a
	$(CXX) $(CXXFLAGS) $(OZCOLLIDE_CFLAGS) -o $@ $< $(LDFLAGS) $(OZCOLLIDE_LDFLAGS)
test-cd-trimesh: test-cd-trimesh.cpp libdata.a
	$(CXX) $(CXXFLAGS) $(RAPID_CFLAGS) -o $@ $^ $(LDFLAGS) $(RAPID_LDFLAGS)
test-cd-ode: test-cd-ode.c libdata.a
	$(CC) $(CFLAGS) $(ODE_CFLAGS) -o $@ $< $(LDFLAGS) $(ODE_LDFLAGS) $(ODE_DRAWSTUFF) -lstdc++

bench-heap: $(BENCH_HEAP)
bench-heap-fibo: bench-heap-fibo.c bench-heap.c libdata.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
bench-heap-pairheap: bench-heap-pairheap.c bench-heap.c libdata.a
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

check: all
	@echo ""
	@echo "----------------------------------------";
	./test $(CHECK_TS)
	@echo "----------------------------------------";
	@echo "Checking regressions:";
	$(PYTHON2) $(CHECK_REG) regressions
	@echo ""

check-valgrind: all
	valgrind -q --leak-check=full --show-reachable=yes --trace-children=yes \
             --error-limit=no \
             ./test $(CHECK_TS)

check-valgrind-gen-suppressions: all
	valgrind -q --leak-check=full --show-reachable=yes --trace-children=yes \
             --gen-suppressions=all --log-file=out --error-limit=no \
             ./test $(CHECK_TS)

cu:
	$(MAKE) ENABLE_TIMER=yes -C cu/

clean:
	rm -f *.o
	rm -f objs/*.o
	rm -f $(TARGETS)
	rm -f tmp.*
	rm -f regressions/tmp.*
	rm -f $(BENCH_HEAP)

.PHONY: all clean check check-valgrind cu bench-heap
	
