/***
 * Demonstrike Core
 */

#include "Network.h"

#ifdef NETLIB_IOCP

iocpEngine::iocpEngine(int TLimit)
{
    WSADATA wsadata;
    WSAStartup(MAKEWORD(2,0), &wsadata);
    m_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    ThreadLimit = TLimit;
    new SocketDeleter();
}

iocpEngine::~iocpEngine()
{
    CloseHandle(m_completionPort);
}

void iocpEngine::AddSocket(BaseSocket * s)
{
    /* assign the socket to the completion port */
    CreateIoCompletionPort((HANDLE)s->GetFd(), m_completionPort, (ULONG_PTR)s, 0);

    m_socketLock.Acquire();
    m_sockets.insert(s);
    m_socketLock.Release();

    /* setup the initial read event */
    s->OnRead(0xFFFFFFFF);

    /* and the intial write event */
    if(s->Writable())
        WantWrite(s);
}

void iocpEngine::RemoveSocket(BaseSocket * s)
{
    /* Cancel any pending i/o requests */
    CancelIo((HANDLE)s->GetFd());

    m_socketLock.Acquire();
    m_sockets.erase(s);
    m_socketLock.Release();
}

void iocpEngine::WantWrite(BaseSocket * s)
{
    WSABUF buf;
    buf.len = (ulong)s->GetWriteBuffer()->GetSize();
    buf.buf = (char*)s->GetWriteBuffer()->GetBufferOffset();

    Overlapped * ov = new Overlapped;
    memset(ov, 0, sizeof(Overlapped));
    ov->m_op = IO_EVENT_WRITE;

    DWORD sent;
    if(WSASend(s->GetFd(), &buf, 1, &sent, 0, &ov->m_ov, 0) == SOCKET_ERROR)
    {
        if(WSAGetLastError() != WSA_IO_PENDING)
        {
            s->OnError(WSAGetLastError());
        }
    }
}

void OnAccept(Overlapped * ov, BaseSocket * s, DWORD len)
{
    s->OnAccept(ov->m_acceptBuffer);
}

void OnRead(Overlapped * ov, BaseSocket * s, DWORD len)
{
    s->OnRead(len);
}

void OnWrite(Overlapped * ov, BaseSocket * s, DWORD len)
{
    s->OnWrite(len);
}

typedef void(*IOCPHandler)(Overlapped * ov, BaseSocket * s, DWORD len);
static IOCPHandler Handlers[] = { &OnAccept, &OnRead, &OnWrite, };

void iocpEngine::MessageLoop()
{
    BaseSocket * s;
    LPOVERLAPPED ov;
    Overlapped * myov;
    DWORD len;
    for(;;)
    {
        if(!GetQueuedCompletionStatus(m_completionPort, &len, (PULONG_PTR)&s, &ov, 5000))
            continue;

        myov = CONTAINING_RECORD(ov, Overlapped, m_ov);
        if(myov->m_op == IO_SHUTDOWN)
        {
            delete myov;
            break;
        }

        if(s->IsConnected())
            Handlers[myov->m_op](myov, s, len);

        delete myov;
        myov = NULL;
    }
}

void iocpEngine::SpawnThreads()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    thread_count = si.dwNumberOfProcessors*2;
    if(ThreadLimit > 0 && thread_count > ThreadLimit)
        thread_count = ThreadLimit;

    for(int i = 1; i <= thread_count; i++)
    {
        char ThreadName[45];
        sprintf(ThreadName, "SocketEngineThread|%u", i);
        ThreadPool.ExecuteTask(ThreadName, new SocketEngineThread(this));
    }
}

void iocpEngine::Shutdown()
{
    /* post messages to all threads to shut 'em down */
    for(int i = 1; i <= thread_count; i++)
    {
        Overlapped * ov = new Overlapped();
        memset(ov, 0, sizeof(Overlapped));
        ov->m_op = IO_SHUTDOWN;
        PostQueuedCompletionStatus(m_completionPort, 0, 0, &ov->m_ov);
    }

    /* kill all the sockets */
    m_socketLock.Acquire();
    std::set<BaseSocket*>::iterator itr;
    BaseSocket * s;
    for(itr = m_sockets.begin(); itr != m_sockets.end();)
    {
        s = *itr;
        ++itr;

        s->Delete();
    }

    m_socketLock.Release();

    /* shutdown the socket deleter */
    sSocketDeleter.Kill();
}

#endif      // NETLIB_IOCP
