/***
 * Demonstrike Core
 */

#pragma once

class SERVER_DECL SkillNameMgr
{
public:
    char **SkillNames;
    uint32 maxskill;

    SkillNameMgr();

    ~SkillNameMgr()
    {
        for(uint32 i = 0;i<=maxskill;++i)
        {
            if(SkillNames[i] != 0)
                delete[] SkillNames[i];
        }
        delete[] SkillNames;
    }
};
