
ifneq '$(DEBUG)' 'no'
  DEBUG := yes
endif

-include ../Makefile.local
-include ../Makefile.include

CFLAGS += -I./ -I../ -Icu/
CXXFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -ldata -Lcu/ -lcu -lrt -lm -L../ -lboruvka -pthread
CFLAGS_BENCH += $(CFLAGS) -O3 -funroll-loops -funsafe-loop-optimizations -fira-loop-pressure

CHECK_REG=cu/cu-check-regressions
CHECK_TS ?=

#TARGETS = libdata.a test bench-heap test-rand-mt test-nn bench
TARGETS = libdata.a test


BENCH_HEAP = bench-heap-fibo bench-heap-pairheap
OBJS  = vec4
OBJS += vec3
OBJS += vec2
OBJS += vec
OBJS += quat
OBJS += pc3
OBJS += pc
OBJS += poly2
OBJS += mat3
OBJS += mat4
OBJS += gug
OBJS += mesh3
OBJS += nearest
OBJS += fibo
OBJS += pairheap
OBJS += dij
OBJS += chull3
OBJS += tasks
OBJS += task-pool
OBJS += vptree
OBJS += nn
OBJS += sort
OBJS += vptree-hamming
OBJS += htable
OBJS += hfunc
OBJS += segmarr
OBJS += bucketheap
OBJS += rbtree
OBJS += splaytree
OBJS += rbtree_int
OBJS += multimap
OBJS += fifo
OBJS += lifo
OBJS += splaytree_int
OBJS += scc
OBJS += msg-schema
OBJS += msg-schema-common
OBJS += lp
OBJS += set
OBJS += queue

OBJS_DATA  = data-vec2
OBJS_DATA += data-vec3
OBJS_DATA += data-quat
OBJS_DATA += data-vec4
OBJS_DATA += data-mat3
OBJS_DATA += data-mat4
OBJS_DATA += data-bunny

ifeq '$(USE_OPENCL)' 'yes'
  CFLAGS  += $(OPENCL_CFLAGS)
  LDFLAGS += $(OPENCL_LDFLAGS)
endif

ifeq '$(USE_HDF5)' 'yes'
ifeq '$(USE_GSL)' 'yes'
  OBJS += thdf5
endif
  CFLAGS  += $(HDF5_CFLAGS)
  LDFLAGS += $(HDF5_LDFLAGS)
endif

ifeq '$(USE_GSL)' 'yes'
  CFLAGS  += $(GSL_CFLAGS)
  LDFLAGS += $(GSL_LDFLAGS)
endif

ifeq '$(USE_CPLEX)' 'yes'
  CFLAGS  += $(CPLEX_CFLAGS)
  LDFLAGS += $(CPLEX_LDFLAGS)
endif
ifeq '$(USE_GUROBI)' 'yes'
  CFLAGS  += $(GUROBI_CFLAGS)
  LDFLAGS += $(GUROBI_LDFLAGS)
endif
ifeq '$(USE_LPSOLVE)' 'yes'
  CFLAGS  += $(LPSOLVE_CFLAGS)
  LDFLAGS += $(LPSOLVE_LDFLAGS)
endif

BENCH_OBJS =

OBJS := $(foreach obj,$(OBJS),$(obj).o)
OBJS_DATA := $(foreach obj,$(OBJS_DATA),$(obj).o)


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

msg-schema-gen: msg-schema-gen.c msg-schema-common.o
	$(CC) $(CFLAGS) -o $@ $^ -L.. -lboruvka -lm
msg-schema-regen: msg-schema-regen.c msg-schema-common.o
	$(CC) $(CFLAGS) -o $@ $^ -L.. -lboruvka -lm
msg-schema-cmp: msg-schema-cmp.c msg-schema-common.o
	$(CC) $(CFLAGS) -o $@ $^ -L.. -lboruvka -lm
msg-schema-print: msg-schema-print.c msg-schema-common.o
	$(CC) $(CFLAGS) -o $@ $^ -L.. -lboruvka -lm

msg-schema.gen.h: msg-schema.sch ../bin/bor-msg-schema.py
	../bin/bor-msg-schema.py --h <$< >$@
msg-schema.gen.c: msg-schema.sch ../bin/bor-msg-schema.py
	../bin/bor-msg-schema.py --c <$< >$@
msg-schema.o: msg-schema.c msg-schema.h msg-schema.gen.h msg-schema.gen.c
	$(CC) $(CFLAGS) -c -o $@ $<
msg-schema-common.o: msg-schema-common.c msg-schema-common.h msg-schema.gen.h msg-schema.gen.c
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

check: all
	./test $(CHECK_TS); \
        T=$$?; \
        $(PYTHON2) $(CHECK_REG) reg; \
        exit $$(($$T + $$?))

check-ci: all
	./test $(CHECK_TS); \
        T=$$?; \
        $(PYTHON2) $(CHECK_REG) --no-progress reg; \
        exit $$(($$T + $$?))

check-msg-schema: msg-schema-gen msg-schema-print
	@echo "Checking msg-schema..."
	@(i=0; while [ $$i != 100 ]; do \
        ./msg-schema-gen tmp.test-msg >tmp.test-msg.txt; \
        ./msg-schema-print tmp.test-msg >tmp.test-msg.2.txt; \
        if ! diff tmp.test-msg.txt tmp.test-msg.2.txt >/dev/null; then \
            echo "FAIL. See tmp.test-msg* files."; \
            exit -1; \
        fi; \
        rm tmp.test-msg tmp.test-msg.txt tmp.test-msg.2.txt; \
        i=$$(($$i + 1)); \
    done;)
	@echo "OK"

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
	rm -f reg/tmp.*
	rm -f reg/TS*.rand-*
	rm -f $(BENCH_HEAP)
	rm -f msg-schema-{gen,regen,print,cmp}

.PHONY: all clean check check-valgrind cu bench-heap
