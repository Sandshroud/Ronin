
#pragma once

class UnitCellManager
{
public:
    UnitCellManager(Unit *unit) : m_Unit(unit), _currX(0xFFFF), _currY(0xFFFF) {}
    ~UnitCellManager() {}

    void OnRemoveFromWorld();

    void Update(MapInstance *instance, uint32 msTime, uint32 uiDiff);
    void SetCurrentCell(MapInstance *instance, uint16 newX, uint16 newY, uint8 cellRange);

private:
    friend class MapInstance;
    static uint32 _makeCell(uint16 x, uint16 y) { return (((uint32)x)<<16) | ((uint32)y); }
    static std::pair<uint16, uint16> unPack(uint32 cellId) { return std::make_pair(((uint16)(cellId>>16)), ((uint16)(cellId & 0x0000FFFF))); }

    uint16 _currX, _currY, _lowX, _lowY, _highX, _highY;
    // Stack 0 is higher priority, stack 1 is lower priority
    std::set<uint32> _delayedCells[2], _processedCells;

    Unit *m_Unit;
};
