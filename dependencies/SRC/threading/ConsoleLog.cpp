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

#include "Threading.h"

createFileSingleton(consoleLog);

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

void consoleLog::Init(int log_Level)
{
    m_logLevel = log_Level;
    m_clogLevel = log_Level;
    m_delayPrint = false;

    // get error handle
#if PLATFORM == PLATFORM_WIN
    stdout_handle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

unsigned int consoleLog::Update(int targetTime)
{
    if(m_delayPrint == false)
    {
        Sleep(targetTime);
        return targetTime;
    }

    unsigned int counter = 0, now, start = getMSTime();
    while(counter < targetTime)
    {
        AcquireLock();
        while(!m_delayedMessages.empty())
        {
            int color = m_delayedMessages.begin()->first.first;
            time_t printTimer = m_delayedMessages.begin()->first.second;
            std::string header = m_delayedMessages.begin()->second.first;
            std::string message = m_delayedMessages.begin()->second.second;
            m_delayedMessages.erase(m_delayedMessages.begin());
            ReleaseLock();

            if(printTimer)
            {
                PrintTime(printTimer);
                std::printf("N ");
            }

            if(!header.empty())
            {
                SetColor(TWHITE);
                std::printf("%s", header.c_str());
                SetColor(TNORMAL);
            }

            if(color) SetColor(color);
            std::printf("%s", message.c_str());
            SetColor(TNORMAL);

            // Update counter inside loop
            if((now = getMSTime()) != start)
            {
                counter += getMSTimeDiff(now, start);
                start = now;
            }
            AcquireLock();
        }
        ReleaseLock();
        Sleep(1);

        // Update counter
        if((now = getMSTime()) != start)
        {
            counter += now-start;
            start = now;
        }
    }
    return counter;
}

void consoleLog::PrintTime(time_t t_override)
{
    time_t now = t_override ? t_override : GetTime();
    tm local = *localtime(&now);
    std::printf("%02u:%02u:%02u ", local.tm_hour, local.tm_min, local.tm_sec);
}

time_t consoleLog::GetTime()
{
    return time(NULL);
}

void consoleLog::SetColor(int color)
{
#if PLATFORM != PLATFORM_WIN
    fputs(colorstrings[color], stdout);
#else
    SetConsoleTextAttribute(stdout_handle, (WORD)color);
#endif
}

void consoleLog::printf( const char *format, ... )
{
    va_list ap;
    char buf[32768];
    va_start(ap, format);
    vsnprintf(buf, 32768, format, ap);
    va_end(ap);

    std::string message = buf;
    if(message.empty())
        return;

    if(message.find("\n") == std::string::npos)
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", message)));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::outString( const char * str, ... )
{
    if(m_logLevel < 0)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    size_t pos = 0;
    std::string message = buf;
    if(message.empty())
        return;

    if((pos = message.rfind("\n")) == std::string::npos || (pos+5 < message.size()))
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", message)));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::outError( const char * err, ... )
{
    if(m_logLevel < 1)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, err);
    vsnprintf(buf, 32768, err, ap);
    va_end(ap);

    size_t pos = 0;
    std::string message = buf;
    if(message.empty())
        return;

    if((pos = message.rfind("\n")) == std::string::npos || (pos+5 < message.size()))
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(TRED, 0), std::make_pair("", message)));
    else
    {
        SetColor(TRED);
        std::printf(message.c_str());
        SetColor(TNORMAL);
    }
    ReleaseLock();
}

void consoleLog::outDetail( const char * str, ... )
{
    if(m_logLevel < 2)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    size_t pos = 0;
    std::string message = buf;
    if(message.empty())
        return;

    if((pos = message.rfind("\n")) == std::string::npos || (pos+5 < message.size()))
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", std::string(buf))));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::outDebug( const char * str, ... )
{
    if(m_logLevel < 3)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    size_t pos = 0;
    std::string message = buf;
    if(message.empty())
        return;

    if((pos = message.rfind("\n")) == std::string::npos || (pos+5 < message.size()))
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", message)));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::outDebugInLine(const char * str, ...)
{
    if(m_logLevel < 3)
        return;

    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    std::string message = buf;
    if(message.empty())
        return;

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", message)));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::outColor(int color, const char * str, ...)
{
    va_list ap;
    char buf[32768];
    va_start(ap, str);
    vsnprintf(buf, 32768, str, ap);
    va_end(ap);

    size_t pos = 0;
    std::string message = buf;
    if(message.empty())
        return;

    if((pos = message.rfind("\n")) == std::string::npos || (pos+5 < message.size()))
        message.append("\n");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(color, 0), std::make_pair("", message)));
    else std::printf(message.c_str());
    ReleaseLock();
}

void consoleLog::Line()
{
    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(0, 0), std::make_pair("", std::string("\n"))));
    else std::printf("\n");
    ReleaseLock();
}

void consoleLog::Notice(const char * source, const char * format, ...)
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

void consoleLog::Info(const char * source, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TPURPLE, source, msg0);
}

void consoleLog::Error(const char * source, const char * format, ...)
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

void consoleLog::Warning(const char * source, const char * format, ...)
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

void consoleLog::Success(const char * source, const char * format, ...)
{
    va_list ap;
    va_start(ap, format);
    char msg0[1024];
    vsnprintf(msg0, 1024, format, ap);
    va_end(ap);
    CNotice(TGREEN, source, msg0);
}

void consoleLog::Debug(const char * source, const char * format, ...)
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

void consoleLog::CNotice(int color, const char * source, const char * message)
{
    size_t pos = 0;
    std::string strHeader, strMessage = message;
    if(strMessage.empty())
        return;

    if((pos = strMessage.rfind("\n")) == std::string::npos || (pos+5 < strMessage.size()))
        strMessage.append("\n");
    strHeader.append(source);
    if(!strHeader.empty())
        strHeader.append(": ");

    AcquireLock();
    if(m_delayPrint)
        m_delayedMessages.push_back(std::make_pair(std::make_pair(color, GetTime()), std::make_pair(strHeader, strMessage)));
    else
    {
        PrintTime();
        std::printf("N ");
        if(!strHeader.empty())
        {
            SetColor(TWHITE);
            std::printf(strHeader.c_str());
            SetColor(TNORMAL);
        }

        SetColor(color);
        std::printf(strMessage.c_str());
        SetColor(TNORMAL);
    }
    ReleaseLock();
}

void consoleLog::LargeErrorMessage(int Colour, ...)
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

    std::printf("*********************************************************************\n");
    std::printf("*                        MAJOR ERROR/WARNING                        *\n");
    std::printf("*                        ===================                        *\n");
    std::printf("*********************************************************************\n");
    std::printf("*                                                                   *\n");

    for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        i = strlen(*itr);
        j = (i<=65) ? 65 - i : 0;

        std::printf("* %s", *itr);
        for( k = 0; k < j; ++k )
        {
            std::printf(" ");
        }

        std::printf(" *\n");
    }

    std::printf("*********************************************************************\n");

#if PLATFORM == PLATFORM_WIN
    std::string str = "MAJOR ERROR/WARNING:\n";
    for(std::vector<char*>::iterator itr = lines.begin(); itr != lines.end(); ++itr)
    {
        str += *itr;
        str += "\n";
    }

    MessageBox(0, str.c_str(), "Error", MB_OK);
#else
    std::printf("Sleeping for 5 seconds.\n");
    usleep(5000*1000);
#endif

    SetColor(TNORMAL);
    ReleaseLock();
}
