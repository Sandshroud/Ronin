/***
 * Demonstrike Core
 */

#include "MemoryLeaks.h"

//#include <vld.h>
//#include <vldapi.h>

#if COMPILER == COMPILER_MICROSOFT

createFileSingleton( MemoryManager ) ;

/// Catch memory leaks
MemoryManager::MemoryManager( )
{
//  VLDEnable();
}

#endif

