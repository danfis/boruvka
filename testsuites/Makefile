
ifneq '$(DEBUG)' 'no'
  DEBUG := yes
endif

-include ../Makefile.include
-include ../Makefile.local

CFLAGS += -I./ -I../ -Icu/
CXXFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -ldata -Lcu/ -lcu -lrt -lm -L../ -lboruvka -pthread
CFLAGS_BENCH += $(CFLAGS) -O3 -funroll-loops -funsafe-loop-optimizations -fira-loop-pressure

CHECK_REG=cu/check-regressions
CHECK_TS ?=

#TARGETS = libdata.a test bench-heap test-rand-mt test-nn bench
TARGETS = libdata.a test
ifeq '$(USE_OPENCL)' 'yes'
  LDFLAGS += $(OPENCL_LDFLAGS)
  CFLAGS  += $(OPENCL_CFLAGS)
endif


BENCH_HEAP = bench-heap-fibo bench-heap-pairheap
OBJS = vec4.o vec3.o vec2.o vec.o quat.o pc3.o pc.o \
       mat3.o mat4.o gug.o mesh3.o nearest.o \
       fibo.o pairheap.o dij.o chull3.o \
       tasks.o task-pool.o vptree.o nn.o cfg.o opts.o sort.o \
	   vptree-hamming.o hmap.o htable.o
OBJS_DATA = data-vec2.o data-vec3.o data-quat.o data-vec4.o \
            data-mat3.o data-mat4.o data-bunny.o
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
	
