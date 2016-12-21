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

std::string FormatOutputString(const char * Prefix, const char * Description, bool useTimeStamp);
bool HookCrashReporter(bool logon);
void OutputCrashLogLine(const char* format, ...);

#if PLATFORM == PLATFORM_WIN && (defined(NDEBUG) || defined(_DEBUG))

#define CRASHHANDLER_ENABLED

class CStackWalker : public StackWalker
{
public:
    void OnOutput(LPCSTR szText);
    void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName);
    void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion);
    void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry);
    void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr);
};

void StartCrashHandler();
void OnCrash(bool Terminate);

typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
int __cdecl HandleCrash(PEXCEPTION_POINTERS pExceptPtrs);

#define THREAD_TRY_EXECUTION __try
#define THREAD_HANDLE_CRASH  __except( HandleCrash( GetExceptionInformation() ) ) {}

#define THREAD_TRY_EXECUTION2 __try {
#define THREAD_HANDLE_CRASH2  } __except(HandleCrash(GetExceptionInformation())) {}

#else

// We dont wanna confuse nix ;p
#define THREAD_TRY_EXECUTION
#define THREAD_HANDLE_CRASH

#define THREAD_TRY_EXECUTION2 ;
#define THREAD_HANDLE_CRASH2 ;

#endif
