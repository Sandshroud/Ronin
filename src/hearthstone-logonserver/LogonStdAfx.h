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

#include "../hearthstone-shared/git_version.h"
#include "../hearthstone-shared/hearthstone_log.h"
#include "../hearthstone-shared/Util.h"
#include "../hearthstone-shared/ByteBuffer.h"
#include "../hearthstone-shared/Config/IniFiles.h"
#include <zlib/zlib.h>

#include <threading/Threading.h>
#include <database/Database.h>

#include "../hearthstone-shared/Auth/BigNumber.h"
#include "../hearthstone-shared/Auth/Sha1.h"
#include "../hearthstone-shared/Auth/WowCrypt.h"
#include "../hearthstone-shared/NameTables.h"

#include "LogonOpcodes.h"
#include "../hearthstone-logonserver/Main.h"
#include "AccountCache.h"
#include "PeriodicFunctionCall_Thread.h"
#include "../hearthstone-logonserver/AutoPatcher.h"
#include "../hearthstone-logonserver/AuthSocket.h"
#include "../hearthstone-logonserver/AuthStructs.h"
#include "../hearthstone-logonserver/LogonOpcodes.h"
#include "../hearthstone-logonserver/LogonCommServer.h"
#include "../hearthstone-logonserver/LogonConsole.h"
#include "../hearthstone-shared/WorldPacket.h"

// database decl
extern DirectDatabase* sLogonSQL;
