/***
 * Demonstrike Core
 */

#pragma once

enum HIGHGUID_TYPE
{
    HIGHGUID_TYPE_PLAYER            = 0x0000,
    HIGHGUID_TYPE_WAYPOINT          = 0x1000,
    HIGHGUID_TYPE_ITEM              = 0x4000,
    HIGHGUID_TYPE_CONTAINER         = 0x4000,
    HIGHGUID_TYPE_GAMEOBJECT        = 0xF110,
    HIGHGUID_TYPE_TRANSPORTER       = 0xF120,
    HIGHGUID_TYPE_CREATURE          = 0xF130,
    HIGHGUID_TYPE_UNIT              = 0xF130,
    HIGHGUID_TYPE_PET               = 0xF140,
    HIGHGUID_TYPE_VEHICLE           = 0xF150,
    HIGHGUID_TYPE_DYNAMICOBJECT     = 0xF100,
    HIGHGUID_TYPE_CORPSE            = 0xF101,
    HIGHGUID_TYPE_AREATRIGGER       = 0xF102,
    HIGHGUID_TYPE_MO_TRANSPORT      = 0x1FC0,
    HIGHGUID_TYPE_GROUP             = 0x1F50,
    HIGHGUID_TYPE_GUILD             = 0x1FF6,
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
    TYPEMASK_TYPE_AIGROUP       = 0x00000200,
    TYPEMASK_TYPE_UNUSED        = 0x00000400,
    TYPEMASK_TYPE_UNUSED2       = 0x00000800,
    TYPEMASK_TYPE_UNUSED3       = 0x00001000,
    TYPEMASK_TYPE_UNUSED4       = 0x00002000,
    TYPEMASK_TYPE_UNUSED5       = 0x00004000,
    TYPEMASK_TYPE_UNUSED6       = 0x00008000,
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
    TYPEID_AREATRIGGER      = 8,
    TYPEID_AIGROUP          = 9,
    TYPEID_UNUSED           = 10,//used to signal invalid reference (object dealocated but someone is still using it)
};

enum OBJECT_UPDATE_TYPE {
    UPDATETYPE_VALUES = 0,
    //  8 bytes - GUID
    //  Goto Update Block
    UPDATETYPE_CREATE_OBJECT = 1,
    //  8 bytes - GUID
    //  1 byte - Object Type (*)
    //  Goto Position Update
    //  Goto Update Block
    UPDATETYPE_CREATE_PLAYEROBJ = 2,
    //  8 bytes - GUID
    //  1 byte - Object Type (*)
    //  Goto Position Update
    //  Goto Update Block
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 3
    //  4 bytes - Count
    //  Loop Count Times:
    //  8 bytes - GUID
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
    UPDATEFLAG_UNK5             = 0x1000,
    UPDATEFLAG_UNK6             = 0x2000
};

enum ObjectAreaFlags
{
    OBJECT_AREA_FLAG_NONE           = 0x00,
    OBJECT_AREA_FLAG_INCITY         = 0x01,
    OBJECT_AREA_FLAG_INSANCTUARY    = 0x02,
    OBJECT_AREA_FLAG_CONTESTED      = 0x04,
    OBJECT_AREA_FLAG_ALLIANCE_ZONE  = 0x08,
    OBJECT_AREA_FLAG_HORDE_ZONE     = 0x10,
    OBJECT_AREA_FLAG_ARENA_ZONE     = 0x20
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

    HEARTHSTONE_INLINE uint8 GetByte(uint32 index, uint32 byteIndex) { return ((uint8*)m_uint32Values)[index*4+byteIndex]; }
    HEARTHSTONE_INLINE const uint16& GetUInt16Value(uint32 index, uint8 offset) const { ASSERT( index < m_valuesCount ); ASSERT( offset < 2 ); return *(((uint16*)&m_uint32Values[index])+offset); }
    HEARTHSTONE_INLINE const uint32& GetUInt32Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_uint32Values[ index ]; }
    HEARTHSTONE_INLINE const uint64& GetUInt64Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return *((uint64*)&(m_uint32Values[ index ])); }
    HEARTHSTONE_INLINE const float& GetFloatValue( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_floatValues[ index ]; }

    void SetUpdateField(uint32 index);
    virtual void OnFieldUpdated(uint32 index) {}
    bool HasUpdateField(uint32 index) { return m_updateMask.GetBit(index); }

    //! This includes any nested objects we have, inventory for example.
    virtual uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer *buf, Player* target );
    uint32 __fastcall BuildOutOfRangeUpdateBlock( ByteBuffer *buf );

    virtual void DestroyForPlayer( Player* target, bool anim = false );

    ////////////////////////////////////////
    void ClearUpdateMask()
    {
        m_updateMask.Clear();
        m_objectUpdated = false;
    }

    //! Fill values with data from a space separated string of uint32s.
    void LoadValues(const char* data);

    HEARTHSTONE_INLINE uint16 GetValuesCount() const { return m_valuesCount; }

    // guid always comes first
    HEARTHSTONE_INLINE WoWGuid& GetGUID() { return m_objGuid; }

    uint16 GetTypeFlags() { return GetUInt32Value(OBJECT_FIELD_TYPE) & TYPEMASK_TYPE_MASK; }
    void SetTypeFlags(uint16 typeFlag) { SetFlag(OBJECT_FIELD_TYPE, typeFlag); };

    uint32 GetEntry() { return GetUInt32Value(OBJECT_FIELD_ENTRY); }
    void SetEntry(uint32 value) { SetUInt32Value(OBJECT_FIELD_ENTRY, value); }

    float GetObjectScale() { return GetUInt32Value(OBJECT_FIELD_SCALE_X); }
    void SetObjectScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); };

    HEARTHSTONE_INLINE uint32 GetEntryFromGUID() { return m_objGuid.getEntry(); }
    HEARTHSTONE_INLINE uint32 GetHighGUID() { return m_objGuid.getHigh(); }
    HEARTHSTONE_INLINE uint32 GetLowGUID() { return m_objGuid.getLow(); }

    // type
    HEARTHSTONE_INLINE uint8 GetTypeId() { return HighestMaskType16(GetTypeFlags()); }
    HEARTHSTONE_INLINE bool IsUnit() { return (GetTypeFlags() & TYPEMASK_TYPE_UNIT); }
    HEARTHSTONE_INLINE bool IsItem() { return (GetTypeFlags() & TYPEMASK_TYPE_ITEM); }
    HEARTHSTONE_INLINE bool IsContainer() { return GetTypeId() == TYPEID_CONTAINER; }
    HEARTHSTONE_INLINE bool IsCreature() { return GetTypeId() == TYPEID_UNIT; }
    HEARTHSTONE_INLINE bool IsPlayer() { return GetTypeId() == TYPEID_PLAYER; }
    HEARTHSTONE_INLINE bool IsGameObject() { return GetTypeId() == TYPEID_GAMEOBJECT; }
    HEARTHSTONE_INLINE bool IsDynamicObj() { return GetTypeId() == TYPEID_DYNAMICOBJECT; }
    HEARTHSTONE_INLINE bool IsCorpse() { return GetTypeId() == TYPEID_CORPSE; }
    virtual bool IsObject() { return false; }
    virtual bool IsPet() { return false; }
    virtual bool IsTotem() { return false; }
    virtual bool IsSummon() { return false; }
    virtual bool IsVehicle() { return false; }
    virtual bool IsTransport() { return false; }

protected:
    //! Mark values that need updating for specified player.
    bool _SetUpdateBits(UpdateMask *updateMask, Player* target);

    uint32 GetUpdateFlag(Player *target);
    void GetUpdateFieldData(uint8 type, uint32 *&flags, uint32 &length);
    void _BuildCreateValuesUpdate( ByteBuffer *data, Player* target );
    void _BuildChangedValuesUpdate( ByteBuffer *data, UpdateMask *updateMask, Player* target );

    void _BuildMovementUpdate( ByteBuffer *data, uint16 flags, Player* target );
    virtual void _WriteLivingMovementUpdate(ByteBuffer *bits, ByteBuffer *bytes, Player *target);
    virtual void _WriteStationaryPosition(ByteBuffer *bits, ByteBuffer *bytes, Player *target);
    virtual void _WriteTargetMovementUpdate(ByteBuffer *bits, ByteBuffer *bytes, Player *target);
    Mutex m_objlock;

    //! Object properties.
    union {
        uint32 *m_uint32Values;
        float *m_floatValues;
    };

    //! Object's guid
    WoWGuid m_objGuid;
    //! Number of properties
    uint32 m_valuesCount;
    //! Notification flags for updates
    uint16 m_notifyFlags;
    //! List of object properties that need updating.
    UpdateMask m_updateMask;
    //! True if object was updated
    bool m_objectUpdated;

public:
    // declaration to fix scripting
    HEARTHSTONE_INLINE Loot* GetLoot() { return &m_loot; }
    HEARTHSTONE_INLINE bool IsLooted() { return m_looted; }
    HEARTHSTONE_INLINE void SetLooted() { m_looted = true; }

    // empties loot vector
    void ClearLoot();

private:
    // loooooot
    Loot m_loot;
    bool m_looted;
};

//===============================================
//===============================================
//===============================================
class SERVER_DECL WorldObject : public Object, public EventableObject
{
public:
    typedef std::unordered_set<WorldObject*> InRangeSet;
    typedef std::unordered_set<Unit*> InRangeUnitSet;
    typedef std::unordered_set<Player*> InRangePlayerSet;
    typedef std::unordered_set<GameObject*> InRangeGameObjectSet;

public:
    WorldObject(uint64 guid, uint32 fieldCount = OBJECT_END);
    virtual ~WorldObject( );
    virtual void Init();
    virtual void Destruct();

    virtual bool IsObject() { return true; }

    WorldPacket* BuildFieldUpdatePacket(uint32 index,uint32 value);
    void BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value);
    void BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value);

    float GetCHeightForPosition(bool checkwater = false, float x = 0.0f, float y = 0.0f, float z = 0.0f);

    int32 DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

    //! True if object exists in world
    HEARTHSTONE_INLINE bool IsInWorld() { return m_mapMgr != NULL; }
    virtual void AddToWorld();
    virtual void AddToWorld(MapMgr* pMapMgr);
    void PushToWorld(MapMgr* );
    virtual void OnPushToWorld() { }
    virtual void OnPrePushToWorld() { }
    virtual void RemoveFromWorld(bool free_guid);
    virtual void _WriteLivingMovementUpdate(ByteBuffer *bits, ByteBuffer *bytes, Player *target) {};
    virtual void _WriteStationaryPosition(ByteBuffer *bits, ByteBuffer *bytes, Player *target) {};

    virtual void OnFieldUpdated(uint32 index);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    void CastSpell(WorldObject* Target, SpellEntry* Sp, bool triggered);
    void CastSpell(WorldObject* Target, uint32 SpellID, bool triggered);
    void CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered);
    void CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered);

    HEARTHSTONE_INLINE const float& GetPositionX( ) const { return m_position.x; }
    HEARTHSTONE_INLINE const float& GetPositionY( ) const { return m_position.y; }
    HEARTHSTONE_INLINE const float& GetPositionZ( ) const { return m_position.z; }
    HEARTHSTONE_INLINE const float& GetOrientation( ) const { return m_position.o; }
    HEARTHSTONE_INLINE void SetOrientation( float &o ) { m_position.o = o; }

    virtual float GetSpawnX() { return 0.f; }
    virtual float GetSpawnY() { return 0.f; }
    virtual float GetSpawnZ() { return 0.f; }
    virtual float GetSpawnO() { return 0.f; }

    bool canWalk();
    bool canSwim();
    bool canFly();

    HEARTHSTONE_INLINE const LocationVector & GetPosition() { return m_position; }
    HEARTHSTONE_INLINE LocationVector & GetPositionNC() { return m_position; }
    HEARTHSTONE_INLINE LocationVector * GetPositionV() { return &m_position; }
    HEARTHSTONE_INLINE void GetPosition2D(float &x, float &y) { x = m_position.x; y = m_position.y; }
    HEARTHSTONE_INLINE void GetPosition(float &x, float &y, float &z) { x = m_position.x; y = m_position.y; z = m_position.z; }
    HEARTHSTONE_INLINE void GetPosition(float &x, float &y, float &z, float &o) { x = m_position.x; y = m_position.y; z = m_position.z; o = m_position.o; }

    bool IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta);

    //Distance Calculation
    float CalcDistance(WorldObject* Ob);
    float CalcDistance(float ObX, float ObY, float ObZ);
    float CalcDistance(WorldObject* Oa, WorldObject* Ob);
    float CalcDistance(WorldObject* Oa, float ObX, float ObY, float ObZ);
    float CalcDistance(float OaX, float OaY, float OaZ, float ObX, float ObY, float ObZ);

    //! Only for MapMgr use
    HEARTHSTONE_INLINE MapCell* GetMapCell() const { return m_mapCell; }
    //! Only for MapMgr use
    HEARTHSTONE_INLINE void SetMapCell(MapCell* cell) { m_mapCell = cell; }
    //! Only for MapMgr use
    HEARTHSTONE_INLINE MapMgr* GetMapMgr() const { return m_mapMgr; }

    HEARTHSTONE_INLINE void SetMapId(uint32 newMap) { m_mapId = newMap; }
    HEARTHSTONE_INLINE const uint32 GetMapId( ) const { return m_mapId; }

    void SetZoneId(uint32 newZone);
    void UpdateAreaInfo(MapMgr *mgr = NULL);
    HEARTHSTONE_INLINE const uint32& GetAreaId( ) const { return m_areaId; }
    HEARTHSTONE_INLINE const uint32& GetZoneId( ) const { return m_zoneId; }
    HEARTHSTONE_INLINE void SetLastMovementZone(uint32 zone) { m_lastMovementZone = zone; }
    HEARTHSTONE_INLINE uint32 GetLastMovementZone() { return m_lastMovementZone; }

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

    HEARTHSTONE_INLINE float CalcDistance(LocationVector & comp) { return comp.Distance(m_position); }
    HEARTHSTONE_INLINE float GetDistanceSq(LocationVector & comp) { return comp.DistanceSq(m_position); }
    HEARTHSTONE_INLINE const float GetDistanceSq(WorldObject* obj)
    {
        if(obj->GetMapId() != m_mapId)
            return 40000.0f; //enough for out of range
        return m_position.DistanceSq(obj->GetPosition());
    }

    HEARTHSTONE_INLINE const float GetDistanceSq(float x, float y, float z)
    {
        return m_position.DistanceSq(x, y, z);
    }

    HEARTHSTONE_INLINE const float GetDistance2dSq( WorldObject* obj )
    {
        if( obj->GetMapId() != m_mapId )
            return 40000.0f; //enough for out of range
        return m_position.Distance2DSq( obj->m_position );
    }

    HEARTHSTONE_INLINE float GetDistance2dSq(float x, float y)
    {
        return m_position.Distance2DSq( x, y );
    }

    // In-range object management, not sure if we need it
    HEARTHSTONE_INLINE bool IsInRangeSet( WorldObject* pObj )
    {
        return m_objectsInRange.find(pObj) != m_objectsInRange.end();
    }

    virtual void AddInRangeObject(WorldObject* pObj)
    {
        if( pObj == NULL )
            return;

        m_objectsInRange.insert( pObj );

        if(pObj->IsGameObject())
            m_gameObjectsInRange.insert(castPtr<GameObject>(pObj));
        else
        {
            if(pObj->IsUnit()) m_unitsInRange.insert(castPtr<Unit>(pObj));
            if(pObj->IsPlayer()) m_inRangePlayers.insert(castPtr<Player>(pObj));
        }
    }

    HEARTHSTONE_INLINE bool HasInRangeObjects()
    {
        return ( m_objectsInRange.size() > 0 );
    }

    virtual void OnRemoveInRangeObject( WorldObject* pObj )
    {
        if(pObj->IsGameObject())
            m_gameObjectsInRange.erase(castPtr<GameObject>(pObj));
        else
        {
            if(pObj->IsUnit()) m_unitsInRange.erase(castPtr<Unit>(pObj));
            if(pObj->IsPlayer()) m_inRangePlayers.erase(castPtr<Player>(pObj));
        }
    }

    virtual void ClearInRangeSet()
    {
        m_objectsInRange.clear();
        m_unitsInRange.clear();
        m_inRangePlayers.clear();
        m_oppFactsInRange.clear();
        m_gameObjectsInRange.clear();
    }

    HEARTHSTONE_INLINE size_t GetInRangeCount() { return m_objectsInRange.size(); }
    HEARTHSTONE_INLINE size_t GetInRangeUnitsCount() { return m_unitsInRange.size();}
    HEARTHSTONE_INLINE size_t GetInRangePlayersCount() { return m_inRangePlayers.size();}
    HEARTHSTONE_INLINE InRangePlayerSet *GetInRangePlayerSet() { return &m_inRangePlayers; };

    InRangeSet::iterator FindInRangeSet(WorldObject * obj) { return m_objectsInRange.find(obj); }
    HEARTHSTONE_INLINE void RemoveInRangeObject(InRangeSet::iterator itr)
    {
        ASSERT(*itr);
        OnRemoveInRangeObject(*itr);
        m_objectsInRange.erase(itr);
    }

    HEARTHSTONE_INLINE bool RemoveInRangeObject( WorldObject* obj )
    {
        ASSERT(obj);
        InRangeSet::iterator itr = m_objectsInRange.find(obj);
        if( itr == m_objectsInRange.end() )
            return false;

        OnRemoveInRangeObject(*itr);
        m_objectsInRange.erase(itr);
        return true;
    }

    bool IsInRangeOppFactSet(Unit* pObj) { return (m_oppFactsInRange.count(pObj) > 0); }
    void UpdateOppFactionSet();

    HEARTHSTONE_INLINE InRangeSet::iterator GetInRangeSetBegin() { return m_objectsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeSet::iterator GetInRangeSetEnd() { return m_objectsInRange.end(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeOppFactsSetBegin() { return m_oppFactsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeOppFactsSetEnd() { return m_oppFactsInRange.end(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeUnitSetBegin() { return m_unitsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeUnitSetEnd() { return m_unitsInRange.end(); }
    HEARTHSTONE_INLINE InRangePlayerSet::iterator GetInRangePlayerSetBegin() { return m_inRangePlayers.begin(); }
    HEARTHSTONE_INLINE InRangePlayerSet::iterator GetInRangePlayerSetEnd() { return m_inRangePlayers.end(); }
    HEARTHSTONE_INLINE InRangeGameObjectSet::iterator GetInRangeGameObjectSetBegin() { return m_gameObjectsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeGameObjectSet::iterator GetInRangeGameObjectSetEnd() { return m_gameObjectsInRange.end(); }

    void __fastcall SendMessageToSet(WorldPacket *data, bool self,bool myteam_only=false);
    void OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self);

    int32 SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage = false, bool no_remove_auras = false, uint32 AdditionalCritChance = 0);

    //*****************************************************************************************
    //* SpellLog packets just to keep the code cleaner and better to read
    //*****************************************************************************************
    void SendSpellLog(WorldObject* Caster, WorldObject* Target,uint32 Ability, uint8 SpellLogType);
    void SendSpellNonMeleeDamageLog( WorldObject* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet );
    void SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate );

    //object faction
    void _setFaction();
    FactionTemplateEntry *GetFactionTemplate() { return m_factionTemplate; };
    uint32 GetFactionID() { return m_factionTemplate ? m_factionTemplate->Faction : 0; }
    FactionEntry *GetFaction() { return m_factionTemplate ? m_factionTemplate->GetFaction() : NULL; }

    HEARTHSTONE_INLINE void SetInstanceID(int32 instance) { m_instanceId = instance; }
    HEARTHSTONE_INLINE int32 GetInstanceID() { return m_instanceId; }

    int32 event_GetInstanceID();

    void DestroyForInrange(bool anim = false);
    WorldPacket *BuildTeleportAckMsg( const LocationVector & v);

    bool Active;
    bool CanActivate();
    void Activate(MapMgr* mgr);
    void Deactivate(MapMgr* mgr);
    HEARTHSTONE_INLINE void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

    HEARTHSTONE_INLINE size_t GetInRangeOppFactCount() { return m_oppFactsInRange.size(); }
    void PlaySoundToPlayer( Player* plr, uint32 sound_entry );
    void PlaySoundToSet(uint32 sound_entry);
    void EventSpellHit(Spell* pSpell);

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
    LocationVector m_position, m_lastMapUpdatePosition;

    //! Set of Objects in range.
    InRangeSet m_objectsInRange;

    // Inrange units
    InRangeUnitSet m_unitsInRange, m_oppFactsInRange;

    // Inrange players
    InRangePlayerSet m_inRangePlayers;

    // Inrange Gameobjects
    InRangeGameObjectSet m_gameObjectsInRange;

public:
    bool IsInLineOfSight(WorldObject* pObj);
    bool IsInLineOfSight(float x, float y, float z);
    int32 GetSpellBaseCost(SpellEntry *sp);
};
