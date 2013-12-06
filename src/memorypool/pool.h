#ifndef _POOL_H_
#define _POOL_H_

struct pheap;
struct pfree;
typedef struct pool_struct  
{  
    int size;  
    struct pfree *cleanup;  
    struct pfree *cleanup_tail;  
    struct pheap *heap;  
} _pool,*pool_t;

pool_t 	_pool_new_heap(int size);
int 	pool_size(pool_t p);
void *	pool_malloc(pool_t, int size);
void 	pool_free(pool_t p);

#endif/* _POOL_H_ */