/***
 * Demonstrike Core
 */

#pragma once

#include "Common.h"
#include "ByteBuffer.h"
#include "hearthstone_log.h"
#include "Client/OpcodeList.h"

class SERVER_DECL WorldPacket : public ByteBuffer
{
public:
    __inline WorldPacket() : ByteBuffer(), m_opcode(0) { }
    __inline WorldPacket(uint16 opcode, size_t res) : ByteBuffer(res), m_opcode(opcode) {}
    __inline WorldPacket(size_t res) : ByteBuffer(res), m_opcode(0) { }
    __inline WorldPacket(const WorldPacket &packet) : ByteBuffer(packet), m_opcode(packet.m_opcode) {}
    __inline WorldPacket(Opcodes opcode) : ByteBuffer(), m_opcode(opcode) { }

    //! Clear packet and set opcode all in one mighty blow
    __inline void Initialize(uint16 opcode )
    {
        clear();
        m_opcode = opcode;
    }

    __inline void Initialize(uint16 opcode, size_t newres )
    {
        clear();
        _storage.reserve(newres);
        m_opcode = opcode;
    }

    __inline uint16 GetOpcode() const { return m_opcode; }
    __inline void SetOpcode(uint16 opcode) { m_opcode = opcode; }

protected:
    uint16 m_opcode;

public:
    void print_storage() const
    {
        if(sLog.isOutProcess())
        {
            sLog.outDebugInLine("STORAGE_SIZE: %lu\n", ulong(size()) );
            sLog.outDebugInLine("START: ");
            for(uint32 i = 0; i < size(); ++i)
                sLog.outDebugInLine("%u - ", read<uint8>(i) );
            sLog.outDebugInLine("END\n");
        }
    }
};
