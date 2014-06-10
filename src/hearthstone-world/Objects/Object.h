/***
 * Demonstrike Core
 */

#pragma once

class Unit;
class Spell;
class Aura;
class DynamicObject;

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
    HIGHGUID_TYPE_MO_TRANSPORT      = 0x1FC0,
    HIGHGUID_TYPE_GROUP             = 0x1F50,
    HIGHGUID_TYPE_GUILD             = 0x1FF6,
//===============================================
    HIGHGUID_TYPE_MASK              = 0xFFFF0000,
    LOWGUID_ENTRY_MASK              = 0x0000FFFF,
};

enum TypeMask
{
    TYPEMASK_OBJECT         = 0x000001,
    TYPEMASK_ITEM           = 0x000002,
    TYPEMASK_CONTAINER      = 0x000004,
    TYPEMASK_UNIT           = 0x000008,
    TYPEMASK_PLAYER         = 0x000010,
    TYPEMASK_GAMEOBJECT     = 0x000020,
    TYPEMASK_DYNAMICOBJECT  = 0x000040,
    TYPEMASK_CORPSE         = 0x000080,
    TYPEMASK_AIGROUP        = 0x000100,
    TYPEMASK_AREATRIGGER    = 0x000200,
    TYPEMASK_IN_GUILD       = 0x010000
};

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
    TYPEID_AIGROUP          = 8,
    TYPEID_AREATRIGGER      = 9,
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
    UPDATETYPE_CREATE_YOURSELF = 2,
    //  8 bytes - GUID
    //  1 byte - Object Type (*)
    //  Goto Position Update
    //  Goto Update Block
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 3
    //  4 bytes - Count
    //  Loop Count Times:
    //  8 bytes - GUID
};

enum OBJECT_UPDATE_FLAGS {
    UPDATEFLAG_NONE         = 0x0000,
    UPDATEFLAG_SELF         = 0x0001,
    UPDATEFLAG_TRANSPORT    = 0x0002,
    UPDATEFLAG_HAS_TARGET   = 0x0004,
    UPDATEFLAG_STA_MODEL    = 0x0008,
    UPDATEFLAG_DYN_MODEL    = 0x0010,
    UPDATEFLAG_LIVING       = 0x0020,
    UPDATEFLAG_HAS_POSITION = 0x0040,
    UPDATEFLAG_VEHICLE      = 0x0080,
    UPDATEFLAG_POSITION     = 0x0100,
    UPDATEFLAG_ROTATION     = 0x0200,
    UPDATEFLAG_UNK3         = 0x0400,
    UPDATEFLAG_ANIMKITS     = 0x0800,
    UPDATEFLAG_UNK5         = 0x1000,
    UPDATEFLAG_UNK6         = 0x2000
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

class WorldPacket;
class ByteBuffer;
class WorldSession;
class Player;
class MapCell;
class MapMgr;

//====================================================================
//  Object
//  Base object for every item, unit, player, corpse, container, etc
//====================================================================
class SERVER_DECL Object : public EventableObject
{
public:
    typedef unordered_set< Object* > InRangeSet;
    typedef unordered_set< Unit* >   InRangeUnitSet;
    typedef unordered_set< Player* > InRangePlayerSet;
    typedef std::map<string, void*> ExtensionSet;

    virtual ~Object ( );
    virtual void Init();
    virtual void Destruct();

    virtual void Update ( uint32 time ) { }
    //! True if object exists in world

    float GetCHeightForPosition(bool checkwater = false, float x = 0.0f, float y = 0.0f, float z = 0.0f);

    HEARTHSTONE_INLINE bool IsInWorld() { return m_mapMgr != NULL; }
    virtual void AddToWorld();
    virtual void AddToWorld(MapMgr* pMapMgr);
    void PushToWorld(MapMgr* );
    virtual void OnPushToWorld() { }
    virtual void OnPrePushToWorld() { }
    virtual void RemoveFromWorld(bool free_guid);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    HEARTHSTONE_INLINE void ObjLock() { m_objlock.Acquire(); }
    HEARTHSTONE_INLINE void ObjUnlock() { m_objlock.Release(); }

    // guid always comes first
    HEARTHSTONE_INLINE const uint64& GetGUID() const { return *((uint64*)m_uint32Values); }
    void SetGUID(uint64 GUID) { SetUInt64Value(OBJECT_FIELD_GUID, GUID);  }
    void SetLowGUID(uint32 val) { m_uint32Values[0] = val; }
    void SetHighGUID(uint32 val) { m_uint32Values[1] = val; }

    HEARTHSTONE_INLINE const WoWGuid& GetNewGUID() const { return m_wowGuid; }
    HEARTHSTONE_INLINE uint32 GetEntry(){return m_uint32Values[OBJECT_FIELD_ENTRY];}
    void SetEntry(uint32 value) { SetUInt32Value(OBJECT_FIELD_ENTRY, value); }

//  float GetScale() { return m_floatValues[ OBJECT_FIELD_SCALE_X ]; }
    void SetScale(float scale) { SetFloatValue(OBJECT_FIELD_SCALE_X, scale); };

    HEARTHSTONE_INLINE uint32 GetEntryFromGUID() const  { return GUID_ENPART(GetGUID()); }
    HEARTHSTONE_INLINE uint32 GetTypeFromGUID() const { return GUID_HIPART(GetGUID()); }
    HEARTHSTONE_INLINE uint32 GetUIdFromGUID() const { return GUID_LOPART(GetGUID()); }
    HEARTHSTONE_INLINE uint32 GetHighGUID() const { return GUID_HIPART(GetGUID()); }
    HEARTHSTONE_INLINE uint32 GetLowGUID() const { return GUID_LOPART(GetGUID()); }

    bool m_isTransport;
    bool m_isVehicle;
    bool m_isSummon;
    bool m_isTotem;
    bool m_isPet;

    // type
    HEARTHSTONE_INLINE const uint8& GetTypeId() const { return m_objectTypeId; }
    HEARTHSTONE_INLINE bool IsUnit()    { return ( m_objectTypeId == TYPEID_UNIT || m_objectTypeId == TYPEID_PLAYER ); }
    HEARTHSTONE_INLINE bool IsPlayer() { return m_objectTypeId == TYPEID_PLAYER; }
    HEARTHSTONE_INLINE bool IsCreature() { return m_objectTypeId == TYPEID_UNIT; }
    HEARTHSTONE_INLINE bool IsDynamicObj() { return m_objectTypeId == TYPEID_DYNAMICOBJECT; }
    HEARTHSTONE_INLINE bool IsGameObject() { return m_objectTypeId == TYPEID_GAMEOBJECT; }
    HEARTHSTONE_INLINE bool IsContainer()   { return m_objectTypeId == TYPEID_CONTAINER; }
    HEARTHSTONE_INLINE bool IsItem()    { return m_objectTypeId == TYPEID_ITEM; }
    HEARTHSTONE_INLINE bool IsTransport() { return m_isTransport; }
    HEARTHSTONE_INLINE bool IsVehicle() { return m_isVehicle; }
    HEARTHSTONE_INLINE bool IsSummon() { return m_isSummon; }
    HEARTHSTONE_INLINE bool IsTotem() { return m_isTotem; }
    HEARTHSTONE_INLINE bool IsPet() { return m_isPet; }

    //! This includes any nested objects we have, inventory for example.
    virtual uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer *buf, Player* target );
    uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer * buf, UpdateMask * mask );
    uint32 __fastcall BuildOutOfRangeUpdateBlock( ByteBuffer *buf );

    WorldPacket* BuildFieldUpdatePacket(uint32 index,uint32 value);
    void BuildFieldUpdatePacket(Player* Target, uint32 Index, uint32 Value);
    void BuildFieldUpdatePacket(ByteBuffer * buf, uint32 Index, uint32 Value);

    int32 DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

    void DestroyForInrange(bool anim = false);
    virtual void DestroyForPlayer( Player* target, bool anim = false );

    WorldPacket * BuildTeleportAckMsg( const LocationVector & v);

    void SetRotation( uint64 guid );

    void CastSpell(Object* Target, SpellEntry* Sp, bool triggered);
    void CastSpell(Object* Target, uint32 SpellID, bool triggered);
    void CastSpell(uint64 targetGuid, SpellEntry* Sp, bool triggered);
    void CastSpell(uint64 targetGuid, uint32 SpellID, bool triggered);

    HEARTHSTONE_INLINE const float& GetPositionX( ) const { return m_position.x; }
    HEARTHSTONE_INLINE const float& GetPositionY( ) const { return m_position.y; }
    HEARTHSTONE_INLINE const float& GetPositionZ( ) const { return m_position.z; }
    HEARTHSTONE_INLINE const float& GetOrientation( ) const { return m_position.o; }
    HEARTHSTONE_INLINE void SetOrientation( float &o ) { m_position.o = o; }

    HEARTHSTONE_INLINE const float& GetSpawnX( ) const { return m_spawnLocation.x; }
    HEARTHSTONE_INLINE const float& GetSpawnY( ) const { return m_spawnLocation.y; }
    HEARTHSTONE_INLINE const float& GetSpawnZ( ) const { return m_spawnLocation.z; }
    HEARTHSTONE_INLINE const float& GetSpawnO( ) const { return m_spawnLocation.o; }
    HEARTHSTONE_INLINE void SetSpawnX(float x) { m_spawnLocation.x = x; }
    HEARTHSTONE_INLINE void SetSpawnY(float y) { m_spawnLocation.y = y; }
    HEARTHSTONE_INLINE void SetSpawnZ(float z) { m_spawnLocation.z = z; }
    HEARTHSTONE_INLINE void SetSpawnO(float o) { m_spawnLocation.o = o; }

    bool canWalk();
    bool canSwim();
    bool canFly();

    HEARTHSTONE_INLINE const LocationVector & GetPosition() { return m_position; }
    HEARTHSTONE_INLINE LocationVector & GetPositionNC() { return m_position; }
    HEARTHSTONE_INLINE LocationVector * GetPositionV() { return &m_position; }
    HEARTHSTONE_INLINE void GetPosition(float &x, float &y, float &z) { x = m_position.x; y = m_position.y; z = m_position.z; }

    bool IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta);

    //Distance Calculation
    float CalcDistance(Object* Ob);
    float CalcDistance(float ObX, float ObY, float ObZ);
    float CalcDistance(Object* Oa, Object* Ob);
    float CalcDistance(Object* Oa, float ObX, float ObY, float ObZ);
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

    //! Get uint16 property
    HEARTHSTONE_INLINE const uint16& GetUInt16Value(uint16 index, uint8 offset) const { ASSERT( index < m_valuesCount ); ASSERT( offset < 2 ); return *(((uint16*)&m_uint32Values[index])+offset); }

    //! Get uint32 property
    HEARTHSTONE_INLINE const uint32& GetUInt32Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_uint32Values[ index ]; }

    //! Get uint64 property
    HEARTHSTONE_INLINE const uint64& GetUInt64Value( uint32 index ) const { ASSERT( index < m_valuesCount ); return *((uint64*)&(m_uint32Values[ index ])); }

    //! Get float property
    HEARTHSTONE_INLINE const float& GetFloatValue( uint32 index ) const { ASSERT( index < m_valuesCount ); return m_floatValues[ index ]; }

    void __fastcall ModFloatValue(const uint32 index, const float value );
    void ModSignedInt32Value(uint32 index, int32 value);
    void ModUnsigned32Value(uint32 index, int32 mod);
    uint32 GetModPUInt32Value(const uint32 index, const int32 value);

    //! Set uint32 property
    void SetByte(uint32 index, uint32 index1,uint8 value);

    HEARTHSTONE_INLINE uint8 GetByte(uint32 index, uint32 byteIndex) { return ((uint8*)m_uint32Values)[index*4+byteIndex]; }

    HEARTHSTONE_INLINE void SetNewGuid(uint32 Guid)
    {
        uint64 new_full_guid = Guid;
        if( m_objectTypeId == TYPEID_GAMEOBJECT )
            new_full_guid |= ((uint64)GetEntry() << 24) | ((uint64)HIGHGUID_TYPE_GAMEOBJECT << 32);

        if( m_objectTypeId == TYPEID_UNIT )
            new_full_guid |= ((uint64)GetEntry() << 24) | ((uint64)HIGHGUID_TYPE_CREATURE << 32);

        SetUInt64Value(OBJECT_FIELD_GUID, new_full_guid);
        m_wowGuid.Init(GetGUID());
    }

    void __fastcall SetUInt16Value(uint16 index, uint8 offset, uint16 value);

    void EventSetUInt32Value(uint32 index, uint32 value);
    void __fastcall SetUInt32Value( const uint32 index, const uint32 value );

    //! Set uint64 property
    void __fastcall SetUInt64Value( const uint32 index, const uint64 value );

    //! Set float property
    void __fastcall SetFloatValue( const uint32 index, const float value );

    void __fastcall SetFlag( const uint32 index, uint32 newFlag );

    void __fastcall RemoveFlag( const uint32 index, uint32 oldFlag );

    void __fastcall SetByteFlag( const uint32 index, const uint32 flag, uint8 newFlag);
    bool __fastcall HasByteFlag( const uint32 index, const uint32 flag, uint8 checkFlag);
    void __fastcall RemoveByteFlag(const uint32 index, const uint32 flag, uint8 checkFlag);

    HEARTHSTONE_INLINE bool HasFlag( const uint32 index, uint32 flag ) const { return (m_uint32Values[ index ] & flag) != 0;    }

    ////////////////////////////////////////
    void ClearUpdateMask( )
    {
        m_updateMask.Clear();
        m_objectUpdated = false;
    }

    void SetUpdateField(uint32 index);
    bool HasUpdateField(uint32 index) { return m_updateMask.GetBit(index); }

    //use it to check if a object is in range of another
    bool isInRange(Object* target, float range);

    // Use it to Check if a object is in front of another one
    bool isTargetInFront(Object* target);
    bool isTargetInBack(Object* target) { return !isTargetInFront(target); };

    bool isInFrontOfTarget(Object* target) { return target->isTargetInFront(this); };
    bool isInBackOfTarget(Object* target) { return target->isTargetInBack(this); };

    // Check to see if an object is in front of a target in a specified arc (in degrees)
    bool isInArc(Object* target , float degrees);

    /* Calculates the angle between two Positions */
    float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );
    float calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );

    /* converts to 360 > x > 0 */
    float getEasyAngle( float angle );

    HEARTHSTONE_INLINE float CalcDistance(LocationVector & comp)
    {
        return comp.Distance(m_position);
    }

    HEARTHSTONE_INLINE float GetDistanceSq(LocationVector & comp)
    {
        return comp.DistanceSq(m_position);
    }

    HEARTHSTONE_INLINE const float GetDistanceSq(Object* obj)
    {
        if(obj->GetMapId() != m_mapId)
            return 40000.0f; //enough for out of range
        return m_position.DistanceSq(obj->GetPosition());
    }

    HEARTHSTONE_INLINE const float GetDistanceSq(float x, float y, float z)
    {
        return m_position.DistanceSq(x, y, z);
    }

    HEARTHSTONE_INLINE const float GetDistance2dSq( Object* obj )
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
    HEARTHSTONE_INLINE bool IsInRangeSet( Object* pObj )
    {
        return !( m_objectsInRange.find( pObj ) == m_objectsInRange.end() );
    }

    virtual void AddInRangeObject(Object* pObj)
    {
        if( pObj == NULL )
            return;

        m_objectsInRange.insert( pObj );

        if( pObj->IsUnit() )
            m_unitsInRange.insert( TO_UNIT(pObj) );
        if( pObj->IsPlayer() )
            m_inRangePlayers.insert( TO_PLAYER(pObj) );
    }

    HEARTHSTONE_INLINE void RemoveInRangeObject( Object* pObj )
    {
        if( pObj == NULL )
            return;

        OnRemoveInRangeObject( pObj );
        m_objectsInRange.erase( pObj );
    }

    HEARTHSTONE_INLINE bool HasInRangeObjects()
    {
        return ( m_objectsInRange.size() > 0 );
    }

    virtual void OnRemoveInRangeObject( Object* pObj )
    {
        if( pObj->IsUnit() )
            m_unitsInRange.erase( TO_UNIT(pObj) );
        if( pObj->IsPlayer() )
            m_inRangePlayers.erase( TO_PLAYER(pObj) );
    }

    virtual void ClearInRangeSet()
    {
        m_objectsInRange.clear();
        m_inRangePlayers.clear();
        m_oppFactsInRange.clear();
    }

    HEARTHSTONE_INLINE size_t GetInRangeCount() { return m_objectsInRange.size(); }
    HEARTHSTONE_INLINE size_t GetInRangeUnitsCount() { return m_unitsInRange.size();}
    HEARTHSTONE_INLINE size_t GetInRangePlayersCount() { return m_inRangePlayers.size();}
    HEARTHSTONE_INLINE InRangePlayerSet *GetInRangePlayerSet() { return &m_inRangePlayers; };

    InRangeSet::iterator GetInRangeSetBegin() { return m_objectsInRange.begin(); }
    InRangeSet::iterator GetInRangeSetEnd() { return m_objectsInRange.end(); }
    InRangeSet::iterator FindInRangeSet(Object * obj) { return m_objectsInRange.find(obj); }

    void RemoveInRangeObject(InRangeSet::iterator itr)
    {
        OnRemoveInRangeObject(*itr);
        m_objectsInRange.erase(itr);
    }

    HEARTHSTONE_INLINE bool RemoveIfInRange( Object* obj )
    {
        InRangeSet::iterator itr = m_objectsInRange.find(obj);
        if( obj->IsPlayer() )
            m_inRangePlayers.erase( TO_PLAYER(obj) );

        if( itr == m_objectsInRange.end() )
            return false;

        m_objectsInRange.erase( itr );
        return true;
    }

    HEARTHSTONE_INLINE void AddInRangePlayer( Object* obj )
    {
        m_inRangePlayers.insert( TO_PLAYER(obj) );
    }

    HEARTHSTONE_INLINE void RemoveInRangePlayer( Object* obj )
    {
        m_inRangePlayers.erase( TO_PLAYER(obj) );
    }

    bool IsInRangeOppFactSet(Unit* pObj) { return (m_oppFactsInRange.count(pObj) > 0); }
    void UpdateOppFactionSet();

    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeOppFactsSetBegin() { return m_oppFactsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeOppFactsSetEnd() { return m_oppFactsInRange.end(); }
    HEARTHSTONE_INLINE InRangePlayerSet::iterator GetInRangePlayerSetBegin() { return m_inRangePlayers.begin(); }
    HEARTHSTONE_INLINE InRangePlayerSet::iterator GetInRangePlayerSetEnd() { return m_inRangePlayers.end(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeUnitSetBegin() { return m_unitsInRange.begin(); }
    HEARTHSTONE_INLINE InRangeUnitSet::iterator GetInRangeUnitSetEnd() { return m_unitsInRange.end(); }

    void __fastcall SendMessageToSet(WorldPacket *data, bool self,bool myteam_only=false);
    void OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self);

    //! Fill values with data from a space seperated string of uint32s.
    void LoadValues(const char* data);

    HEARTHSTONE_INLINE uint16 GetValuesCount() const { return m_valuesCount; }

    //! Blizzard seem to send those for all object types. weird.
    float m_walkSpeed;
    float m_runSpeed;
    float m_backWalkSpeed;
    float m_swimSpeed;
    float m_backSwimSpeed;
    float m_turnRate;
    float m_pitchRate;
    float m_flySpeed;
    float m_backFlySpeed;

    float m_base_runSpeed;
    float m_base_walkSpeed;

    int32 SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, bool allowProc, bool static_damage = false, bool no_remove_auras = false, uint32 AdditionalCritChance = 0);

    //*****************************************************************************************
    //* SpellLog packets just to keep the code cleaner and better to read
    //*****************************************************************************************
    void SendSpellLog(Object* Caster, Object* Target,uint32 Ability, uint8 SpellLogType);
    void SendSpellNonMeleeDamageLog( Object* Caster, Unit* Target, uint32 SpellID, uint32 Damage, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet );
    void SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate );

    //object faction
    void _setFaction();
    uint32 _getFaction() { return m_factionTemplate ? m_factionTemplate->Faction : 0; }
    uint32 GetFaction() { return _getFaction(); }

    FactionTemplateEntry *m_factionTemplate;
    FactionEntry *m_faction;

    uint32 VendorMask;

    HEARTHSTONE_INLINE void SetInstanceID(int32 instance) { m_instanceId = instance; }
    HEARTHSTONE_INLINE int32 GetInstanceID() { return m_instanceId; }

    int32 event_GetInstanceID();

    bool Active;
    bool CanActivate();
    void Activate(MapMgr* mgr);
    void Deactivate(MapMgr* mgr);
    bool m_inQueue;
    HEARTHSTONE_INLINE void SetMapMgr(MapMgr* mgr) { m_mapMgr = mgr; }

    HEARTHSTONE_INLINE size_t GetInRangeOppFactCount() { return m_oppFactsInRange.size(); }
    void PlaySoundToPlayer( Player* plr, uint32 sound_entry );
    void PlaySoundToSet(uint32 sound_entry);
    void EventSpellHit(Spell* pSpell);

    bool PhasedCanInteract(Object* pObj);
    int32 GetPhaseMask() { return m_phaseMask; }
    bool IsInPhase(int32 phaseMode) { if(AllPhases) return true; return ((m_phaseMask & phaseMode) != 0); };
    void EnablePhase(int32 phaseMode);
    void DisablePhase(int32 phaseMode);
    void SendPhaseShift();

    // Don't fucking use this.
    void SetPhaseMask(int32 phase);

    Aura* m_phaseAura;
    bool OwnPhase; // Players only really.

    // Area flags
    bool HasAreaFlag(uint8 areaFlag) { return (m_areaFlags & areaFlag); };
    uint8 const GetAreaFlags() { return m_areaFlags; };

protected:
    Object (  );

    //void _Create (uint32 guidlow, uint32 guidhigh);
    void _Create( uint32 mapid, float x, float y, float z, float ang);

    //! Mark values that need updating for specified player.
    virtual void _SetUpdateBits(UpdateMask *updateMask, Player* target) const;
    //! Mark values that player should get when he/she/it sees object for first time.
    virtual void _SetCreateBits(UpdateMask *updateMask, Player* target) const;

    void _BuildMovementUpdate( ByteBuffer *data, uint16 flags, Player* target );
    void _BuildValuesUpdate( ByteBuffer *data, UpdateMask *updateMask, Player* target );

    /* Main Function called by isInFront(); */
    bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y );

    bool AllPhases;
    int32 m_phaseMask;
    LocationVector m_phaseLocation;
    uint32 m_phaseDistanceLimit;

    Mutex m_objlock;

    //! WoWGuid class
    WoWGuid m_wowGuid;

    //! Type id.
    uint8 m_objectTypeId;
    //! Zone id.
    uint32 m_zoneId;
    //! Area id.
    uint32 m_areaId;
    //! Area Flags.
    uint8 m_areaFlags;
    //! Continent/map id.
    int32 m_mapId;
    //! Map manager
    MapMgr* m_mapMgr;
    //! Current map cell
    MapCell *m_mapCell;

    LocationVector m_position;
    LocationVector m_lastMapUpdatePosition;
    LocationVector m_spawnLocation;

    // Semaphores - needed to forbid two operations on the same object at the same very time (may cause crashing\lack of data)
    bool mSemaphoreTeleport;

    //! Object properties.
    union {
        uint32* m_uint32Values;
        float* m_floatValues;
    };

    //! Number of properties
    uint32 m_valuesCount;

    //! List of object properties that need updating.
    UpdateMask m_updateMask;

    //! True if object was updated
    bool m_objectUpdated;

    //! Set of Objects in range.
    //! TODO: that functionality should be moved into WorldServer.
    unordered_set<Object* > m_objectsInRange;
    unordered_set<Player* > m_inRangePlayers;
    unordered_set<Unit* > m_oppFactsInRange;
    unordered_set<Unit* > m_unitsInRange;

    int32 m_instanceId;

    ExtensionSet * m_extensions;
    void _SetExtension(const string& name, void* ptr);      // so we can set from scripts. :)

public:

    template<typename T>
        void SetExtension(const string& name, T ptr)
    {
        _SetExtension(name, ((void*)ptr));
    }

    template<typename T>
        T GetExtension(const string& name)
    {
        if( m_extensions == NULL )
            return ((T)NULL);
        else
        {
            ExtensionSet::iterator itr = m_extensions->find( name );
            if( itr == m_extensions->end() )
                return ((T)NULL);
            else
                return ((T)itr->second);
        }
    }

    bool m_loadedFromDB;

    /************************************************************************/
    /* ACCESSOR FUNCTIONS                                                   */
    /************************************************************************/
    // Stats
    HEARTHSTONE_INLINE uint32 GetStrength() { return m_uint32Values[UNIT_FIELD_STRENGTH]; }
    HEARTHSTONE_INLINE uint32 GetAgility() { return m_uint32Values[UNIT_FIELD_AGILITY]; }
    HEARTHSTONE_INLINE uint32 GetStamina() { return m_uint32Values[UNIT_FIELD_STAMINA]; }
    HEARTHSTONE_INLINE uint32 GetIntellect() { return m_uint32Values[UNIT_FIELD_INTELLECT]; }
    HEARTHSTONE_INLINE uint32 GetSpirit() { return m_uint32Values[UNIT_FIELD_SPIRIT]; }

    // Health
    HEARTHSTONE_INLINE uint32 GetHealth() { return m_uint32Values[UNIT_FIELD_HEALTH]; }
    HEARTHSTONE_INLINE uint32 GetMaxHealth() { return m_uint32Values[UNIT_FIELD_MAXHEALTH]; }
    HEARTHSTONE_INLINE uint32 GetMana() { return m_uint32Values[UNIT_FIELD_POWER1]; }
    HEARTHSTONE_INLINE uint32 GetMaxMana() { return m_uint32Values[UNIT_FIELD_MAXPOWER1]; }

    bool IsInLineOfSight(Object* pObj);
    bool IsInLineOfSight(float x, float y, float z);
    int32 GetSpellBaseCost(SpellEntry *sp);

    /************************************************************************/
    /* END ACCESSOR FUNCTIONS                                               */
    /************************************************************************/

    // declaration to fix scripting
    HEARTHSTONE_INLINE Loot* GetLoot() { return &m_loot; }

public:
    // loooooot
    Loot m_loot;
    bool m_looted;

    // empties loot vector
    void ClearLoot();
};
