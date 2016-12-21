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

#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>
//#include <fstream>

#include "Common.h"
#include <Network/Network.h>

#include "../ronin-shared/git_version.h"
#include "../ronin-shared/Util.h"
#include "../ronin-shared/ByteBuffer.h"
#include "../ronin-shared/Config/IniFiles.h"
#include <zlib/zlib.h>

#include <threading/Threading.h>
#include <database/Database.h>

#include "../ronin-shared/Auth/BigNumber.h"
#include "../ronin-shared/Auth/Sha1.h"
#include "../ronin-shared/Auth/WowCrypt.h"
#include "../ronin-shared/NameTables.h"

#include "LogonOpcodes.h"
#include "../ronin-logonserver/Main.h"
#include "AccountCache.h"
#include "PeriodicFunctionCall_Thread.h"
#include "../ronin-logonserver/AutoPatcher.h"
#include "../ronin-logonserver/AuthSocket.h"
#include "../ronin-logonserver/AuthStructs.h"
#include "../ronin-logonserver/LogonOpcodes.h"
#include "../ronin-logonserver/LogonCommServer.h"
#include "../ronin-logonserver/LogonConsole.h"
#include "../ronin-shared/WorldPacket.h"

// database decl
extern DirectDatabase* sLogonSQL;
