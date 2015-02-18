/***
 * Demonstrike Core
 */

#pragma once

// sanity checking
enum AddItemResult : uint8
{
    ADD_ITEM_RESULT_ERROR           = 0x00,
    ADD_ITEM_RESULT_OK              = 0x01,
    ADD_ITEM_RESULT_SPLIT           = 0x02,
    ADD_ITEM_RESULT_ADDED_TO        = 0x04,
    ADD_ITEM_RESULT_DUPLICATED      = 0x08
};

struct SlotResult
{
    uint16 slot;
    AddItemResult result;

    void Init(AddItemResult res)
    {
        slot = 0xFFFF;
        result = res;
    }
    std::set<uint16> splitSlot;
};

class Item;
class Container;
class Player;
class UpdateData;
class ByteBuffer;

class SERVER_DECL ItemInterface
{
public:
    ItemInterface( Player* pPlayer );
    ~ItemInterface();
};
