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
