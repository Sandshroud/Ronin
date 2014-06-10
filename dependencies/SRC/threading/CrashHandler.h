/***
 * Demonstrike Core
 */

#pragma once

std::string FormatOutputString(const char * Prefix, const char * Description, bool useTimeStamp);
bool HookCrashReporter(bool logon);
void OutputCrashLogLine(const char* format, ...);

#if PLATFORM == PLATFORM_WIN && defined(NDEBUG)

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
