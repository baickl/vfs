#include "pool.h"
#include <stdio.h>

typedef void (*pool_cleanup_t)(void *arg);  

struct pheap  
{  
    void *block;  
    int size, used;  
};

struct pfree  
{  
    pool_cleanup_t f;  
    void *arg;  
    struct pheap *heap;  
    struct pfree *next;  
};  

pool_t _pool_new();
struct pheap *_pool_heap(pool_t p, int size);
struct pfree *_pool_free(pool_t p, pool_cleanup_t f, void *arg);
void _pool_heap_free(void *arg);
void _pool_cleanup_append(pool_t p, struct pfree *pf);


pool_t _pool_new_heap(int size)  
{  
    pool_t p;  
    p = _pool_new();  
    p->heap = _pool_heap(p,size);  
    return p;  
}

int pool_size(pool_t p) 
{
	if(p == NULL)return 0;
	return p->size;
}

void *pool_malloc(pool_t p, int size)  
{  
    void *block;  
  
    if(p == NULL)  
    {  
        return NULL;
    }  
  
    if(p->heap == NULL || size > (p->heap->size / 2))  
    {  
        while((block = malloc(size)) == NULL) sleep(1);  
        p->size += size;  
        _pool_cleanup_append(p, _pool_free(p, _pool_heap_free, block));  
        return block;  
    }  
  
    if(size >= 4)  
        while(p->heap->used&7) p->heap->used++;  
  
    if(size > (p->heap->size - p->heap->used))  
        p->heap = _pool_heap(p, p->heap->size);  
  
    block = (char *)p->heap->block + p->heap->used;  
    p->heap->used += size; 
    return block;  
}  

void pool_free(pool_t p)  
{  
    struct pfree *cur, *stub;  
  
    if(p == NULL)return;  
  
    cur = p->cleanup;  
    while(cur != NULL){  
        (*cur->f)(cur->arg);   
		stub = cur->next;   
		free(cur);
		cur = stub;  
    }  
    free(p);
}



static pool_t _pool_new()
{  
    pool_t p;  
    while((p = malloc(sizeof(_pool))) == NULL) sleep(1);  
    p->cleanup = NULL;  
    p->heap = NULL;  
    p->size = 0;  
    return p;  
}

static struct pheap *_pool_heap(pool_t p, int size)  
{  
    struct pheap *ret;  
    struct pfree *clean;  
  
    while((ret = _pool__malloc(sizeof(struct pheap))) == NULL) sleep(1);  
    while((ret->block = _pool__malloc(size)) == NULL) sleep(1);  
    ret->size = size;  
    p->size += size;  
    ret->used = 0;  
  
    clean = _pool_free(p, _pool_heap_free, (void *)ret);  
    clean->heap = ret; /* for future use in finding used mem for pstrdup */  
    _pool_cleanup_append(p, clean);  
  
    return ret;  
}

static struct pfree *_pool_free(pool_t p, pool_cleanup_t f, void *arg)  
{  
    struct pfree *ret;  
    while((ret = malloc(sizeof(struct pfree))) == NULL) sleep(1);  
    ret->f = f;  
    ret->arg = arg;  
    ret->next = NULL;  
  
    return ret;  
}

static void _pool_heap_free(void *arg)
{  
    struct pheap *h = (struct pheap *)arg;  
    free(h->block);  
    free(h);  
}

static void _pool_cleanup_append(pool_t p, struct pfree *pf)
{  
    struct pfree *cur;  
  
    if(p->cleanup == NULL)  
    {  
        p->cleanup = pf;  
        p->cleanup_tail = pf;  
        return;  
    }  
  
    cur = p->cleanup_tail;   
    cur->next = pf;  
    p->cleanup_tail = pf;  
}