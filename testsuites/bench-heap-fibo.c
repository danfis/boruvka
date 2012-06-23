#include <boruvka/fibo.h>

#define HHEAP fer_fibo_t
#define HNODE fer_fibo_node_t
#define HFUNC(name) ferFibo ## name

#include "bench-heap.c"
