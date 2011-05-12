
ifneq '$(DEBUG)' 'no'
  DEBUG := yes
endif

-include ../Makefile.include

CFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -Lcu/ -lcu -lrt -lm -L../ -lfermat
CFLAGS_BENCH += $(CFLAGS) -O3 -funroll-loops -funsafe-loop-optimizations -fira-loop-pressure

CHECK_REG=cu/check-regressions
CHECK_TS ?=

TARGETS = test bench-heap test-rand-mt test-nn
ifeq '$(USE_OPENCL)' 'yes'
  TARGETS += test-surf-match
endif

BENCH_HEAP = bench-heap-fibo bench-heap-pairheap
OBJS = vec4.o vec3.o vec2.o vec.o quat.o pc3.o \
       mat3.o mat4.o cubes3.o cubes2.o nncells.o mesh3.o nearest.o \
       fibo.o pairheap.o dij.o tasks.o \
       data-vec2.o data-vec3.o data-quat.o data-vec4.o \
       data-mat3.o data-mat4.o
BENCH_OBJS =


all: $(TARGETS)

test: cu $(OBJS) main.c
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LDFLAGS)
bench: cu bench.c data-vec3.o data-quat.o
	$(CC) $(CFLAGS_BENCH) -o $@ bench.c data.o $(LDFLAGS)
	$(CC) $(CFLAGS_BENCH) -S -c -o $@.s bench.c

test-rand-mt: test-rand-mt.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-nn: test-nn.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

test-surf-match: test-surf-match.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS) $(OPENCL_LDFLAGS)

bench-heap: $(BENCH_HEAP)
bench-heap-fibo: bench-heap-fibo.c bench-heap.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
bench-heap-pairheap: bench-heap-pairheap.c bench-heap.c
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
	$(PYTHON) $(CHECK_REG) regressions
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
	rm -f test bench bench2 test-nn test-rand-mt
	rm -f tmp.*
	rm -f regressions/tmp.*
	rm -f $(BENCH_HEAP)

.PHONY: all clean check check-valgrind cu bench-heap
	
