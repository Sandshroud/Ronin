
#pragma once

#define LARGERRORMESSAGE_ERROR 1
#define LARGERRORMESSAGE_WARNING 2

#if PLATFORM == PLATFORM_WIN

#define TRED FOREGROUND_RED | FOREGROUND_INTENSITY
#define TGREEN FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TYELLOW FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY
#define TNORMAL FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE
#define TWHITE TNORMAL | FOREGROUND_INTENSITY
#define TBLUE FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY
#define TPURPLE FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY

#else

#define TRED 1
#define TGREEN 2
#define TYELLOW 3
#define TNORMAL 4
#define TWHITE 5
#define TBLUE 6
#define TPURPLE 7

#endif

class SERVER_DECL consoleLog : public Singleton<consoleLog>
{
public:
    void Init(int log_Level);
    void SetLoggingLevel(int loglevel) { m_logLevel = loglevel; };
    void SetCLoggingLevel(int cloglevel) { m_clogLevel = cloglevel; };
    void SetAllLoggingLevel(int tloglevel) { m_logLevel = m_clogLevel = tloglevel; };
    int GetLogLevel() { return m_logLevel; }
    int GetCLogLevel() { return m_clogLevel; }

private:
    void PrintTime();
    virtual time_t GetTime();
    void SetColor(int color);

    void AcquireLock() { logLock.Acquire(); };
    void ReleaseLock() { logLock.Release(); };

public: // String outputs
    void outString( const char * str, ... );
    void outError( const char * err, ... );
    void outDetail( const char * str, ... );
    void outDebug( const char * str, ... );
    void outDebugInLine( const char * str, ... );
    void outColor(int color, const char * str, ...);

public: // Console outputs
    void Line();
    void Notice(const char * source, const char * format, ...);
    void Info(const char * source, const char * format, ...);
    void Error(const char * source, const char * format, ...);
    void Warning(const char * source, const char * format, ...);
    void Success(const char * source, const char * format, ...);
    void Debug(const char * source, const char * format, ...);
    void CNotice(int color, const char * source, const char * message);

    void LargeErrorMessage(int color, ...);

private:
#if PLATFORM == PLATFORM_WIN
    HANDLE stdout_handle;
#endif

    Mutex logLock;
    int m_logLevel, m_clogLevel;
};

#define sLog consoleLog::getSingleton()

#define OUT_ERROR sLog.outError
#define OUT_DEBUG sLog.outDebug
#define OUT_DETAIL sLog.outDetail
