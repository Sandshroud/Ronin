/*
 * Sandshroud Project Ronin
 * Copyright (C) 2015-2017 Sandshroud <https://github.com/Sandshroud>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

RONIN_INLINE void EvokeCrashHandler() { uint i = 3, t = 0; for(uint b = 0; b < 6; b++) t = b/--i; }

#if PLATFORM == PLATFORM_WIN && defined(_DEBUG) && defined(CRASHHANDLER_ENABLED)
#define WPAssert2( assertion ) { if( !(assertion) ) { fprintf( stderr, "\n%s:%i ASSERTION FAILED:\n  %s\n", __FILE__, __LINE__, #assertion ); EvokeCrashHandler(); } }
#endif

#define WPAssert( assertion ) { if( !(assertion) ) { fprintf( stderr, "\n%s:%i ASSERTION FAILED:\n  %s\n", __FILE__, __LINE__, #assertion ); assert(assertion); } }
#define WPError( assertion, errmsg ) if( ! (assertion) ) { fprintf( stderr, "%s:%i ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg ); assert( false ); }
#define WPWarning( assertion, errmsg ) if( ! (assertion) ) { fprintf( stdout, "%s:%i WARNING:\n  %s\n", __FILE__, __LINE__, (char *)errmsg ); }

// This should always halt everything.  If you ever find yourself wanting to remove the assert( false ), switch to WPWarning or WPError
#define WPFatal( assertion, errmsg ) if( ! (assertion) ) { fprintf( stderr, "%s:%i FATAL ERROR:\n  %s\n", __FILE__, __LINE__, (char *)errmsg ); assert( #assertion &&0 ); EvokeCrashHandler(); }

// An assert isn't necessarily fatal, although if compiled with asserts enabled it will be.
#if PLATFORM == PLATFORM_WIN && defined(_DEBUG) && defined(CRASHHANDLER_ENABLED)
#define ASSERT WPAssert2
#else
#define ASSERT WPAssert
#endif