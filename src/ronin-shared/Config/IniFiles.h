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

#ifndef _WIN322
#include <map>
#include <string>
#endif

class SERVER_DECL CIniFile
{
public:
    CIniFile(char* szFileName);
    void Reload();
    void Unload();

    int ReadInteger(char* szSection, char* szKey, int iDefaultValue);
    float ReadFloat(char* szSection, char* szKey, float fltDefaultValue);
    bool ReadBoolean(char* szSection, char* szKey, bool bolDefaultValue);
    std::string ReadString(char* szSection, char* szKey, const char* szDefaultValue);
    int ParseError();

    const char* getFileName() { return m_szFileName; }

private:
    int _error;
    char m_szFileName[255];
#ifndef _WIN32
    std::map<std::string, std::string> _values;
    static int ValueHandler(void* user, const char* section, const char* name, const char* value);
#endif
};

std::string MakeKey(std::string section, std::string name);

SERVER_DECL extern CIniFile *mainIni;
