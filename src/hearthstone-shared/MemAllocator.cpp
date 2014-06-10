/***
 * Demonstrike Core
 */

#include "MemAllocator.h"

#define USE_COMPILER_ALLOCATION
#ifndef USE_COMPILER_ALLOCATION

void* operator new(size_t sz)
{
    void *res = std::malloc(sz);

    if (res == NULL)
        throw std::bad_alloc();

    return res;
}

void* operator new[](size_t sz)
{
    void *res = std::malloc(sz);

    if (res == NULL)
        throw std::bad_alloc();

    return res;
}

void operator delete(void* ptr) throw()
{
    std::free(ptr);
}

void operator delete[](void* ptr) throw()
{
    std::free(ptr);
}

void* operator new(size_t sz, const std::nothrow_t&) throw()
{
    return std::malloc(sz);
}

void* operator new[](size_t sz, const std::nothrow_t&) throw()
{
    return std::malloc(sz);
}

void operator delete(void* ptr, const std::nothrow_t&) throw()
{
    std::free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) throw()
{
    std::free(ptr);
}

#endif
