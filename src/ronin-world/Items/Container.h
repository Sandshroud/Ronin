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

#define INVENTORY_SLOT_NOT_SET -1
#define MAX_CONTAINER_SLOTS 36

class SERVER_DECL Container : public Item
{
public:
    Container(ItemPrototype *proto, uint32 counter);
    ~Container();

    virtual bool IsContainer() { return true; }

    void LoadFromDB( Field*fields);

    bool AddItem(uint8 slot, Item* item);
    bool AddItemToFreeSlot(Item* pItem, uint8 *r_slot);

    Item* GetItem(WoWGuid guid);
    Item* GetItem(uint8 slot) { ASSERT(slot < GetSlotCount()); return m_itemSlots[slot]; }

    int16 FindFreeSlot(int16 ourSlot, std::set<std::pair<int16, int16>> *ignoreSlots = NULL);
    bool HasItems();

    void SwapItems(uint8 SrcSlot, uint8 DstSlot);
    Item* SafeRemoveAndRetreiveItemFromSlot(uint8 slot, bool destroy); //doesnt destroy item from memory
    bool SafeFullRemoveItemFromSlot(uint8 slot); //destroys item fully

    void SaveBagToDB(uint8 slot, bool first, QueryBuffer * buf);

    uint8 GetSlotCount() { return (m_uint32Values[CONTAINER_FIELD_NUM_SLOTS]&0xFF); }
    void SetSlotCount(uint32 count) { SetUInt32Value(CONTAINER_FIELD_NUM_SLOTS, std::min<uint32>(MAX_CONTAINER_SLOTS, count)); }

protected:
    Item **m_itemSlots;

};
