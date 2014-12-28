/***
 * Demonstrike Core
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
