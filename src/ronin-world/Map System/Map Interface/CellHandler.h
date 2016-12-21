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

//
// CellHandler.h
//

#pragma once

#define TilesCount 64
#define TileSize 533.33333f
#define _minY (-TilesCount*TileSize/2)
#define _minX (-TilesCount*TileSize/2)

#define _maxY (TilesCount*TileSize/2)
#define _maxX (TilesCount*TileSize/2)

#define CellsPerTile 8
#define _cellSize (TileSize/CellsPerTile)
#define _sizeX (TilesCount*CellsPerTile)
#define _sizeY (TilesCount*CellsPerTile)

#define GetRelatCoord(Coord,CellCoord) ((_maxX-Coord)-CellCoord*_cellSize)

class Map;

template < class Class >
class CellHandler
{
public:
    CellHandler(Map *map);
    ~CellHandler();

    void UnloadCells();

    Class *GetCell(uint32 x, uint32 y);
    Class *GetCellByCoords(float x, float y);
    Class *Create(uint32 x, uint32 y);
    Class *CreateByCoords(float x, float y);
    void Remove(uint32 x, uint32 y);

    RONIN_INLINE bool Allocated(uint32 x, uint32 y)
    { 
        if( x >= _sizeX ||  y >= _sizeY )
            return false;
        return _cells.find(std::make_pair(x, y)) != _cells.end();
    }

    static uint32 GetPosX(float x);
    static uint32 GetPosY(float y);

    RONIN_INLINE Map *GetBaseMap() { return _map; }

protected:
    void _Init();

    std::map<std::pair<uint32, uint32>, Class*> _cells;

    Map* _map;
};

template <class Class>
CellHandler<Class>::CellHandler(Map* map)
{
    _map = map;


    _Init();
}



template <class Class>
void CellHandler<Class>::_Init()
{
    _cells.clear();
}

template <class Class>
CellHandler<Class>::~CellHandler()
{
    UnloadCells();
}

template <class Class>
void CellHandler<Class>::UnloadCells()
{
    while(_cells.size())
    {
        Class * _class = _cells.begin()->second;
        _cells.erase(_cells.begin());
        _class->UnloadCellData(true);
        delete _class;
    }
    _cells.clear();
}

template <class Class>
Class* CellHandler<Class>::Create(uint32 x, uint32 y)
{
    if( x >= _sizeX ||  y >= _sizeY )
        return NULL;

    std::pair<uint32, uint32> cellPair = std::make_pair(x, y);
    if(_cells.find(cellPair) == _cells.end())
        _cells.insert(std::make_pair(cellPair, new Class()));
    return _cells.at(cellPair);
}

template <class Class>
Class* CellHandler<Class>::CreateByCoords(float x, float y)
{
    return Create(GetPosX(x),GetPosY(y));
}

template <class Class>
void CellHandler<Class>::Remove(uint32 x, uint32 y)
{
    if( x >= _sizeX ||  y >= _sizeY )
        return;

    std::pair<uint32, uint32> cellPair = std::make_pair(x, y);
    if(_cells.find(cellPair) == _cells.end())
        return;

    delete _cells.at(cellPair);
    _cells.erase(cellPair);
}

template <class Class>
Class* CellHandler<Class>::GetCell(uint32 x, uint32 y)
{
    if( x >= _sizeX ||  y >= _sizeY )
        return NULL;
    std::pair<uint32, uint32> cellPair = std::make_pair(x, y);
    if(_cells.find(cellPair) == _cells.end())
        return NULL;
    return _cells.at(cellPair);
}

template <class Class>
Class* CellHandler<Class>::GetCellByCoords(float x, float y)
{
    return GetCell(GetPosX(x),GetPosY(y));
}

template <class Class>
uint32 CellHandler<Class>::GetPosX(float x)
{
    ASSERT((x >= _minX) && (x <= _maxX));
    return (uint32)((_maxX-x)/_cellSize);
}

template <class Class>
uint32 CellHandler<Class>::GetPosY(float y)
{
    ASSERT((y >= _minY) && (y <= _maxY));
    return (uint32)((_maxY-y)/_cellSize);
}
