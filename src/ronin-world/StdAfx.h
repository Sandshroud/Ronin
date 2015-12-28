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

#define _GAME

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
#define num_isnan(x) _isnan(x)
#else
#define num_isnan(x) std::isnan(x)
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
#include <vmaplib/VMapLib.h>
#include <recast/Recast.h>
#include <detour/Detour.h>

#include "../ronin-shared/git_version.h"
#include "../ronin-shared/Common.h"
#include "../ronin-shared/MersenneTwister.h"
#include "../ronin-shared/WorldPacket.h"
#include "../ronin-shared/ByteBuffer.h"
#include "../ronin-shared/Config/IniFiles.h"
#include "../ronin-shared/crc32.h"
#include "../ronin-shared/LocationVector.h"
#include "../ronin-shared/hashmap.h"
#include "../ronin-shared/RC4Engine.h"
#include "../ronin-shared/Auth/MD5.h"
#include "../ronin-shared/Auth/BigNumber.h"
#include "../ronin-shared/Auth/Sha1.h"
#include "../ronin-shared/Auth/WowCrypt.h"
#include "../ronin-shared/Client/AuthCodes.h"
#include "../ronin-shared/FastQueue.h"
#include "../ronin-shared/CircularQueue.h"
#include "../ronin-shared/startup_getopt.h"
#include "../ronin-shared/NameTables.h"

#include "Const.h"
#include "Defines.h"

#include "../ronin-shared/Storage.h"
#include "../ronin-shared/PerfCounters.h"
#include "../ronin-logonserver/LogonOpcodes.h"

#define CRTDBG_MAP_ALLOC
#include <iostream>
#include <stdlib.h>
#include <crtdbg.h>

#include "DBCStores.h"
#include "NameTables.h"
#include "UpdateFields.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldStates.h"

#include "SpellNameHashes.h"
#include "SpellDefines.h"
#include "EventMgr.h"
#include "EventableObject.h"
#include "LootMgr.h"
#include "Object.h"
#include "AI_Headers.h"
#include "AuraInterface.h"
#include "MovementInterface.h"
#include "StatSystem.h"
#include "Unit.h"

#include "CreatureDataMgr.h"
#include "AddonMgr.h"
#include "BattlegroundMgr.h"
#include "AlteracValley.h"
#include "ArathiBasin.h"
#include "WarsongGulch.h"
#include "EyeOfTheStorm.h"
#include "StrandOfTheAncients.h"
#include "IsleOfConquest.h"
#include "TalentInterface.h"
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
#include "MapMgr.h"
#include "DayWatcherThread.h"
#include "InventoryInterface.h"
#include "CurrencyInterface.h"
#include "Player.h"
#include "FactionSystem.h"
#include "BaseSpell.h"
#include "SpellEffects.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "ClassSpells.h"
#include "TaxiMgr.h"
#include "TransporterHandler.h"
#include "WeatherMgr.h"
#include "TicketMgr.h"
#include "ObjectMgr.h"
#include "GuildDefines.h"
#include "GuildManager.h"
#include "WorldManager.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "ArenaTeam.h"
#include "LogonCommClient.h"
#include "LogonCommHandler.h"
#include "WorldRunnable.h"
#include "ObjectStorage.h"
#include "VoiceChatClientSocket.h"
#include "VoiceChatHandler.h"
#include "Tracker.h"
#include "WarnSystem.h"

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
