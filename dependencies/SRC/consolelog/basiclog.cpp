
#include "consolelog.h"

basicLog *basicLog::basicLogExistence = NULL;

#if PLATFORM != PLATFORM_WIN
static const char* colorstrings[TPURPLE+1] = {
"",
"\033[22;31m",
"\033[22;32m",
"\033[01;33m",
"\033[0m",
"\033[01;37m",
"\033[22;34m",
"\033[22;33m",
};
#endif

void basicLog::Init(int log_Level)
{
    m_logLevel = log_Level;
    m_clogLevel = log_Level;

    // get error handle
#if PLATFORM == PLATFORM_WIN
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void basicLog::PrintTime()
{
    time_t now = GetTime();
    tm local = *localtime(&now);
    printf("%02u:%02u:%02u ", local.tm_hour, local.tm_min, local.tm_sec);
}

time_t basicLog::GetTime()
{
    return time(NULL);
}

void basicLog::SetColor(int color)
{
#if PLATFORM != PLATFORM_WIN
    fputs(colorstrings[color], stdout);
#else
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
#endif
}

void basicLog::outString( const char * str, ... )
{
    if(m_logLevel < 0)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    AcquireLock();
    printf(buf);
    printf("\n");
    ReleaseLock();
}

void basicLog::outError( const char * err, ... )
{
    if(m_logLevel < 1)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, err);
    vsnprintf(buf, 32768, err, ap);
    va_end(ap);

    AcquireLock();
    SetColor(TRED);
    printf(buf);
    printf("\n");
    SetColor(TNORMAL);
    ReleaseLock();
}

void basicLog::outDetail( const char * str, ... )
{
    if(m_logLevel < 2)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    AcquireLock();
    printf(buf);
    printf("\n");
    ReleaseLock();
}

void basicLog::outDebug( const char * str, ... )
{
    if(m_logLevel < 3)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    AcquireLock();
    printf(buf);
    printf("\n");
    ReleaseLock();
}

void basicLog::outDebugInLine(const char * str, ...)
{
    if(m_logLevel < 3)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    AcquireLock();
    printf(buf);
    ReleaseLock();
}

void basicLog::outColor(int color, const char * str, ...)
{
    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    AcquireLock();
    SetColor(color);
    printf(buf);
    printf("\n");
    SetColor(TNORMAL);
    ReleaseLock();
}

void basicLog::Line()
{
    AcquireLock();
    printf("\n");
    ReleaseLock();
}

void basicLog::Notice(const char * source, const char * format, ...)
{
    if(m_clogLevel < 0)
        return;

    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TNORMAL, source, msg0);
}

void basicLog::Info(const char * source, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TPURPLE, source, msg0);
}

void basicLog::Error(const char * source, const char * format, ...)
{
    if(m_clogLevel < 1)
        return;

    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TRED, source, msg0);
}

void basicLog::Warning(const char * source, const char * format, ...)
{
    if(m_clogLevel < 2)
        return;

    /* warning is old loglevel 2/detail */
    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TYELLOW, source, msg0);
}

void basicLog::Success(const char * source, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TGREEN, source, msg0);
}

void basicLog::Debug(const char * source, const char * format, ...)
{
    if(m_clogLevel != 3 && m_clogLevel != 6)
        return;

    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TBLUE, source, msg0);
}

void basicLog::CNotice(int color, const char * source, const char * message)
{
    AcquireLock();
    PrintTime();
    printf("N ");
    if(source != NULL && *source)
    {
        SetColor(TWHITE);
        printf("%s: ", source);
        SetColor(TNORMAL);
    }

    SetColor(color);
    printf(message);
    printf("\n");
    SetColor(TNORMAL);
    ReleaseLock();
}

void basicLog::LargeErrorMessage(int Colour, ...)
{
    std::vector<char*> lines;
    char * pointer;
    va_list ap;
    va_start(ap, Colour);

    size_t i,j,k;
    pointer = va_arg(ap, char*);
    while( pointer != NULL )
    {
        lines.push_back( pointer );
        pointer = va_arg(ap, char*);
    }

    AcquireLock();

    if( Colour == LARGERRORMESSAGE_ERROR )
        SetColor(TRED);
    else
        SetColor(TYELLOW);

    printf("*********************************************************************\n");
    printf("*                        MAJOR ERROR/WARNING                        *\n");
    printf("*                        ===================                        *\n");
    printf("*********************************************************************\n");
    printf("*                                                                   *\n");

    for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        i = strlen(*itr);
        j = (i<=65) ? 65 - i : 0;

        printf("* %s", *itr);
        for( k = 0; k < j; ++k )
        {
            printf(" ");
        }

        printf(" *\n");
    }

    printf("*********************************************************************\n");

#if PLATFORM == PLATFORM_WIN
    std::string str = "MAJOR ERROR/WARNING:\n";
    for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        str += *itr;
        str += "\n";
    }

    MessageBox(0, str.c_str(), "Error", MB_OK);
#else
    printf("Sleeping for 5 seconds.\n");
    usleep(5000*1000);
#endif

    SetColor(TNORMAL);
    ReleaseLock();
}
