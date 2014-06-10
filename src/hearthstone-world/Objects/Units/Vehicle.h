/***
 * Demonstrike Core
 */

#pragma once

enum VehicleFlags
{
    VEHICLE_FLAG_NO_STRAFE          = 0x0001, // Sets MOVEFLAG2_NO_STRAFE
    VEHICLE_FLAG_NO_JUMPING         = 0x0002, // Sets MOVEFLAG2_NO_JUMPING
    VEHICLE_FLAG_FULLSPEEDTURNING   = 0x0004, // Sets MOVEFLAG2_FULLSPEEDTURNING
    VEHICLE_FLAG_ALLOW_PITCHING     = 0x0010, // Sets MOVEFLAG2_ALLOW_PITCHING
    VEHICLE_FLAG_FULLSPEEDPITCHING  = 0x0020, // Sets MOVEFLAG2_FULLSPEEDPITCHING
    VEHICLE_FLAG_CUSTOM_PITCH       = 0x0040, // If set use pitchMin and pitchMax from DBC, otherwise pitchMin = -pi/2, pitchMax = pi/2
    VEHICLE_FLAG_ADJUST_AIM_ANGLE   = 0x0400, // Lua_IsVehicleAimAngleAdjustable
    VEHICLE_FLAG_ADJUST_AIM_POWER   = 0x0800, // Lua_IsVehicleAimPowerAdjustable
};

enum VehicleSeatFlags
{
    VEHICLE_SEAT_FLAG_HIDE_PASSENGER    = 0x00000200, // Passenger is hidden
    VEHICLE_SEAT_FLAG_UNK11             = 0x00000400, // needed for CGCamera__SyncFreeLookFacing
    VEHICLE_SEAT_FLAG_CAN_CONTROL       = 0x00000800, // Lua_UnitInVehicleControlSeat
    VEHICLE_SEAT_FLAG_CAN_ATTACK        = 0x00004000, // Can attack, cast spells and use items from vehicle?
    VEHICLE_SEAT_FLAG_CAN_ENTER_OR_EXIT = 0x02000000, // Lua_CanExitVehicle - can enter and exit at free will
    VEHICLE_SEAT_FLAG_CAN_SWITCH        = 0x04000000, // Lua_CanSwitchVehicleSeats
    VEHICLE_SEAT_FLAG_CAN_CAST          = 0x20000000, // Lua_UnitHasVehicleUI
};

enum VehicleSeatFlagsB
{
    VEHICLE_SEAT_FLAG_B_NONE                    = 0x00000000,
    VEHICLE_SEAT_FLAG_B_USABLE_FORCED           = 0x00000002,
    VEHICLE_SEAT_FLAG_B_TARGETS_IN_RAIDUI       = 0x00000008, // Lua_UnitTargetsVehicleInRaidUI
    VEHICLE_SEAT_FLAG_B_EJECTABLE               = 0x00000020, // ejectable
    VEHICLE_SEAT_FLAG_B_USABLE_FORCED_2         = 0x00000040,
    VEHICLE_SEAT_FLAG_B_USABLE_FORCED_3         = 0x00000100,
    VEHICLE_SEAT_FLAG_B_CANSWITCH               = 0x04000000, // can switch seats
    VEHICLE_SEAT_FLAG_B_VEHICLE_PLAYERFRAME_UI  = 0x80000000, // Lua_UnitHasVehiclePlayerFrameUI - actually checked for flagsb &~ 0x80000000
};

class SERVER_DECL Vehicle : public Creature
{
public:
    Vehicle(uint64 guid);
    ~Vehicle();
    virtual void Init();
    virtual void Destruct();

    void InitSeats(uint32 vehicleEntry, Player* pRider = NULLPLR);
    virtual void Update(uint32 time);
    bool Load(CreatureSpawn *spawn, uint32 mode, MapInfo *info);
    void Load(CreatureProto * proto_, uint32 mode, float x, float y, float z, float o = 0.0f);
    void OnPushToWorld();
    void Despawn(uint32 delay, uint32 respawntime);
    void DeleteMe();
    void SafeDelete();
    void AddPassenger(Unit* pPassenger, int8 requestedseat = -1, bool force = false);
    void RemovePassenger(Unit* pPassenger);
    bool HasPassenger(Unit* pPassenger);
    void SendSpells(uint32 entry, Player* plr);
    void VehicleSetDeathState(DeathState s);
    void ChangeSeats(Unit* pPassenger, uint8 seatid);
    void ChangePowerType();
    uint16 GetAddMovement2Flags();
    uint32 GetVehiclePowerType() { return (vehicleData ? vehicleData->m_powerType : 0); };

    //---------------------------------------
    // Accessors
    //---------------------------------------
    uint32 GetMaxPassengerCount() { return m_maxPassengers; }
    uint32 GetPassengerCount() { return m_ppassengerCount; }

    int8 GetMaxSeat() { return m_seatSlotMax; }
    Unit* GetPassenger(uint8 seat)
    {
        if(seat >= 8)
            return NULL;

        return m_passengers[seat];
    }

    void InstallAccessories();
    //---------------------------------------
    // End accessors
    //---------------------------------------

    bool IsFull() { return m_ppassengerCount >= m_maxPassengers; }
    bool Initialised;
    bool m_CreatedFromSpell;
    uint32 m_CastSpellOnMount;

private:
    void _AddToSlot(Unit* pPassenger, uint8 slot);

protected:
    uint64 vehicleguid;
    uint8 m_ppassengerCount;
    uint8 m_maxPassengers;
    uint8 m_seatSlotMax;

    VehicleEntry* vehicleData;
};
