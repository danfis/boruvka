# force some options
DEBUG = yes

-include ../Makefile.include
CFLAGS += -I./ -I../ -Icu/
LDFLAGS += -L./ -Lcu/ -lcu -lrt -lm -L../ -lmg

CHECK_REG=cu/check-regressions
CHECK_TS ?=

OBJS = vec3.o vec2.o tr2.o quat.o
BENCH_OBJS =


all: test

test: cu $(OBJS) main.c
	$(CC) $(CFLAGS) -o $@ main.c $(OBJS) $(LDFLAGS)

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
	
