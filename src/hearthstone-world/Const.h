/***
 * Demonstrike Core
 */

#pragma once

#define NUM_MAPS 900
#define CL_BUILD_SUPPORT 15595

#define CharacterDatabase (*Database_Character)
#define WorldDatabase (*Database_World)
#define LogDatabase (*Database_Log)

template<typename T> T cast(void *val) { return static_cast<T>(val); }
template<typename T> T *castPtr(void *val) { return static_cast<T*>(val); }

SERVER_DECL extern DirectDatabase* Database_Character;
SERVER_DECL extern DirectDatabase* Database_Account;
SERVER_DECL extern DirectDatabase* Database_World;
SERVER_DECL extern DirectDatabase* Database_Log;

SERVER_DECL bool Rand(float chance);
SERVER_DECL bool Rand(uint32 chance);
SERVER_DECL bool Rand(int32 chance);
