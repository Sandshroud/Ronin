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

class BaseBuffer;
class BaseSocket
{
public:
    /** Virtual destructor
     */
    virtual ~BaseSocket() {}

    /** Returns the socket's file descriptor
     */
    inline SOCKET GetFd() { return m_fd; }

    /** Sets the socket's file descriptor
     * @param new_fd The new file descriptor
     */
    inline void SetFd(SOCKET new_fd) { m_fd = new_fd; }

    /** Is this socket in a read state? Or a write state?
     */
    virtual bool Writable() = 0;

    /** Virtual OnRead() callback
     */
    virtual void OnRead(size_t len) = 0;

    /** Virtual OnWrite() callback
     */
    virtual void OnWrite(size_t len) = 0;

    /** Virtual OnError() callback
     */
    virtual void OnError(int errcode) = 0;

    /** This is a windows-only implementation
     */
    virtual void OnAccept(void * pointer) = 0;

    /** Are we connected?
    */
    inline bool IsConnected() { return m_connected; }

    /** Are we deleted?
    */
    inline bool IsDeleted() { return m_deleted; }

    /** If for some reason we need to access the buffers directly 
     * (shouldn't happen) these will return them
     */
    inline BaseBuffer * GetReadBuffer() { return m_readBuffer; }
    inline BaseBuffer * GetWriteBuffer() { return m_writeBuffer; }

    /** Write mutex (so we don't post a write event twice
     */
    volatile long m_writeLock;

    /** Disconnects the socket
     */
    virtual void Disconnect() = 0;

    /** Deletes the socket
     */
    virtual void Delete() = 0;

protected:
    /** This socket's file descriptor
     */
    SOCKET m_fd;

    /** deleted/disconnected markers
     */
    bool m_deleted;
    bool m_connected;

    /** Read (inbound) buffer
     */
    BaseBuffer * m_readBuffer;

    /** Write (outbound) buffer
     */
    BaseBuffer * m_writeBuffer;

    /** Socket Addresses
    */
    sockaddr_in m_address;
};
