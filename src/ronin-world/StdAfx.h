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

#define M_PI 3.14159265358979323846f

#include <list>
#include <vector>
#include <map>
#include <unordered_set>
#include <sstream>
#include <string>
#include <fstream>
#include <iosfwd>
#include <search.h>
#include <fcntl.h>
#include <signal.h>
#include <bitset>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sys/types.h>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>

#include <threading/Threading.h>
#include <database/Database.h>
#include <network/Network.h>
#include <dbclib/DBCLib.h>
#include <zlib/zlib.h>

#include <vmaplib/VMapManagerExt.h>
#include <mmaplib/MMapManagerExt.h>

#include "../ronin-shared/git_version.h"
#include "../ronin-shared/Common.h"
#include "../ronin-shared/Util.h"
#include "../ronin-shared/MersenneTwister.h"
#include "../ronin-shared/WorldPacket.h"
#include "../ronin-shared/ByteBuffer.h"
#include "../ronin-shared/Config/IniFiles.h"
#include "../ronin-shared/crc32.h"
#include "../ronin-shared/LocationVector.h"
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
#include "../ronin-shared/MemAllocator.h"

#ifdef NEW_ALLOCATION_TRACKING

void record_alloc(void *data, const char *file, size_t line);
void unrecord_alloc(void *data);

void* operator new(size_t size);
void operator delete(void *ptr);

extern const char* __file__;
extern size_t __line__;
#define new (__file__=__FILE__,__line__=__LINE__) && 0 ? NULL : new

#endif

#include "DBCStores.h"
#include "NameTables.h"
#include "UpdateFields.h"
#include "UpdateMask.h"
#include "Opcodes.h"
#include "WorldStates.h"

#include "SpellNameHashes.h"
#include "SpellDefines.h"
#include "SpellManager.h"
#include "SpellProcManager.h"
#include "EventHandler.h"
#include "LootMgr.h"
#include "SmartBoundBox.h"
#include "Object.h"
#include "CellHandler.h"
#include "TerrainMgr.h"
#include "Map.h"

#include "AIInterface.h"
#include "AuraInterface.h"
#include "UnitPathSystem.h"
#include "MovementInterface.h"
#include "SpellInterface.h"
#include "StatSystem.h"
#include "Unit.h"
#include "CreatureDataMgr.h"
#include "AddonMgr.h"
#include "SkillNameMgr.h"
#include "Chat.h"
#include "Corpse.h"
#include "QuestMgr.h"
#include "Creature.h"
#include "DynamicObject.h"
#include "GameObject.h"
#include "Group.h"
#include "ItemDefines.h"
#include "ItemManager.h"
#include "Skill.h"
#include "Item.h"
#include "Container.h"
#include "AuctionHouse.h"
#include "AuctionMgr.h"
#include "GroupFinder.h"
#include "MailSystem.h"
#include "MapCell.h"
#include "FactionSystem.h"
#include "MiscHandler.h"
#include "NPCHandler.h"
#include "Summons.h"
#include "WorldSocket.h"
#include "World.h"
#include "WorldSession.h"
#include "WorldStateManager.h"
#include "MapScript.h"
#include "MapInstance.h"
#include "QuestInterface.h"
#include "TalentInterface.h"
#include "FactionInterface.h"
#include "InventoryInterface.h"
#include "CurrencyInterface.h"
#include "Player.h"
#include "BaseSpell.h"
#include "SpellEffects.h"
#include "SpellTarget.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "ClassSpells.h"
#include "TaxiMgr.h"
#include "TransportMgr.h"
#include "WeatherMgr.h"
#include "TicketMgr.h"
#include "ObjectMgr.h"
#include "GossipMgr.h"
#include "GuildDefines.h"
#include "GuildManager.h"
#include "AchievementMgr.h"
#include "ContinentManager.h"
#include "InstanceManager.h"
#include "RaidManager.h"
#include "WorldManager.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "LogonCommClient.h"
#include "LogonCommHandler.h"
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
