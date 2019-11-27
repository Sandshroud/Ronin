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
    TYPEMASK_TYPE_DEACTIVATED   = 0x00008000, // internal
    TYPEMASK_TYPE_MASK          = 0x0000FFFF,
    // Upper 16 bits are flag
    TYPEMASK_FLAG_IN_GUILD      = 0x00010000
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

enum ObjectAreaFlags : uint16
{
    OBJECT_AREA_FLAG_NONE               = 0x0000,
    OBJECT_AREA_FLAG_INDOORS            = 0x0001,
    OBJECT_AREA_FLAG_INCITY             = 0x0002,
    OBJECT_AREA_FLAG_INSANCTUARY        = 0x0004,
    OBJECT_AREA_FLAG_PVP_AREA           = 0x0008,

    OBJECT_AREA_FLAG_CONTESTED          = 0x0010,
    OBJECT_AREA_FLAG_ALLIANCE_ZONE      = 0x0020,
    OBJECT_AREA_FLAG_HORDE_ZONE         = 0x0040,
    OBJECT_AREA_FLAG_ARENA_ZONE         = 0x0080,

    OBJECT_AREA_FLAG_UNDERWATER_AREA    = 0x0100,

    OBJECT_AREA_FLAG_IGNORE_ADT_WATER   = 0x1000,
    OBJECT_AREA_FLAG_USE_WMO_WATER      = 0x2000,
};

enum ObjectInactiveFlags
{
    OBJECT_INACTIVE_FLAG_INACTIVE   = 0x01,
    OBJECT_INACTIVE_FLAG_DESPAWNED  = 0x02,
    OBJECT_INACTIVE_FLAG_CONDITION  = 0x04,
    OBJECT_INACTIVE_FLAG_EVENTS     = 0x08,
    OBJECT_ACTIVE_FLAG_MASK         = 0x0F,
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
class MapInstance;
class ObjectCellManager;

//===============================================
//  Object
//  Base class for every object
//===============================================
class SERVER_DECL Object
{
protected:
    friend class MapInstance;

    Object();
    void Construct(WoWGuid guid, uint32 fieldCount = OBJECT_END);

public:
    virtual ~Object();
    virtual void Init();
    virtual void Destruct();

    virtual void Update (uint32 msTime, uint32 diff);

    // Value fields
    void __fastcall SetByte(uint16 index, uint8 flag, uint8 value);
    void __fastcall SetByteFlag( const uint16 index, const uint8 flag, uint8 newFlag);
    bool __fastcall HasByteFlag( const uint16 index, const uint8 flag, uint8 checkFlag);
    void __fastcall RemoveByteFlag(const uint16 index, const uint8 flag, uint8 checkFlag);

    void __fastcall SetUInt16Value(uint16 index, uint8 offset, uint16 value);
    void __fastcall SetUInt32Value( const uint16 index, const uint32 value );
    void __fastcall SetUInt64Value( const uint16 index, const uint64 value );
    void __fastcall SetFlag( const uint16 index, uint32 newFlag );
    void __fastcall RemoveFlag( const uint16 index, uint32 oldFlag );
    void __fastcall SetFloatValue( const uint16 index, const float value );
    bool __fastcall HasFlag( const uint16 index, uint32 flag ) const { return (m_uint32Values[ index ] & flag) != 0;    }

    void __fastcall ModFloatValue(const uint16 index, const float value );
    void __fastcall ModSignedInt32Value(uint16 index, int32 value);
    void __fastcall ModUnsigned32Value(uint16 index, int32 mod);
    uint32 __fastcall GetModPUInt32Value(const uint16 index, const int32 value);

    RONIN_INLINE uint8 GetByte(uint16 index, uint8 byteIndex) { return ((uint8*)m_uint32Values)[index*4+byteIndex]; }
    RONIN_INLINE const uint16& GetUInt16Value(uint16 index, uint8 offset) const { ASSERT( index < m_valuesCount ); ASSERT( offset < 2 ); return *(((uint16*)&m_uint32Values[index])+offset); }
    RONIN_INLINE const uint32& GetUInt32Value( uint16 index ) const { ASSERT( index < m_valuesCount ); return m_uint32Values[ index ]; }
    RONIN_INLINE const uint64& GetUInt64Value( uint16 index ) const { ASSERT( index < m_valuesCount ); return *((uint64*)&(m_uint32Values[ index ])); }
    RONIN_INLINE const float& GetFloatValue( uint16 index ) const { ASSERT( index < m_valuesCount ); return m_floatValues[ index ]; }
    RONIN_INLINE const float GetUInt32FloatValue(uint16 index) const { ASSERT( index < m_valuesCount ); return (float)m_uint32Values[index]; }

    // Update masks
    void SetUpdateField(uint16 index);
    bool HasUpdateField(uint16 index) { return m_updateMask.GetBit(index); }
    virtual void OnFieldUpdated(uint16 index) {}

    //! Mark values that need updating for specified player.
    bool _SetUpdateBits(UpdateMask *updateMask, uint32 updateFlags);

    uint16 GetUpdateFlag(Player *target);
    void GetUpdateFieldData(uint8 type, uint16 *&flags, uint16 &length);

    //! This includes any nested objects we have, inventory for example.
    virtual uint32 __fastcall BuildCreateUpdateBlockForPlayer( ByteBuffer *data, Player* target );
    uint32 __fastcall BuildValuesUpdateBlockForPlayer( ByteBuffer *buf, Player* target, uint32 updateFlags, uint32 expectedField = 0);

    // Data field updates
    virtual void OnUpdateProcess();

private:
    void _BuildCreateValuesUpdate( ByteBuffer *data, Player* target );
    void _BuildChangedValuesUpdate( ByteBuffer *data, Player* target, UpdateMask *updateMask );
    uint32 _GetSwappedValueForUpdate(uint32 updateField, uint16 fieldFlag, Player *target);

    void _BuildMovementUpdate( ByteBuffer *data, uint16 flags, Player* target );

public:
    virtual void DestroyForPlayer( Player* target, bool anim = false );

    ////////////////////////////////////////
    void ClearUpdateMask() { m_updateMask.Clear(); }

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
    RONIN_INLINE uint8 GetTypeId() { return m_objType; }
    RONIN_INLINE bool IsUnit() { return IsCreature() || IsPlayer(); }
    virtual bool IsItem() { return false; }
    virtual bool IsCreature() { return false; }
    virtual bool IsPlayer() { return false; }
    virtual bool IsGameObject() { return false; }
    virtual bool IsDynamicObj() { return false; }
    virtual bool IsCorpse() { return false; }
    virtual bool IsContainer() { return false; }
    virtual bool IsObject() { return false; }
    virtual bool IsTotem() { return false; }
    virtual bool IsSummon() { return false; }
    virtual bool IsVehicle() { return false; }
    virtual bool IsTransport() { return false; }
    virtual bool IsSummonerPlayer() { return false; }
    virtual bool IsMapCellInitializer() { return false; }

    template<typename T> RONIN_INLINE bool IsType() { return false; }
    template<> RONIN_INLINE bool IsType<Unit>() { return IsUnit(); }
    template<> RONIN_INLINE bool IsType<Player>() { return IsPlayer(); }
    template<> RONIN_INLINE bool IsType<Creature>() { return IsCreature(); }
    template<> RONIN_INLINE bool IsType<GameObject>() { return IsGameObject(); }

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
    //! Object's type
    uint8 m_objType;
    //! Flags for building update data
    uint32 m_updateFlags;
    //! Number of properties
    uint16 m_valuesCount;
    //! Notification flags for updates
    uint16 m_notifyFlags;
    //! List of object properties that need updating.
    UpdateMask m_updateMask, m_dynamicFields;
    //! True if object is in world
    bool m_inWorld;
    //! Event handler for processing delayed functions
    EventHandler m_eventHandler;

public:
    RONIN_INLINE void LootLock() { m_loot._lock.Acquire(); }
    RONIN_INLINE void LootUnlock() { m_loot._lock.Release(); }

    RONIN_INLINE ObjectLoot* GetLoot() { return &m_loot; }
    RONIN_INLINE bool IsLooted() { return m_looted; }
    RONIN_INLINE void SetLooted() { m_looted = true; }

    // fills loot vector
    RONIN_INLINE bool IsLootGenerated() { return m_lootGenerated; }
    virtual void GenerateLoot() { m_lootGenerated = true; }
    // empties loot vector
    void ClearLoot();

private:
    // loooooot
    ObjectLoot m_loot;
    bool m_lootGenerated, m_looted;
};

//===============================================
//===============================================
//===============================================
class ObjectCellManager
{
public:
    ObjectCellManager(WorldObject *obj) : _object(obj), _currX(0xFFFF), _currY(0xFFFF), _lowX(0xFFFF), _lowY(0xFFFF), _highX(0xFFFF), _highY(0xFFFF) {}
    ~ObjectCellManager() {}

    // Settings for our cell manager, set in object.cpp
    static unsigned int VisibleCellRange;
    static bool cutCorners;

    virtual void Update(MapInstance *instance, uint32 msTime, uint32 uiDiff);
    virtual void SetCurrentCell(MapInstance *instance, float newX, float newY, float newZ, uint8 cellRange);

    virtual void OnRelocate(MapInstance *instance, LocationVector &destination);
    virtual void ClearInRangeObjects(MapInstance *instance);

    virtual void OnUnitDeath(MapInstance *instance);
    virtual void PostRemoveFromWorld();

    virtual void UpdateVisibility(MapInstance *instance);

    bool hasCell(uint32 cellId);
    void FillCellRange(std::vector<uint32> *fillVector);
    void CreateCellRange(std::vector<uint32> *fillVector, float range);
    void CreateCellRange(std::vector<uint32> *fillVector, uint32 range);

    bool IsLocationChanged(float x, float y, float z);
    static void ConstructCellData(float x, float y, float range, std::vector<uint32> *fillVector);

    void AddVisibleBy(WoWGuid guid) { m_visibleTo.insert(guid); }
    void RemoveVisibleBy(WoWGuid guid) { m_visibleTo.erase(guid); }

    std::set<WoWGuid>::iterator beginVisible() { return m_visibleTo.begin(); }
    std::set<WoWGuid>::iterator endVisible() { return m_visibleTo.end(); }

protected:
    friend class MapInstance;
    static uint32 _makeCell(uint16 x, uint16 y) { return (((uint32)x)<<16) | ((uint32)y); }
    static std::pair<uint16, uint16> unPack(uint32 cellId) { return std::make_pair(((uint16)(cellId>>16)), ((uint16)(cellId & 0x0000FFFF))); }
    static uint32 _getCellId(float pos);

    RONIN_INLINE bool isCorner(uint16 x, uint16 y, uint16 lX, uint16 hX, uint16 lY, uint16 hY, uint16 visRange = 0)
    {
        uint16 lowX = _lowX, highX = _highX, lowY = _lowY, highY = _highY;
        if(visRange > 1)
        {
            // We can just add the range extension here
            lowX += visRange-1;
            lowY += visRange-1;
            // We need to make sure we don't go negative
            highX -= std::min<uint16>(highX, visRange-1);
            highY -= std::min<uint16>(highY, visRange-1);
        }

        if((x < lowX && y < lowY)
            || (x < lowX && y > highY)
            || (x > highX && y < lowY)
            || (x > highX && y > highY))
            return true;
        return false;
    }

    uint16 _visRange;
    uint16 _currX, _currY, _lowX, _lowY, _highX, _highY;

    WorldObject *_object;
    float _luX, _luY, _luZ;

    // Players that can see us
    std::set<WoWGuid> m_visibleTo;
};

//===============================================
//===============================================
//===============================================
class SERVER_DECL WorldObject : public Object
{
protected:
    friend class MapInstance;

    // DO NOT CALL DIRECTLY, USE MAP INSTANCE ALLOCATION SYSTEM
    WorldObject();
    void Construct(WoWGuid guid, uint32 fieldCount = OBJECT_END);

public:
    virtual ~WorldObject( );

    virtual void Init();
    virtual void Destruct();

    void Cleanup();

    virtual void Update(uint32 msTime, uint32 uiDiff);
    virtual void InactiveUpdate(uint32 msTime, uint32 uiDiff);

    virtual bool IsObject() { return true; }
    virtual bool IsActiveObject() { return false; }

    //! True if object exists in world
    virtual bool IsInWorld() { return m_mapInstance != NULL; }
    virtual void RemoveFromWorld();
    virtual void EventExploration(MapInstance *instance);

    virtual bool CanReactivate() { return true; }
    virtual void Reactivate() = 0;
    virtual void Deactivate(uint32 reactivationTime);

    bool IsActivated() { return (m_inactiveFlags & OBJECT_INACTIVE_FLAG_INACTIVE) == 0; }
    bool hasInactiveFlag(uint16 flag) { return m_inactiveFlags & flag; }

    virtual float GetModelHalfZSize() { return 0.5f; }
    virtual float getViewDistanceMod() { return 1.f; }
    virtual uint32 getEventID() { return 0; }
    virtual uint32 getConditionID() { return 0; }

    RONIN_INLINE void SetMapInstance(MapInstance* instance) { m_mapInstance = instance; }

    void PushToWorld(MapInstance* instance, uint32 msTime);

    virtual void OnPreSetInWorld() { }
    virtual void OnPrePushToWorld() { }
    virtual void OnPushToWorld(uint32 msTime) { }

    virtual void OnFieldUpdated(uint16 index);

    virtual void SetPosition( float newX, float newY, float newZ, float newOrientation );
    virtual void SetPosition( const LocationVector & v) { SetPosition(v.x, v.y, v.z, v.o); }

    int32 DealDamage(Unit* pVictim, uint32 damage, uint32 targetEvent, uint32 unitEvent, uint32 spellId, bool no_remove_auras = false);

    // Field values
    virtual uint32 getLevel() = 0;

    RONIN_INLINE const float& GetPositionX( ) const { return m_position.x; }
    RONIN_INLINE const float& GetPositionY( ) const { return m_position.y; }
    RONIN_INLINE const float& GetPositionZ( ) const { return m_position.z; }
    RONIN_INLINE const float& GetOrientation( ) const { return m_position.o; }
    RONIN_INLINE void SetOrientation( float o ) { m_position.o = NormAngle(o); }

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

    RONIN_INLINE SmartBounding *GetBoundBox() { return &m_boundBox; }

    bool IsInBox(float centerX, float centerY, float centerZ, float BLength, float BWidth, float BHeight, float BOrientation, float delta);

    RONIN_INLINE MapCell* GetMapCell() const { return m_mapCell; }
    RONIN_INLINE void SetMapCell(MapCell* cell) { m_mapCell = cell; }
    RONIN_INLINE MapInstance* GetMapInstance() const { return m_mapInstance; }

    RONIN_INLINE void SetMapId(uint32 newMap) { m_mapId = newMap; }
    RONIN_INLINE const uint32 GetMapId( ) const { return m_mapId; }

    virtual void UpdateAreaInfo(MapInstance *instance = NULL);
    RONIN_INLINE const uint32& GetWMOId( ) const { return m_wmoId; }
    RONIN_INLINE const uint32& GetAreaId( ) const { return m_areaId; }
    RONIN_INLINE const uint32& GetZoneId( ) const { return m_zoneId; }
    RONIN_INLINE void SetLastMovementZone(uint32 zone) { m_lastMovementZone = zone; }
    RONIN_INLINE uint32 GetLastMovementZone() { return m_lastMovementZone; }
    RONIN_INLINE void SetLastMovementArea(uint32 area) { m_lastMovementArea = area; }
    RONIN_INLINE uint32 GetLastMovementArea() { return m_lastMovementArea; }

    //use it to check if a object is in range of another
    bool isInRange(WorldObject* target, float range);
    bool isInRange(float x, float y, float z, float range);

    // Use it to Check if a object is in front of another one
    bool isTargetInFront(WorldObject* target);
    bool isTargetInBack(WorldObject* target) { return !isTargetInFront(target); };

    bool isInFrontOfTarget(WorldObject* target) { return target->isTargetInFront(this); };
    bool isInBackOfTarget(WorldObject* target) { return target->isTargetInBack(this); };

    // Check to see if an object is in front of a target in a specified arc (in degrees)
    bool isInArc(WorldObject* target , float degrees);

    /* Calculates the angle between two Positions */
    static float calcAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );
    static float calcRadAngle( float Position1X, float Position1Y, float Position2X, float Position2Y );

    /* converts to 360 > x > 0 */
    float getEasyAngle( float angle );

    float GetAngle(WorldObject *obj);
    static float GetAngle(float x, float y, float tX, float tY);
    static float CalcDistanceSq(float x1, float y1, float z1, float x2, float y2, float z2);
    RONIN_INLINE float GetDistanceSq(LocationVector & comp) { return comp.DistanceSq(m_position); }
    RONIN_INLINE const float GetDistanceSq(WorldObject* obj)
    {
        if(obj->GetMapId() != m_mapId)
            return 40000.0f; //enough for out of range
        return m_position.DistanceSq(obj->GetPosition());
    }

    RONIN_INLINE const float GetDistance2dSq( WorldObject* obj )
    {
        if( obj->GetMapId() != m_mapId )
            return 40000.0f; //enough for out of range
        return m_position.Distance2DSq( obj->m_position );
    }

    RONIN_INLINE const float GetDistanceSq(float x, float y, float z) { return m_position.DistanceSq(x, y, z); }
    RONIN_INLINE float GetDistance2dSq(float x, float y) { return m_position.Distance2DSq( x, y ); }

    WorldObject *GetInRangeObject(WoWGuid guid);
    template<typename T> T *GetInRangeObject(WoWGuid guid)
    {
        WorldObject *obj = GetInRangeObject(guid);
        if(obj == NULL || !obj->IsType<T>())
            return NULL;
        return castPtr<T>(obj);
    }

    ObjectCellManager *GetCellManager() { return m_cellManager; }

    void __fastcall SendMessageToSet(WorldPacket *data, bool self, bool myteam_only = false, float maxRange = -1.f);
    void OutPacketToSet(uint16 Opcode, uint16 Len, const void * Data, bool self, float maxRange=-1.f);

    void SpellNonMeleeDamageLog(Unit* pVictim, uint32 spellID, uint32 damage, float resistPct);

    //*****************************************************************************************
    //* SpellLog packets just to keep the code cleaner and better to read
    //*****************************************************************************************
    void SendSpellLog(WorldObject* Caster, WorldObject* Target,uint32 Ability, uint8 SpellLogType);
    void SendSpellNonMeleeDamageLog( WorldObject* Caster, Unit* Target, uint32 SpellID, uint32 damageDone, uint8 School, uint32 AbsorbedDamage, uint32 ResistedDamage, bool PhysicalDamage, uint32 BlockedDamage, bool CriticalHit, bool bToSet );
    void SendAttackerStateUpdate( Unit* Target, dealdamage *dmg, uint32 realdamage, uint32 abs, uint32 blocked_damage, uint32 hit_status, uint32 vstate );

    //object faction
    void SetFactionTemplate(uint32 templateId);
    FactionTemplateEntry *GetFactionTemplate() { return m_factionTemplate; };
    uint32 GetFactionID() { return m_factionTemplate ? m_factionTemplate->Faction : 0; }
    FactionEntry *GetFaction() { return m_factionTemplate ? m_factionTemplate->GetFaction() : NULL; }

    RONIN_INLINE void SetInstanceID(int32 instance) { m_instanceId = instance; }
    RONIN_INLINE int32 GetInstanceID() { return m_instanceId; }

    WorldPacket *BuildTeleportAckMsg( const LocationVector & v);

    void PlaySoundToPlayer( Player* plr, uint32 sound_entry );
    void PlaySoundToSet(uint32 sound_entry);
    void EventSpellHit(Spell* pSpell);

    uint16 GetPhaseMask() { return m_phaseMask; }
    void BuildPhaseSet(std::vector<uint16> *phaseSet);

    // Area flags
    bool HasAreaFlag(uint16 areaFlag) { return (m_areaFlags & areaFlag); }
    uint16 const GetAreaFlags() { return m_areaFlags; }

    float const GetGroundHeight() { return m_groundHeight; }

    uint16 const GetLiqFlags() { return m_liquidFlags; }
    float const GetLiqHeight() { return m_liquidHeight; }

    void MarkForCleanup() { m_isMarkedForCleanup = true; }
    bool IsBulkSpawn() { return m_isManagedBulkSpawn; }
    void MarkNonBulk() { m_isManagedBulkSpawn = false; }

protected:
    void _Create( uint32 mapid, float x, float y, float z, float ang);

    /* Main Function called by isInFront(); */
    bool inArc(float Position1X, float Position1Y, float FOV, float Orientation, float Position2X, float Position2Y );

    //! WMO id.
    uint32 m_wmoId;
    //! Zone id.
    uint32 m_zoneId;
    //! Area id.
    uint32 m_areaId;
    //! Phase mask.
    uint16 m_phaseMask;
    //! Area Flags.
    uint16 m_areaFlags;
    //! Ground Height.
    float m_groundHeight;
    //! Liquid Flags.
    uint16 m_liquidFlags;
    //! Liquid Height.
    float m_liquidHeight;
    //! Object deactivation
    uint16 m_inactiveFlags;
    //! Continent/map id.
    int32 m_mapId;
    //! Instance Id
    int32 m_instanceId;
    //! Last set Movement zone
    uint32 m_lastMovementZone;
    uint32 m_lastMovementArea;
    uint32 m_objDeactivationTimer;

    //! Map manager
    MapInstance* m_mapInstance;
    //! Current map cell
    MapCell *m_mapCell;
    //! Current object faction
    FactionTemplateEntry *m_factionTemplate;
    // Current map location
    LocationVector m_position, m_spawnLocation;
    //! Bound box implementation for inrange checks/location offset
    SmartBounding m_boundBox;
    //! Inrange cell management
    ObjectCellManager *m_cellManager;
    //! Marked for mapinstance cleanup
    bool m_isMarkedForCleanup;
    //! Marked as mass allocated object
    bool m_isManagedBulkSpawn;

public:
    bool IsInLineOfSight(WorldObject* pObj);
    bool IsInLineOfSight(float x, float y, float z);
    int32 GetSpellBaseCost(SpellEntry *sp);
};
