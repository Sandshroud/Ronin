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

#include "../Common.h"
#ifndef _WIN32
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include "unix/ini.h"
#endif
#include "IniFiles.h"

SERVER_DECL CIniFile *mainIni;

CIniFile::CIniFile(char* szFileName) : _error(0)
{
    memset(m_szFileName, 0x00, 255);
    memcpy(m_szFileName, szFileName, strlen(szFileName));
#ifdef _WIN32
    FILE *iniFile = NULL;
    fopen_s(&iniFile, szFileName, "r");
    if(iniFile != NULL)
        fclose(iniFile);
    else _error = -1;
#else
    _error = ini_parse(m_szFileName, ValueHandler, this);
#endif
}

void CIniFile::Reload()
{
    _error = 0;
#ifdef _WIN32
    FILE *iniFile = NULL;
    fopen_s(&iniFile, m_szFileName, "r");
    if(iniFile != NULL)
        fclose(iniFile);
    else _error = -1;
#else
    std::map<std::string, std::string> _oldValues;
    for(std::map<std::string, std::string>::iterator itr = _values.begin(); itr != _values.end(); itr++)
        _oldValues[itr->first] = itr->second;

    _error = ini_parse(m_szFileName, ValueHandler, this);
    if(_error)
    {
        _values.clear();
        for(std::map<std::string, std::string>::iterator itr = _oldValues.begin(); itr != _oldValues.end(); itr++)
            _values[itr->first] = itr->second;
    }
#endif
}

void CIniFile::Unload()
{
    delete [] (char*)m_szFileName;
#ifndef _WIN32
    _values.clear();
#endif
}

int CIniFile::ParseError()
{
    return _error;
}

#ifndef _WIN32
int CIniFile::ValueHandler(void* user, const char* section, const char* name, const char* value)
{
    CIniFile* reader = (CIniFile*)user;
    std::string key = MakeKey(section, name);
    if (reader->_values[key].size() > 0)
        reader->_values[key] += "\n";
    reader->_values[key] += value;
    return 1;
}
#endif

/**************************
 * Read
 */
int CIniFile::ReadInteger(char* szSection, char* szKey, int iDefaultValue)
{
    int iResult = iDefaultValue;
    std::string valstr = ReadString(szSection, szKey, "");
    if(valstr.length())
        iResult = atol(valstr.c_str());
    return iResult;
}

float CIniFile::ReadFloat(char* szSection, char* szKey, float fltDefaultValue)
{
    float fltResult = fltDefaultValue;
    std::string valstr = ReadString(szSection, szKey, "");
    if(valstr.length())
        fltResult = (float)atof(valstr.c_str());
    return fltResult;
}

bool CIniFile::ReadBoolean(char* szSection, char* szKey, bool bolDefaultValue)
{
    bool bolResult = bolDefaultValue;
    std::string valstr = ReadString(szSection, szKey, "");
    // Convert to lower case to make string comparisons case-insensitive
    std::transform(valstr.begin(), valstr.end(), valstr.begin(), ::tolower);
    if (valstr == "true" || valstr == "yes" || valstr == "on" || valstr == "1")
        bolResult = true;
    else if (valstr == "false" || valstr == "no" || valstr == "off" || valstr == "0")
        bolResult = false;
    return bolResult;
}

#ifndef _WIN32
std::string MakeKey(std::string section, std::string name)
{
    std::string key = section + "." + name;
    // Convert to lower case to make section/name lookups case-insensitive
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    return key;
}
#endif

std::string CIniFile::ReadString(char* szSection, char* szKey, const char* szDefaultValue)
{
    std::string key;
#ifdef _WIN32
    char* szResult = new char[255];
    memset(szResult, 0x00, 255);
    if(!GetPrivateProfileString(szSection,  szKey, szDefaultValue, szResult, 255, m_szFileName))
        _error = GetLastError();
    key.append(szResult);
    delete [] szResult;
    return key;
#else
    key = MakeKey(szSection, szKey);
    return (_values.count(key) ? _values[key] : szDefaultValue);
#endif
}
