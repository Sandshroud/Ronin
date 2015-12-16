/***
 * Demonstrike Core
 */

#pragma once

enum HIGHGUID_TYPE
{
    HIGHGUID_TYPE_PLAYER            = 0x0000,
    HIGHGUID_TYPE_WAYPOINT          = 0x0100,
    HIGHGUID_TYPE_ITEM              = 0x0470,
    HIGHGUID_TYPE_GAMEOBJECT        = 0x0F11,
    HIGHGUID_TYPE_TRANSPORTER       = 0x0F12,
    HIGHGUID_TYPE_UNIT              = 0x0F13,
    HIGHGUID_TYPE_PET               = 0x0F14,
    HIGHGUID_TYPE_VEHICLE           = 0x0F15,
    HIGHGUID_TYPE_DYNAMICOBJECT     = 0x0F10,
    HIGHGUID_TYPE_CORPSE            = 0x0F50,
    HIGHGUID_TYPE_MO_TRANSPORT      = 0x01FC,
    HIGHGUID_TYPE_INSTANCE          = 0x01F4,
    HIGHGUID_TYPE_GROUP             = 0x01F5,
    HIGHGUID_TYPE_GUILD             = 0x1FF7,
//===============================================
    HIGHGUID_TYPE_MASK              = 0xFFFF0000,
    LOWGUID_ENTRY_MASK              = 0x0000FFFF,
};

enum TypeMask
{
    // Lower 16 bits are type
    TYPEMASK_TYPE_OBJECT        = 0x00000001,
    TYPEMASK_TYPE_ITEM          = 0x00000002,
    TYPEMASK_TYPE_CONTAINER     = 0x00000004,
    TYPEMASK_TYPE_UNIT          = 0x00000008,
    TYPEMASK_TYPE_PLAYER        = 0x00000010,
    TYPEMASK_TYPE_GAMEOBJECT    = 0x00000020,
    TYPEMASK_TYPE_DYNAMICOBJECT = 0x00000040,
    TYPEMASK_TYPE_CORPSE        = 0x00000080,
    TYPEMASK_TYPE_AREATRIGGER   = 0x00000100,
    TYPEMASK_TYPE_UNUSED2       = 0x00000200,
    TYPEMASK_TYPE_UNUSED3       = 0x00000400,
    TYPEMASK_TYPE_UNUSED4       = 0x00000800,
    TYPEMASK_TYPE_UNUSED5       = 0x00001000,
    TYPEMASK_TYPE_UNUSED6       = 0x00002000,
    TYPEMASK_TYPE_UNUSED7       = 0x00004000,
    TYPEMASK_TYPE_UNUSED8       = 0x00008000,
    TYPEMASK_TYPE_MASK          = 0x0000FFFF,
    // Upper 16 bits are flag
    TYPEMASK_FLAG_IN_GUILD      = 0x00010000
};

RONIN_INLINE uint8 HighestMaskType16(uint32 type)
{
    uint8 high=0;
    for(uint8 i = 0; i < 16; i++)
        if(type & 1<<i) high = i;
    return high;
}

enum TYPEID
{
    TYPEID_OBJECT           = 0,
    TYPEID_ITEM             = 1,
    TYPEID_CONTAINER        = 2,
    TYPEID_UNIT             = 3,
    TYPEID_PLAYER           = 4,
    TYPEID_GAMEOBJECT       = 5,
    TYPEID_DYNAMICOBJECT    = 6,
    TYPEID_CORPSE           = 7,
    TYPEID_AREATRIGGER      = 8
};

enum OBJECT_UPDATE_TYPE {
    UPDATETYPE_VALUES = 0,
    UPDATETYPE_CREATE_OBJECT = 1,
    UPDATETYPE_CREATE_PLAYEROBJ = 2,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 3
};

enum OBJECT_UPDATE_FLAGS
{
    UPDATEFLAG_NONE             = 0x0000,
    UPDATEFLAG_SELF             = 0x0001,
    UPDATEFLAG_TRANSPORT        = 0x0002,
    UPDATEFLAG_HAS_TARGET       = 0x0004,
    UPDATEFLAG_STA_MODEL        = 0x0008,
    UPDATEFLAG_DYN_MODEL        = 0x0010,
    UPDATEFLAG_LIVING           = 0x0020,
    UPDATEFLAG_STATIONARY_POS   = 0x0040,
    UPDATEFLAG_VEHICLE          = 0x0080,
    UPDATEFLAG_GO_TRANSPORT_POS = 0x0100,
    UPDATEFLAG_ROTATION         = 0x0200,
    UPDATEFLAG_UNK3             = 0x0400,
    UPDATEFLAG_ANIMKITS         = 0x0800,
    UPDATEFLAG_TRANSPORT_ARR    = 0x1000,
    UPDATEFLAG_ENABLE_PORTALS   = 0x2000,
    UPDATEFLAG_UNK2             = 0x4000,
};

enum ObjectAreaFlags
{
    OBJECT_AREA_FLAG_NONE           = 0x00,
    OBJECT_AREA_FLAG_INDOORS        = 0x01,
    OBJECT_AREA_FLAG_INCITY         = 0x02,
    OBJECT_AREA_FLAG_INSANCTUARY    = 0x04,
    OBJECT_AREA_FLAG_CONTESTED      = 0x08,
    OBJECT_AREA_FLAG_ALLIANCE_ZONE  = 0x10,
    OBJECT_AREA_FLAG_HORDE_ZONE     = 0x20,
    OBJECT_AREA_FLAG_ARENA_ZONE     = 0x40,
};

typedef struct
{
    uint32 school_type;
    int32 full_damage;
    uint32 resisted_damage;
} dealdamage;

class Spell;
class Aura;
class Unit;
class Player;
class Creature;
class GameObject;
class DynamicObject;
class WorldPacket;
class ByteBuffer;
class WorldSession;
class MapCell;
class MapMgr;

//===============================================
//  Object
//  Base class for every object
//===============================================
class SERVER_DECL Object
{
public:
    Object(uint64 guid, uint32 fieldCount = OBJECT_END);
    virtual ~Object();
    virtual void Init();
    virtual void Destruct();
    // Events are forced class so call virtual destruct through EventDestruct
    void EventDestruct() { Destruct(); }

    virtual void Update ( uint32 time ) { }

    // Value fields
    void __fastcall SetByte(uint32 index, uint32 index1, uint8 value);
    void __fastcall SetByteFlag( const uint32 index, const uint32 flag, uint8 newFlag);
    bool __fastcall HasByteFlag( const uint32 index, const uint32 flag, uint8 checkFlag);
    void __fastcall RemoveByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag);

    void __fastcall SetUInt16Value(uint16 index, uint8 offset, uint16 value);
    void __fastcall SetUInt32Value( const uint32 index, const uint32 value );
    void __fastcall SetUInt64Value( const uint32 index, const uint64 value );
    void __fastcall SetFlag( const uint32 index, uint32 newFlag );
    void __fastcall RemoveFlag( const uint32 index, uint32 oldFlag );
    void __fastcall SetFloatValue( const uint32 index, const float value );
    bool __fastcall HasFlag( const uint32 index, uint32 flag ) const { return (m_uint32Values[ index ] & flag) != 0;    }

    void __fastcall ModFloatValue(const uint32 index, const float value );
    void __fastcall ModSignedInt32Value(uint32 index, int32 value);
    void __fastcall ModUnsigned32Value(uint32 index, int32 mod);
    uint32 __fastcall GetModPUInt32Value(const uint32 index, const int32 value);

    RONIN_INLINE uint8 GetByte(uint32 index, uint32 byteIndex) { return ((uint8*)m_uint32Values)[index*4+byteIndex]; }
    RONIN_INLINE const uint16& GetUInt16Value(uint32 index, uint8 offset) const { ASSERT( index < m_valuesCount ); ASSERT( offset < 2 ); return *(((uint16*)&m_uint32Values[index])+offset); }
    RONIN_INLINE const uint32& GetUInt32Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_uint32Values[ index ]; }
    RONIN_INLINE const uint64& GetUInt64Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return *((uint64*)&(m_uint32Values[ index ])); }
    RONIN_INLINE const float& GetFloatValue( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_floatValues[ index ]; }
    RONIN_INLINE const float GetUInt32FloatValue(uint32 index) const { ASSERT( index < m_valuesCount ); return (float)m_uint32Values[index]; }

    // Update masks
    void SetUpdateField(uint32 index);
    bool HasUpdateField(uint32 index) { return m_updateMask.GetBit(index); }
    virtual void OnFieldUpdated(uint32 index) {}

    //! Mark values that need updating for specified player.
    bool _SetUpdateBits(UpdateMask *updateMask, uint32 updateFlags);

    uint16 GetUpdateFlag(Player *target);
    void GetUpdateFieldData(uint8 type, uint16 *&flags, uint16 &length);

    //! This includes any nested objects we have, inventory for example.
    virtual uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer *buf, uint32 updateFlags, uint32 expectedField = 0);

private:
    void _BuildCreateValuesUpdate( ByteBuffer *data, Player* target );
    void _BuildChangedValuesUpdate( ByteBuffer *data, UpdateMask *updateMask );

    void _BuildMovementUpdate( ByteBuffer *data, uint16 flags, Player* target );

public:
    virtual void DestroyForPlayer( Player* target, bool anim = false );

    ////////////////////////////////////////
    void ClearUpdateMask()
    {
        m_updateMask.Clear();
        m_objectUpdated = false;
    }

    // guid always comes first
    RONIN_INLINE WoWGuid& GetGUID() { return m_objGuid; }
    RONIN_INLINE uint16 GetValuesCount() const { return m_valuesCount; }

    uint16 GetTypeFlags() { return GetUInt32Value(OBJECT_FIELD_TYPE) & TYPEMASK_TYPE_MASK; }
    void SetTypeFlags(uint16 typeFlag) { SetFlag(OBJECT_FIELD_TYPE, typeFlag); };

    uint32 GetEntry() { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
    void SetEntry(uint32 value) { SetUInt32Value(OBJECT_FIELD_ENTRY, value); }

    float GetObjectScale() { return GetFloatValue(OBJECT_FIELD_SCALE_X); }
    void SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); };

    RONIN_INLINE uint32 GetEntryFromGUID() { return m_objGuid.getEntry(); }
    RONIN_INLINE uint32 GetHighGUID() { return m_objGuid.getHigh(); }
    RONIN_INLINE uint32 GetLowGUID() { return m_objGuid.getLow(); }

    // type
    RONIN_INLINE uint8 GetTypeId() { return HighestMaskType16(GetTypeFlags()); }
    RONIN_INLINE bool IsUnit() { return (GetTypeFlags() & TYPEMASK_TYPE_UNIT); }
    RONIN_INLINE bool IsItem() { return (GetTypeFlags() & TYPEMASK_TYPE_ITEM); }
    RONIN_INLINE bool IsCreature() { return GetTypeId() == TYPEID_UNIT; }
    RONIN_INLINE bool IsPlayer() { return GetTypeId() == TYPEID_PLAYER; }
    RONIN_INLINE bool IsGameObject() { return GetTypeId() == TYPEID_GAMEOBJECT; }
    RONIN_INLINE bool IsDynamicObj() { return GetTypeId() == TYPEID_DYNAMICOBJECT; }
    RONIN_INLINE bool IsCorpse() { return GetTypeId() == TYPEID_CORPSE; }
    virtual bool IsContainer() { return false; }
    virtual bool IsObject() { return false; }
    virtual bool IsPet() { return false; }
    virtual bool IsTotem() { return false; }
    virtual bool IsSummon() { return false; }
    virtual bool IsVehicle() { return false; }
    virtual bool IsTransport() { return false; }

    // In world bools
    virtual bool IsInWorld() { return m_inWorld; }
    void SetInWorld(bool res) { m_inWorld = res; }

protected:
    //! Object properties.
    union {
        uint32 *m_uint32Values;
        float *m_floatValues;
    };

    //! Object's guid
    WoWGuid m_objGuid;
    //! Flags for building update data
    uint32 m_updateFlags;
    //! Number of properties
    uint32 m_valuesCount;
    //! Notification flags for updates
    uint16 m_notifyFlags;
    //! List of object properties that need updating.
    UpdateMask m_updateMask;
    //! True if object is in world
    bool m_inWorld;
    //! True if object was updated
    bool m_objectUpdated;

public:
    RONIN_INLINE ObjectLoot* GetLoot() { return &m_loot; }
    RONIN_INLINE bool IsLooted() { return m_looted; }
    RONIN_INLINE void SetLooted() { m_looted = true; }

    // empties loot vector
    void ClearLoot();

private:
    // loooooot
    ObjectLoot m_loot;
    bool m_looted;
};

//===============================================
//===============================================
//===============================================
class SERVER_DECL WorldObject : public Object, public EventableObject
{
public:
    typedef std::unordered_set<WoWGuid> InRangeSet;
    typedef std::unordered_map<WoWGuid, WorldObject*> InRangeMap;
    typedef std::unordered_set<WorldObject*> InRangeWorldObjectSet;

public:
    WorldObject(uint64 guid, uint32 fieldCount = OBJECT_END);
    virtual ~WorldObject( );
    virtual void Init();
    virtual void Destruct();

    virtual bool IsObject() { return true; }

    float GetCHeightForPosition(bool checkwater = false, float x = 0.0f, float y = 0.0f, float z = 0.0f);

    int32 DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

    //! True if object exists in world
    virtual bool IsInWorld() { return m_mapMgr != NULL; }
    virtual void RemoveFromWorld(bool free_guid);

    void PushToWorld(MapMgr* );
    virtual void OnPushToWorld() { }
    virtual void OnPrePushToWorld() { }

    virtual void OnFieldUpdated(uint32 index);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    // Field values
    virtual uint32 getLevel() = 0;

    void CastSpell(WorldObject* Target, SpellEntry* Sp, bool triggered);
    void CastSpell(WorldObject* Target, uint32 SpellID, bool triggered);
    void CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered);
    void CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered);

    RONIN_INLINE const float& GetPositionX( ) const { return m_position.x; }
    RONIN_INLINE const float& GetPositionY( ) const { return m_position.y; }
    RONIN_INLINE const float& GetPositionZ( ) const { return m_position.z; }
    RONIN_INLINE const float& GetOrientation( ) const { return m_position.o; }
    RONIN_INLINE void SetOrientation( float &o ) { m_position.o = NormAngle(o); }

    RONIN_INLINE float GetSpawnX() { return m_spawnLocation.x; }
    RONIN_INLINE float GetSpawnY() { return m_spawnLocation.y; }
    RONIN_INLINE float GetSpawnZ() { return m_spawnLocation.z; }
    RONIN_INLINE float GetSpawnO() { return m_spawnLocation.o; }

    RONIN_INLINE const LocationVector & GetPosition() { return m_position; }
    RONIN_INLINE LocationVector & GetPositionNC() { return m_position; }
    RONIN_INLINE LocationVector * GetPositionV() { return &m_position; }
    RONIN_INLINE void GetPosition2D(float &x, float &y) { x = m_position.x; y = m_position.y; }
    RONIN_INLINE void GetPosition(LocationVector &loc) { loc = m_position; }
    RONIN_INLINE void GetPosition(float &x, float &y, float &z) { x = m_position.x; y = m_position.y; z = m_position.z; }
    RONIN_INLINE void GetPosition(float &x, float &y, float &z, float &o) { x = m_position.x; y = m_position.y; z = m_position.z; o = m_position.o; }

    bool IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta);

    //Distance Calculation
    float CalcDistance(WorldObject* Ob);
    float CalcDistance(float ObX, float ObY, float ObZ);
    float CalcDistance(WorldObject* Oa, WorldObject* Ob);
    float CalcDistance(WorldObject* Oa, float ObX, float ObY, float ObZ);
    float CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ);

    //! Only for MapMgr use
    RONIN_INLINE MapCell* GetMapCell() const { return m_mapCell; }
    //! Only for MapMgr use
    RONIN_INLINE void SetMapCell(MapCell* cell) { m_mapCell = cell; }
    //! Only for MapMgr use
    RONIN_INLINE MapMgr* GetMapMgr() const { return m_mapMgr; }

    RONIN_INLINE void SetMapId(uint32 newMap) { m_mapId = newMap; }
    RONIN_INLINE const uint32 GetMapId( ) const { return m_mapId; }

    void SetZoneId(uint32 newZone);
    void UpdateAreaInfo(MapMgr *mgr = NULL);
    RONIN_INLINE const uint32& GetAreaId( ) const { return m_areaId; }
    RONIN_INLINE const uint32& GetZoneId( ) const { return m_zoneId; }
    RONIN_INLINE void SetLastMovementZone(uint32 zone) { m_lastMovementZone = zone; }
    RONIN_INLINE uint32 GetLastMovementZone() { return m_lastMovementZone; }

    //use it to check if a object is in range of another
    bool isInRange(WorldObject* target, float range);

    // Use it to Check if a object is in front of another one
    bool isTargetInFront(WorldObject* target);
    bool isTargetInBack(WorldObject* target) { return !isTargetInFront(target); };

    bool isInFrontOfTarget(WorldObject* target) { return target->isTargetInFront(this); };
    bool isInBackOfTarget(WorldObject* target) { return target->isTargetInBack(this); };

    // Check to see if an object is in front of a target in a specified arc (in degrees)
    bool isInArc(WorldObject* target , float degrees);

    /* Calculates the angle between two Positions */
    float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );
    float calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );

    /* converts to 360 > x > 0 */
    float getEasyAngle( float angle );

    RONIN_INLINE float CalcDistance(LocationVector & comp) { return comp.Distance(m_position); }
    RONIN_INLINE float GetDistanceSq(LocationVector & comp) { return comp.DistanceSq(m_position); }
    RONIN_INLINE const float GetDistanceSq(WorldObject* obj)
    {
        if(obj->GetMapId() != m_mapId)
            return 40000.0f; //enough for out of range
        return m_position.DistanceSq(obj->GetPosition());
    }

    RONIN_INLINE const float GetDistanceSq(float x, float y, float z)
    {
        return m_position.DistanceSq(x, y, z);
    }

    RONIN_INLINE const float GetDistance2dSq( WorldObject* obj )
    {
        if( obj->GetMapId() != m_mapId )
            return 40000.0f; //enough for out of range
        return m_position.Distance2DSq( obj->m_position );
    }

    RONIN_INLINE float GetDistance2dSq(float x, float y)
    {
        return m_position.Distance2DSq( x, y );
    }

    // In-range object management, not sure if we need it
    RONIN_INLINE bool IsInRangeMap( WorldObject* pObj )
    {
        return m_objectsInRange.find(pObj->GetGUID()) != m_objectsInRange.end();
    }

    RONIN_INLINE void AddInRangeObject(WorldObject* pObj)
    {
        if( pObj == NULL )
            return;

        m_objectsInRange.insert(pObj->GetGUID());
        m_inRangeObjects.insert( std::make_pair(pObj->GetGUID(), pObj) );
        OnAddInRangeObject(pObj);
    }

    RONIN_INLINE bool RemoveInRangeObject( WorldObject* obj )
    {
        ASSERT(obj);
        InRangeMap::iterator itr = m_inRangeObjects.find(obj->GetGUID());
        if( itr == m_inRangeObjects.end() )
            return false;

        m_objectsInRange.erase(itr->first);
        OnRemoveInRangeObject(itr->second);
        itr = m_inRangeObjects.erase(itr);
        return true;
    }

    RONIN_INLINE WorldObject *GetInRangeObject(WoWGuid guid)
    {
        if(GetGUID() == guid)
            return this;

        InRangeMap::iterator itr = m_inRangeObjects.find(guid);
        if(itr != m_inRangeObjects.end())
            return itr->second;
        return NULL;
    }

    template<typename T> RONIN_INLINE T *GetInRangeObject(WoWGuid guid)
    {
        if(GetGUID() == guid)
            return castPtr<T>(this);

        InRangeMap::iterator itr;
        if((itr = m_inRangeObjects.find(guid)) != m_inRangeObjects.end())
            return castPtr<T>(itr->second);
        return NULL;
    }

    RONIN_INLINE bool HasInRangeObjects() { return !m_objectsInRange.empty(); }

    RONIN_INLINE virtual void OnAddInRangeObject( WorldObject* pObj )
    {
        if(pObj->IsGameObject())
            m_inRangeGameObjects.insert(pObj->GetGUID());
        else if(pObj->IsUnit())
        {
            m_inRangeUnits.insert(pObj->GetGUID());
            if(pObj->IsPlayer())
                m_inRangePlayers.insert(pObj->GetGUID());
            else m_inRangeCreatures.insert(pObj->GetGUID());
        }
    }

    RONIN_INLINE virtual void OnRemoveInRangeObject( WorldObject* pObj )
    {
        m_inRangeUnits.erase(pObj->GetGUID());
        m_inRangePlayers.erase(pObj->GetGUID());
        m_inRangeCreatures.erase(pObj->GetGUID());
        m_inRangeGameObjects.erase(pObj->GetGUID());
    }

    RONIN_INLINE virtual void ClearInRangeSet()
    {
        m_objectsInRange.clear();
        m_inRangeObjects.clear();
        m_inRangeUnits.clear();
        m_inRangePlayers.clear();
        m_inRangeCreatures.clear();
        m_inRangeGameObjects.clear();
    }

    RONIN_INLINE size_t GetInRangeCount() { return m_objectsInRange.size(); }
    RONIN_INLINE size_t GetInRangeUnitCount() { return m_inRangeUnits.size(); }
    RONIN_INLINE size_t GetInRangePlayerCount() { return m_inRangePlayers.size(); }
    RONIN_INLINE size_t GetInRangeCreatureCount() { return m_inRangeCreatures.size(); }
    RONIN_INLINE size_t GetInRangeGameObjectCount() { return m_inRangeGameObjects.size(); }

    RONIN_INLINE InRangeSet::iterator GetInRangeUnitSetBegin() { return m_inRangeUnits.begin(); }
    RONIN_INLINE InRangeSet::iterator GetInRangeUnitSetEnd() { return m_inRangeUnits.end(); }
    RONIN_INLINE InRangeSet::iterator GetInRangePlayerSetBegin() { return m_inRangePlayers.begin(); }
    RONIN_INLINE InRangeSet::iterator GetInRangePlayerSetEnd() { return m_inRangePlayers.end(); }
    RONIN_INLINE InRangeSet::iterator GetInRangeCreatureSetBegin() { return m_inRangeCreatures.begin(); }
    RONIN_INLINE InRangeSet::iterator GetInRangeCreatureSetEnd() { return m_inRangeCreatures.end(); }
    RONIN_INLINE InRangeSet::iterator GetInRangeGameObjectSetBegin() { return m_inRangeGameObjects.begin(); }
    RONIN_INLINE InRangeSet::iterator GetInRangeGameObjectSetEnd() { return m_inRangeGameObjects.end(); }

    RONIN_INLINE InRangeMap const* GetInRangeMap() { return &m_inRangeObjects; }
    RONIN_INLINE InRangeMap::iterator GetInRangeMapBegin() { return m_inRangeObjects.begin(); }
    RONIN_INLINE InRangeMap::iterator GetInRangeMapEnd() { return m_inRangeObjects.end(); }
    RONIN_INLINE InRangeMap::iterator FindInRangeSet(WorldObject * obj) { ASSERT(obj); return m_inRangeObjects.find(obj->GetGUID()); }

    void __fastcall SendMessageToSet(WorldPacket *data, bool self,bool myteam_only=false);
    void OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self);

    void SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool no_remove_auras = false);

    //*****************************************************************************************
    //* SpellLog packets just to keep the code cleaner and better to read
    //*****************************************************************************************
    void SendSpellLog(WorldObject* Caster, WorldObject* Target,uint32 Ability, uint8 SpellLogType);
    void SendSpellNonMeleeDamageLog( WorldObject* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet );
    void SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate );

    //object faction
    void SetFactionTemplate(uint32 templateId);
    FactionTemplateEntry *GetFactionTemplate() { return m_factionTemplate; };
    uint32 GetFactionID() { return m_factionTemplate ? m_factionTemplate->Faction : 0; }
    FactionEntry *GetFaction() { return m_factionTemplate ? m_factionTemplate->GetFaction() : NULL; }

    RONIN_INLINE void SetInstanceID(int32 instance) { m_instanceId = instance; }
    RONIN_INLINE int32 GetInstanceID() { return m_instanceId; }

    int32 event_GetInstanceID();

    void DestroyForInrange(bool anim = false);
    WorldPacket *BuildTeleportAckMsg( const LocationVector & v);

    bool Active;
    bool CanActivate();
    void Activate(MapMgr* mgr);
    void Deactivate(MapMgr* mgr);
    RONIN_INLINE void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

    void PlaySoundToPlayer( Player* plr, uint32 sound_entry );
    void PlaySoundToSet(uint32 sound_entry);
    void EventSpellHit(Spell* pSpell);

    bool AreaCanInteract(WorldObject *pObj);
    bool PhasedCanInteract(WorldObject* pObj);

    // Area flags
    bool HasAreaFlag(uint8 areaFlag) { return (m_areaFlags & areaFlag); };
    uint8 const GetAreaFlags() { return m_areaFlags; };

    int32 GetPhaseMask() { return 0x01; }

protected:
    void _Create( uint32 mapid, float x, float y, float z, float ang);

    /* Main Function called by isInFront(); */
    bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y );

    //! Zone id.
    uint32 m_zoneId;
    //! Area id.
    uint32 m_areaId;
    //! Area Flags.
    uint8 m_areaFlags;
    //! Continent/map id.
    int32 m_mapId;
    //! Instance Id
    int32 m_instanceId;
    //! Last set Movement zone
    uint32 m_lastMovementZone;

    //! Map manager
    MapMgr* m_mapMgr;
    //! Current map cell
    MapCell *m_mapCell;
    //! Current object faction
    FactionTemplateEntry *m_factionTemplate;
    // Current map location
    LocationVector m_position, m_spawnLocation, m_lastMapUpdatePosition;

    //! Set of Objects in range.
    InRangeSet m_objectsInRange, m_inRangeUnits, m_inRangePlayers, m_inRangeCreatures, m_inRangeGameObjects;
    InRangeMap m_inRangeObjects;

public:
    bool IsInLineOfSight(WorldObject* pObj);
    bool IsInLineOfSight(float x, float y, float z);
    int32 GetSpellBaseCost(SpellEntry *sp);
};
