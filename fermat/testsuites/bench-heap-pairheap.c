#include <fermat/pairheap.h>

#define HHEAP fer_pairheap_t
#define HNODE fer_pairheap_node_t
#define HFUNC(name) ferPairHeap ## name

#include "bench-heap.c"
