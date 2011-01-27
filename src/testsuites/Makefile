# force some options
DEBUG = yes

-include ../Makefile.include
CFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -Lcu/ -lcu -lrt -lm -L../ -lfermat
CFLAGS_BENCH += $(CFLAGS) -O3 -funroll-loops -funsafe-loop-optimizations -fira-loop-pressure

CHECK_REG=cu/check-regressions
CHECK_TS ?=

OBJS = vec4.o vec3.o vec2.o quat.o point_cloud.o \
       mat3.o mat4.o cubes3.o \
       data-vec2.o data-vec3.o data-quat.o data-vec4.o \
       data-mat3.o data-mat4.o
BENCH_OBJS =

ifeq '$(USE_GSL)' 'yes'
  OBJS += vec.o mat.o
  CFLAGS += $(GSL_CFLAGS)
  LDFLAGS += $(GSL_LDFLAGS)
endif


all: test

test: cu $(OBJS) main.c
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LDFLAGS)
bench: cu bench.c data-vec3.o data-quat.o
	$(CC) $(CFLAGS_BENCH) -o $@ bench.c data.o $(LDFLAGS)
	$(CC) $(CFLAGS_BENCH) -S -c -o $@.s bench.c

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
	rm -f test bench bench2
	rm -f tmp.*
	rm -f regressions/tmp.*

.PHONY: all clean check check-valgrind cu
	
