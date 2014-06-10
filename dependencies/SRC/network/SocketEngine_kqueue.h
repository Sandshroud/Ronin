/***
 * Demonstrike Core
 */

#pragma once

#ifdef NETLIB_KQUEUE

/** This is the maximum number of connections you will be able to hold at one time.
 * adjust it accordingly.
 */
#define MAX_DESCRIPTORS 1024

class  kqueueEngine : public SocketEngine
{
    /** Created kqueue file descriptor
     */
    int kq;

    /** Thread running or not?
     */
    bool m_running;

    /** Binding for fd -> pointer
     */
    BaseSocket * fds[MAX_DESCRIPTORS];

public:
    kqueueEngine();
    ~kqueueEngine();

    /** Adds a socket to the engine.
     */
    void AddSocket(BaseSocket * s);

    /** Removes a socket from the engine. It should not receive any more events.
     */
    void RemoveSocket(BaseSocket * s);

    /** This is called when a socket has data to write for the first time.
     */
    void WantWrite(BaseSocket * s);

    /** Spawn however many worker threads this engine requires
     */
    void SpawnThreads();

    /** Called by SocketWorkerThread, this is the network loop.
     */
    void MessageLoop();
    
    /** Shutdown the socket engine, disconnect any associated sockets and 
     * deletes itself and the socket deleter.
     */
    void Shutdown();
};

/** Returns the socket engine
 */
inline void CreateSocketEngine() { new kqueueEngine; }

#endif      // NETLIB_KQUEUE
