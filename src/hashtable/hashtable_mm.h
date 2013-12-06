#ifndef _HASHTABLE_MM_H_
#define _HASHTABLE_MM_H_

struct hashtable_mm
{
    void* (*malloc)(size_t);
    void* (*realloc)(void*,size_t);
    void  (*free)(void*);
};

#endif
