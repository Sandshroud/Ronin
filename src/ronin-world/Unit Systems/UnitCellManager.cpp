/*
 * Sandshroud Project Ronin
 * Copyright (C) 2016-2017 Sandshroud <https://github.com/Sandshroud>
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

#include "StdAfx.h"

void UnitCellManager::OnRemoveFromWorld()
{
    _currX = _currY = 0;
    _processedCells.clear();
    _delayedCells[0].clear();
    _delayedCells[1].clear();
}

#define MAX_CELL_UPDATES 4

void UnitCellManager::Update(MapInstance *instance, uint32 msTime, uint32 uiDiff)
{
    uint32 count = 0;
    while(!_delayedCells[0].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[0].begin());
        _delayedCells[0].erase(_delayedCells[0].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(m_Unit, pair.first, pair.second, true);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }

    while(!_delayedCells[1].empty())
    {
        std::pair<uint16, uint16> pair = unPack(*_delayedCells[1].begin());
        _delayedCells[1].erase(_delayedCells[1].begin());

        _processedCells.insert(_makeCell(pair.first, pair.second));
        instance->UpdateCellData(m_Unit, pair.first, pair.second, false);
        if(++count >= MAX_CELL_UPDATES)
            return;
    }

    // We have no more cells to process, so queue some priority updates
    uint16 lowX = _currX >= 1 ? _currX-1 : 0,
        lowY = _currY >= 1 ? _currY-1 : 0,
        highX = std::min<uint16>(_currX+1, _sizeX-1),
        highY = std::min<uint16>(_currY+1, _sizeY-1);
    for(uint16 x = lowX; x <= highX; x++)
    {
        for(uint16 y = lowY; y <= highY; y++)
        {
            if(x == _currX && y == _currY)
                continue;

            _delayedCells[0].insert(_makeCell(x, y));
        }
    }
}

void UnitCellManager::SetCurrentCell(MapInstance *instance, uint16 newX, uint16 newY, uint8 cellRange)
{
    _currX = newX;
    _currY = newY;

    // Remove any pending cell handling
    _delayedCells[0].clear();
    _delayedCells[1].clear();

    std::set<uint32> preProcessed;
    // Push old processed data to preProcessed group
    preProcessed.insert(_processedCells.begin(), _processedCells.end());
    _processedCells.clear();
    _processedCells.insert(_makeCell(_currX, _currY));

    if(cellRange)
    {   // Fill priority cells from a range of 1
        _lowX = _currX >= 1 ? _currX-1 : 0;
        _lowY = _currY >= 1 ? _currY-1 : 0;
        _highX = std::min<uint16>(_currX+1, _sizeX-1);
        _highY = std::min<uint16>(_currY+1, _sizeY-1);
        for(uint16 x = _lowX; x <= _highX; x++)
        {
            for(uint16 y = _lowY; y <= _highY; y++)
            {
                uint32 cellId = _makeCell(x, y);
                if(preProcessed.find(cellId) != preProcessed.end())
                {
                    _processedCells.insert(cellId);
                    preProcessed.erase(cellId);
                    continue;
                }
                if(_processedCells.find(cellId) != _processedCells.end())
                    continue;

                _delayedCells[0].insert(cellId);
            }
        }

        // Process non priority cell bounding
        if(cellRange > 1)
        {   // Calculate our new bounds based on range
            _lowX = _currX >= cellRange ? _currX-cellRange : 0;
            _lowY = _currY >= cellRange ? _currY-cellRange : 0;
            _highX = std::min<uint16>(_currX+cellRange, _sizeX-1);
            _highY = std::min<uint16>(_currY+cellRange, _sizeY-1);

            for(uint16 x = _lowX; x <= _highX; x++)
            {
                for(uint16 y = _lowY; y <= _highY; y++)
                {
                    uint32 cellId = _makeCell(x, y);
                    if(preProcessed.find(cellId) != preProcessed.end())
                    {
                        _processedCells.insert(cellId);
                        preProcessed.erase(cellId);
                        continue;
                    }
                    if(_processedCells.find(cellId) != _processedCells.end())
                        continue;

                    _delayedCells[1].insert(cellId);
                }
            }
        }
    }

    instance->UpdateCellData(m_Unit, _currX, _currY, true);
    instance->RemoveCellData(m_Unit, preProcessed);
}
