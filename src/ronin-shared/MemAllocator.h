/***
 * Demonstrike Core
 */

#define USE_COMPILER_ALLOCATION
#ifndef USE_COMPILER_ALLOCATION

#include <iostream>

void* operator new(size_t sz);
void* operator new[](size_t sz);
void operator delete(void* ptr) throw();
void operator delete[](void* ptr) throw();
void* operator new(size_t sz, const std::nothrow_t&) throw();
void* operator new[](size_t sz, const std::nothrow_t&) throw();
void operator delete(void* ptr, const std::nothrow_t&) throw();
void operator delete[](void* ptr, const std::nothrow_t&) throw();

#endif

//#define NEW_ALLOCATION_TRACKING 1
#ifdef NEW_ALLOCATION_TRACKING
void record_alloc(void *data, const char *file, size_t line);
void unrecord_alloc(void *data);

void* operator new(size_t size);
void operator delete(void *ptr);

extern const char* __file__;
extern size_t __line__;
#define new (__file__=__FILE__,__line__=__LINE__) && 0 ? NULL : new
#endif
