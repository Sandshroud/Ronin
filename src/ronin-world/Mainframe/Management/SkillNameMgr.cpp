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

#include "StdAfx.h"

SkillNameMgr::SkillNameMgr()
{
    DBCFile SkillDBC;

    if(!SkillDBC.open(format("%s/SkillLine.dbc", sWorld.DBCPath.c_str()).c_str()))
    {
        sLog.Error( "SkillNameMgr", "Cannot find file %s/SkillLine.dbc", sWorld.DBCPath.c_str());
        return;
    }

    //This will become the size of the skill name lookup table
    maxskill = SkillDBC.getRecord(SkillDBC.getRecordCount()-1).getUInt(0);

    //SkillNames = (char **) malloc(maxskill * sizeof(char *));
    SkillNames = new char * [maxskill+1]; //(+1, arrays count from 0.. not 1.)
    memset(SkillNames,0,(maxskill+1) * sizeof(char *));

    for(uint32 i = 0; i < SkillDBC.getRecordCount(); i++)
    {
        unsigned int SkillID = SkillDBC.getRecord(i).getUInt(0);
        const char *SkillName = SkillDBC.getRecord(i).getString(3);

        SkillNames[SkillID] = new char [strlen(SkillName)+1];
        //When the DBCFile gets cleaned up, so does the record data, so make a copy of it..
        memcpy(SkillNames[SkillID],SkillName,strlen(SkillName)+1);
    }
}
