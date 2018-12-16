
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
ifeq '$(USE_OPENCL)' 'yes'
  LDFLAGS += $(OPENCL_LDFLAGS)
  CFLAGS  += $(OPENCL_CFLAGS)
endif

ifeq '$(USE_CPLEX)' 'yes'
  LDFLAGS += $(CPLEX_LDFLAGS)
endif
ifeq '$(USE_GUROBI)' 'yes'
  LDFLAGS += $(GUROBI_LDFLAGS)
endif
ifeq '$(USE_LPSOLVE)' 'yes'
  LDFLAGS += $(LPSOLVE_LDFLAGS)
endif


BENCH_HEAP = bench-heap-fibo bench-heap-pairheap
OBJS = vec4.o vec3.o vec2.o vec.o quat.o pc3.o pc.o poly2.o \
       mat3.o mat4.o gug.o mesh3.o nearest.o \
       fibo.o pairheap.o dij.o chull3.o \
       tasks.o task-pool.o vptree.o nn.o sort.o \
       vptree-hamming.o htable.o hfunc.o segmarr.o bucketheap.o \
       rbtree.o splaytree.o rbtree_int.o multimap.o fifo.o \
       lifo.o splaytree_int.o scc.o msg-schema.o msg-schema-common.o \
       lp.o set.o queue.o
OBJS_DATA = data-vec2.o data-vec3.o data-quat.o data-vec4.o \
            data-mat3.o data-mat4.o data-bunny.o
BENCH_OBJS =

ifeq '$(USE_HDF5)' 'yes'
ifeq '$(USE_GSL)' 'yes'
  OBJS += thdf5.o
  CFLAGS += $(HDF5_CFLAGS) $(GSL_CFLAGS)
  LDFLAGS += $(HDF5_LDFLAGS) $(GSL_LDFLAGS)
endif
endif


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
	@echo ""
	@echo "----------------------------------------";
	./test $(CHECK_TS)
	@echo "----------------------------------------";
	@echo "Checking regressions:";
	$(PYTHON2) $(CHECK_REG) regressions
	@echo ""

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
	rm -f regressions/tmp.*
	rm -f $(BENCH_HEAP)
	rm -f msg-schema-{gen,regen,print,cmp}

.PHONY: all clean check check-valgrind cu bench-heap
