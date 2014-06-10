/***
 * Demonstrike Core
 */

#pragma once

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define _GAME // You just lost the game

#ifdef _DEBUG
#ifdef WIN32
#pragma warning(disable:4201)
#endif
#pragma warning(disable:4510)
#pragma warning(disable:4512)
#pragma warning(disable:4610)
#pragma warning(disable:4706)
#endif

#ifdef _MSC_VER
    #define isnan(x) _isnan(x)
#else
    #define isnan(x) std::isnan(x)
#endif

#include <list>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <fstream>
#include <iosfwd>
#include <search.h>
#include <fcntl.h>
#include <signal.h>
#include <bitset>
#include <iomanip>
#include <limits>
#include <sys/types.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>

#include <threading/Threading.h>
#include <database/Database.h>
#include <network/Network.h>
#include <dbclib/DBCLib.h>
#include <zlib/zlib.h>
extern "C" {
#include <pcre/pcre.h>
};

#include <g3dlite/G3D.h>
#include <recast/Recast.h>
#include <detour/Detour.h>

// Being very lazy here..
using namespace std;

#include "../hearthstone-shared/git_version.h"
#include "../hearthstone-shared/Common.h"
#include "../hearthstone-shared/MersenneTwister.h"
#include "../hearthstone-shared/WorldPacket.h"
#include "../hearthstone-shared/hearthstone_log.h"
#include "../hearthstone-shared/ByteBuffer.h"
#include "../hearthstone-shared/Config/IniFiles.h"
#include "../hearthstone-shared/crc32.h"
#include "../hearthstone-shared/LocationVector.h"
#include "../hearthstone-shared/hashmap.h"
#include "../hearthstone-shared/hearthstoneConfig.h"
#include "../hearthstone-shared/RC4Engine.h"
#include "../hearthstone-shared/Auth/MD5.h"
#include "../hearthstone-shared/Auth/BigNumber.h"
#include "../hearthstone-shared/Auth/Sha1.h"
#include "../hearthstone-shared/Auth/WowCrypt.h"
#include "../hearthstone-shared/Client/AuthCodes.h"
#include "../hearthstone-shared/FastQueue.h"
#include "../hearthstone-shared/CircularQueue.h"
#include "../hearthstone-shared/hearthstone_getopt.h"
#include "../hearthstone-shared/NameTables.h"

#include "Const.h"
#include "Defines.h"

#include "../hearthstone-shared/Storage.h"
#include "../hearthstone-shared/PerfCounters.h"
#include "../hearthstone-logonserver/LogonOpcodes.h"

#include "DBCStores.h"
#include "NameTables.h"
#include "UpdateFields.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldStates.h"

#include "Packets.h"
#include "WordFilter.h"
#include "EventMgr.h"
#include "EventableObject.h"
#include "LootMgr.h"
#include "Object.h"
#include "AuraInterface.h"
#include "Unit.h"

#include "AddonMgr.h"
#include "AI_Headers.h"
#include "AreaTrigger.h"
#include "BattlegroundMgr.h"
#include "AlteracValley.h"
#include "ArathiBasin.h"
#include "WarsongGulch.h"
#include "EyeOfTheStorm.h"
#include "StrandOfTheAncients.h"
#include "IsleOfConquest.h"
#include "Arenas.h"
#include "CellHandler.h"
#include "SkillNameMgr.h"
#include "Chat.h"
#include "Corpse.h"
#include "Quest.h"
#include "QuestMgr.h"
#include "TerrainMgr.h"
#include "Map.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Group.h"
#include "HonorHandler.h"
#include "ItemDefines.h"
#include "ItemManager.h"
#include "Skill.h"
#include "Item.h"
#include "Container.h"
#include "AuctionHouse.h"
#include "AuctionMgr.h"
#include "LfgMgr.h"
#include "MailSystem.h"
#include "MapCell.h"
#include "MiscHandler.h"
#include "NPCHandler.h"
#include "Pet.h"
#include "Summons.h"
#include "WorldSocket.h"
#include "World.h"
#include "WorldSession.h"
#include "WorldStateManager.h"
#include "MapManagerScript.h"
#include "MapMgr.h"
#include "DayWatcherThread.h"
#include "Player.h"
#include "FactionSystem.h"
#include "SpellNameHashes.h"
#include "SpellDefines.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "ClassSpells.h"
#include "TaxiMgr.h"
#include "TransporterHandler.h"
#include "WeatherMgr.h"
#include "ItemInterface.h"
#include "Stats.h"
#include "ObjectMgr.h"
#include "GuildDefines.h"
#include "GuildManager.h"
#include "WorldCreator.h"
#include "ScriptSetup.h"
#include "ScriptMgr.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "ArenaTeam.h"
#include "LogonCommClient.h"
#include "LogonCommHandler.h"
#include "WorldRunnable.h"
#include "ObjectStorage.h"
#include "VoiceChatClientSocket.h"
#include "VoiceChatHandler.h"
#include "Vehicle.h"
#include "Tracker.h"
#include "WarnSystem.h"
#include "AchievementInterface.h"

#include "CollideInterface.h"
#include "NavMeshInterface.h"

#include "Master.h"
#include "ConsoleCommands.h"

#if PLATFORM != PLATFORM_WIN

#include <termios.h>
#include <sys/resource.h>
#include <sched.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#include <cstring>

#endif
