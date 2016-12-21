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

#ifndef NETLIB_IOCP

/** Standard ListenSocket
 */

template<class T>
class SERVER_DECL ListenSocket : public BaseSocket
{
public:
    ListenSocket()
    {
        m_fd = socket(AF_INET, SOCK_STREAM, 0);
        u_long arg = 1;
        setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&arg, sizeof(u_long));
        if(m_fd == INVALID_SOCKET)
        {
            printf("WARNING: ListenSocket constructor: could not create socket() %u (%s)\n", errno, strerror(errno));
        }
        m_connected = false;
        m_deleted = false;
    }

    void OnRead(size_t len)
    {
        if(!m_connected)
            return;

#if PLATFORM == PLATFORM_WIN
        int len2 = sizeof(sockaddr_in);
#else
        socklen_t len2 = sizeof(sockaddr_in);
#endif
        sockaddr *peer;
        SOCKET fd = accept(m_fd, (sockaddr*)&peer, &len2);
        if(fd > 0)
        {
            T * s = new T(fd, &peer);
            s->Finalize();
        }
    }

    void OnWrite(size_t len) {}
    void OnError(int err) {}
    void OnAccept(void * pointer) {}
    
    bool Open(const char * hostname, u_short port)
    {
        if(m_fd == INVALID_SOCKET)
        {
            printf("No fd in listensocket\n");
            return false;
        }

        if(!strcmp(hostname, "0.0.0.0"))
            m_address.sin_addr.s_addr = htonl(INADDR_ANY);
        else
        {
            hostent * h = gethostbyname(hostname);
            if(!h)
            {
                printf("Could not resolve listen address\n");
                return false;
            }

            memcpy(&address.sin_addr, h->h_addr_list[0], sizeof(in_addr));
        }

        m_address.sin_family = AF_INET;
        m_address.sin_port = ntohs(port);

        if(::bind(m_fd, (const sockaddr*)&m_address, sizeof(sockaddr_in)) < 0)
        {
            printf("Could not bind\n");
            return false;
        }

        if(listen(m_fd, 5) < 0)
        {
            printf("Could not bind\n");
            return false;
        }

        // add to mgr
        m_connected = true;
        sSocketEngine.AddSocket(this);
        return true;
    }

    bool Writable() { return false; }

    void Delete()
    {
        if(m_deleted) return;
        m_deleted = true;
        
        /* queue deletion */
        sSocketDeleter.Add(this);

        if(m_connected) Disconnect();
    }

    void Disconnect()
    {
        if(!m_connected) return;
        m_connected = false;

        sSocketEngine.RemoveSocket(this);
        closesocket(m_fd);
        if(!m_deleted) Delete();
    }
};

#else

/** IOCP ListenSocket
 */

static int Length = sizeof(sockaddr_in)+16;

template<class T>
class SERVER_DECL ListenSocket : public BaseSocket
{
public:
    ListenSocket()
    {
        m_fd = INVALID_SOCKET;
        m_connected = false;
        m_deleted = false;
    }

    void OnRead(size_t len) { }
    void OnWrite(size_t len) {}
    void OnError(int err) {}

    void OnAccept(void * pointer)
    {
        // Our socket was pushed to front of passback buffer
        SOCKET fd = *(SOCKET*)pointer;
        sockaddr *local, *remote;
        int localOut = 0, remoteOut = 0;
        GetAcceptExSockaddrs(((char*)pointer)+sizeof(SOCKET), 0, Length, Length, &local, &localOut, &remote, &remoteOut);

        sockaddr_in *addr = (sockaddr_in*)remote;
        T * s = new T(fd, addr);
        s->Finalize();
        free(pointer);

        /* post the next accept event */
        PostEvent();
    }

    bool Writable() { return false; }

    void Delete()
    {
        if(m_deleted) return;
        m_deleted = true;

        /* queue deletion */
        sSocketDeleter.Add(this);

        if(m_connected) Disconnect();
    }

    void Disconnect()
    {
        if(!m_connected)
            return;
        m_connected = false;

        sSocketEngine.RemoveSocket(this);
        if(m_fd != INVALID_SOCKET) closesocket(m_fd);
        if(!m_deleted) Delete();
    }

    void PostEvent()
    {
        if(m_fd == INVALID_SOCKET)
            return;

        Overlapped * ov = new Overlapped;
        memset(ov, 0, sizeof(Overlapped));
        ov->m_op = IO_EVENT_ACCEPT;
        ov->m_acceptBuffer = malloc(1024);
        memset(ov->m_acceptBuffer, 0, 1024);

        SOCKET s = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
        *(SOCKET*)&((char*)ov->m_acceptBuffer)[0] = s;
        DWORD bytes;

        if(!AcceptEx(m_fd, s, ((char*)ov->m_acceptBuffer) + sizeof(SOCKET), 0, Length, Length, &bytes, &ov->m_ov))
        {
            if(WSA_IO_PENDING != WSAGetLastError())
                printf("AcceptEx error: %u\n", WSAGetLastError());
        }
    }

    bool Open(const char * hostname, u_short port)
    {
        if(m_fd != INVALID_SOCKET)
            return true; // Socket already open

        std::string portStr;
#if _MSC_VER >= 1700 // VC2012 has int to_string
        portStr.append(std::to_string(port).c_str());
#else // 2010 has longlong but use itoa instead
        char *portchar = new char[10];
        itoa(port, portchar, 10);
        portStr.append(portchar);
        delete [] portchar;
#endif

        addrinfo *result = NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;
        if(getaddrinfo(hostname, portStr.c_str(), &hints, &result))
        {
            printf("Could not attain address info\n");
            return false;
        }
        if((m_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol)) == INVALID_SOCKET)
        {
            printf("Could not initialize socket\n");
            freeaddrinfo(result);
            return false;
        }
        if(::bind(m_fd, result->ai_addr, (int)result->ai_addrlen) < 0)
        {
            printf("Could not bind\n");
            freeaddrinfo(result);
            closesocket(m_fd);
            return false;
        }
        if(listen(m_fd, SOMAXCONN) == SOCKET_ERROR)
        {
            printf("Failed to listen\n");
            closesocket(m_fd);
            return false;
        }
        memcpy(&m_address, &result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);

        m_connected = true;
        sSocketEngine.AddSocket(this);
        PostEvent();
        return true;
    }
};

#endif      // NETLIB_IOCP

/* Common Functions */
template<class T> bool CreateListenSocket(const char * hostname, u_short port)
{
    ListenSocket<T> * s = new ListenSocket<T>();
    if(!s->Open(hostname, port))
    {
        s->Delete();
        return false;
    } else return true;
}
