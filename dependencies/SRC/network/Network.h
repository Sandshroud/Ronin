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

#pragma once

// Include our shared dependency defines
#include "../SharedDependencyDefines.h"
// Include our threading capabilities
#include "../Threading/Threading.h"

/* windows sucks dick! */
#if PLATFORM == PLATFORM_WIN
#define USE_IOCP
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#else
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#endif

/* Define these on non-windows systems */
#if PLATFORM != PLATFORM_WIN
#define ioctlsocket ioctl
#define closesocket close
#define TCP_NODELAY 0x6
#define SD_BOTH SHUT_RDWR
#define SOCKET int
#define INVALID_SOCKET -1
#endif

#include "BaseSocket.h"
#include "BaseBuffer.h"
#include "StraightBuffer.h"
#include "CircularBuffer.h"
#include "SocketEngine.h"
#include "TcpSocket.h"

#ifdef USE_POLL
#include <sys/poll.h>
#define NETLIB_POLL
#include "SocketEngine_poll.h"
#endif

#ifdef USE_EPOLL
#include <sys/epoll.h>
#define NETLIB_EPOLL
#include "SocketEngine_epoll.h"
#endif

#ifdef USE_IOCP
#define NETLIB_IOCP
#include "SocketEngine_iocp.h"
#endif

#ifdef USE_SELECT
#define NETLIB_SELECT
#include "SocketEngine_Select.h"
#endif

#ifdef USE_KQUEUE
#include <sys/event.h>
#define NETLIB_KQUEUE
#include "SocketEngine_kqueue.h"
#endif

#include "ListenSocket.h"
