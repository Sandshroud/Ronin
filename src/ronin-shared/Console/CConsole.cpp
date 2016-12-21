/*
 * Sandshroud Project Ronin
 * Copyright (C) 2005-2008 Ascent Team <http://www.ascentemu.com/>
 * Copyright (C) 2008-2009 AspireDev <http://www.aspiredev.org/>
 * Copyright (C) 2009-2017 Sandshroud <https://github.com/Sandshroud>
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

#include "CConsole.h"
#include "BaseConsole.h"

LocalConsole g_localConsole;

#if PLATFORM != PLATFORM_WIN
#include <termios.h>
#endif

ConsoleThread::ConsoleThread() : ThreadContext()
{

}

void ConsoleThread::terminate()
{
    OnShutdown();

#if PLATFORM == PLATFORM_WIN
    /* write the return keydown/keyup event */
    DWORD numb;
    INPUT_RECORD b[5];
    HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    b[0].EventType = KEY_EVENT;
    b[0].Event.KeyEvent.bKeyDown = TRUE;
    b[0].Event.KeyEvent.uChar.AsciiChar = 'X';
    b[0].Event.KeyEvent.wVirtualKeyCode = 'X';
    b[0].Event.KeyEvent.wRepeatCount = 1;

    b[1].EventType = KEY_EVENT;
    b[1].Event.KeyEvent.bKeyDown = FALSE;
    b[1].Event.KeyEvent.uChar.AsciiChar = 'X';
    b[1].Event.KeyEvent.wVirtualKeyCode = 'X';
    b[1].Event.KeyEvent.wRepeatCount = 1;

    b[2].EventType = KEY_EVENT;
    b[2].Event.KeyEvent.bKeyDown = TRUE;
    b[2].Event.KeyEvent.dwControlKeyState = 0;
    b[2].Event.KeyEvent.uChar.AsciiChar = '\r';
    b[2].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
    b[2].Event.KeyEvent.wRepeatCount = 1;
    b[2].Event.KeyEvent.wVirtualScanCode = 0x1c;

    b[3].EventType = KEY_EVENT;
    b[3].Event.KeyEvent.bKeyDown = FALSE;
    b[3].Event.KeyEvent.dwControlKeyState = 0;
    b[3].Event.KeyEvent.uChar.AsciiChar = '\r';
    b[3].Event.KeyEvent.wVirtualKeyCode = VK_RETURN;
    b[3].Event.KeyEvent.wVirtualScanCode = 0x1c;
    b[3].Event.KeyEvent.wRepeatCount = 1;
    BOOL ret = WriteConsoleInput(hStdIn, b, 4, &numb);
#endif

    printf( "Waiting for console thread to terminate....\n" );
    while( m_isRunning )
        Sleep( 100 );
    printf( "Console shut down.\n" );
}

bool ConsoleThread::run()
{
    size_t i = 0;
    size_t len;
    char cmd[300];
#if PLATFORM != PLATFORM_WIN
    fd_set fds;
    struct timeval tv;
#endif

    m_isRunning = true;
    while( GetThreadState() != THREADSTATE_TERMINATE )
    {
#if PLATFORM != PLATFORM_WIN
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO( &fds );
        FD_SET( STDIN_FILENO, &fds );
        if( select( 1, &fds, NULL, NULL, &tv ) <= 0 )
        {
            if(m_threadRunning) // timeout
                continue;
            else
                break;
        }
#endif
        // Read in single line from "stdin"
        memset( cmd, 0, sizeof( cmd ) ); 
        if( fgets( cmd, 300, stdin ) == NULL )
            continue;

        if( GetThreadState() == THREADSTATE_TERMINATE )
            break;

        len = strlen(cmd);
        for( i = 0; i < len; ++i )
        {
            if(cmd[i] == '\n' || cmd[i] == '\r')
                cmd[i] = '\0';
        }

        if( GetThreadState() == THREADSTATE_TERMINATE )
            break;

        HandleConsoleInput(&g_localConsole, cmd);
    }
    m_isRunning = false;
    return false;
}
