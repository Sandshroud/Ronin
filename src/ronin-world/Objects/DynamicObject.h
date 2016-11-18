/***
 * Demonstrike Core
 */

#pragma once

class BaseSpell;
struct SpellEntry;

typedef std::set<uint64>  DynamicObjectList;

class SERVER_DECL DynamicObject : public WorldObject
{
public:
    DynamicObject( uint32 high, uint32 low, uint32 fieldCount = DYNAMICOBJECT_END );
    ~DynamicObject( );
    virtual void Init();
    virtual void Destruct();

    virtual bool IsDynamicObj() { return true; }

    // Don't think we need reactivate for world objects either
    virtual void Reactivate() {}

    RONIN_INLINE uint8 GetDynamicObjectPool() { return m_dynamicobjectPool; }
    RONIN_INLINE void AssignDynamicObjectPool(uint8 pool) { m_dynamicobjectPool = pool; }

    void Create(WorldObject* caster, BaseSpell* pSpell, float x, float y, float z, int32 duration, float radius);
    void UpdateTargets(uint32 p_time);

    void OnRemoveInRangeObject(WorldObject* pObj);
    void Remove();

    uint32 getLevel() { return casterLevel; }
    uint64 GetCasterGuid() { return casterGuid; }

protected:
    uint8 m_dynamicobjectPool;

    uint32 casterLevel;
    uint64 casterGuid;

    SpellEntry * m_spellProto;
    DynamicObjectList targets;

    int32 m_aliveDuration;
};
