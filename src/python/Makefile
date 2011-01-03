#DYNAMIC = yes
-include ../Makefile.include

CFLAGS += -fno-strict-aliasing
CFLAGS += -I..
CFLAGS += $(PYTHON_CFLAGS)
LDFLAGS += $(PYTHON_LDFLAGS)
LDFLAGS += -L.. -lfermat

SO_CFLAGS = -shared -Wl,-soname,fermat.so

TARGETS = fermat.so
OBJS = main.o vec3.o vec4.o quat.o point_cloud.o

all: $(TARGETS)

fermat.so: $(OBJS)
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
