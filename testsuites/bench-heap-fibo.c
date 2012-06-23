#include <boruvka/fibo.h>

#define HHEAP bor_fibo_t
#define HNODE bor_fibo_node_t
#define HFUNC(name) borFibo ## name

#include "bench-heap.c"
