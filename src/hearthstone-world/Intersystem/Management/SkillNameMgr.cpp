/***
 * Demonstrike Core
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
