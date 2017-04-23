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

class BaseSpell;
class DynamicObject;
struct SpellEntry;

typedef std::set<WoWGuid>  DynamicObjectList;

class DynamicObjectTargetCallback { public: virtual void operator()(DynamicObject *obj, Unit *caster, Unit *target, float range) = 0; };
class FillDynamicObjectTargetMapCallback : public DynamicObjectTargetCallback { virtual void operator()(DynamicObject *obj, Unit *caster, Unit *target, float range); };

class SERVER_DECL DynamicObject : public WorldObject
{
    friend class DynamicObjectTargetCallback;
    friend class FillDynamicObjectTargetMapCallback;

public:
    DynamicObject( uint32 high, uint32 low, uint32 fieldCount = DYNAMICOBJECT_END );
    ~DynamicObject( );
    virtual void Init();
    virtual void Destruct();
    virtual void Update(uint32 msTime, uint32 uiDiff);

    virtual bool IsDynamicObj() { return true; }

    // Don't think we need reactivate for world objects either
    virtual void Reactivate() {}

    RONIN_INLINE uint8 GetDynamicObjectPool() { return m_dynamicobjectPool; }
    RONIN_INLINE void AssignDynamicObjectPool(uint8 pool) { m_dynamicobjectPool = pool; }

    void Create(WorldObject* caster, BaseSpell* pSpell, float x, float y, float z, int32 duration, float radius);
    void UpdateTargets(uint32 p_time);
    void Remove();

    uint32 getLevel() { return casterLevel; }
    uint64 GetCasterGuid() { return casterGuid; }

private:
    bool IsInTargetSet(Unit *target) { return (targets.find(target->GetGUID()) != targets.end()); }
    bool IsTargettingStealth() { return m_spellProto->isSpellStealthTargetCapable(); }

protected:
    uint8 m_dynamicobjectPool;

    uint32 casterLevel;
    uint64 casterGuid;

    SpellEntry * m_spellProto;
    DynamicObjectList targets;

    int32 m_aliveDuration;
};
