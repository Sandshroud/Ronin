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

#define NUM_MAPS 900
#define CL_BUILD_SUPPORT 15595

#define CharacterDatabase (*Database_Character)
#define WorldDatabase (*Database_World)
#define LogDatabase (*Database_Log)

template<typename T> T cast(void *val) { return static_cast<T>(val); }
template<typename T> T *castPtr(void *val) { return static_cast<T*>(val); }

SERVER_DECL extern DirectDatabase* Database_Character;
SERVER_DECL extern DirectDatabase* Database_World;
SERVER_DECL extern DirectDatabase* Database_Log;

SERVER_DECL bool Rand(float chance);
SERVER_DECL bool Rand(uint32 chance);
SERVER_DECL bool Rand(int32 chance);
