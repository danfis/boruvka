#DYNAMIC = yes
-include ../Makefile.include

CFLAGS += -fno-strict-aliasing
CFLAGS += -I..
CFLAGS += $(PYTHON_CFLAGS)
LDFLAGS += $(PYTHON_LDFLAGS)
LDFLAGS += -L.. -lmg

SO_CFLAGS = -shared -Wl,-soname,mg.so

TARGETS = mg.so
OBJS = main.o vec3.o

all: $(TARGETS)

mg.so: $(OBJS)
	$(CC) $(CFLAGS) $(SO_CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o
	rm -f $(TARGETS)

.PHONY: clean
.DEFAULT_GOAL := all
