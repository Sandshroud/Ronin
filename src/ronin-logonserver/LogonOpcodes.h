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

#ifndef __LOGONOPCODES_H
#define __LOGONOPCODES_H

enum RealmListOpcodes
{
    // Initialization of server/client connection...
    RCMSG_REGISTER_REALM                        = 1,
    RSMSG_REALM_REGISTERED                      = 2,

    // Upon client connect (for WS)
    RCMSG_REQUEST_SESSION                       = 3,
    RSMSG_SESSION_RESULT                        = 4,

    // Ping/Pong
    RCMSG_PING                                  = 5,
    RSMSG_PONG                                  = 6,
    RMSG_LATENCY                                = 7,

    // SQL Query Execute
    RCMSG_SQL_EXECUTE                           = 8,
    RCMSG_RELOAD_ACCOUNTS                       = 9,

    // Authentication
    RCMSG_AUTH_CHALLENGE                        = 10,
    RSMSG_AUTH_RESPONSE                         = 11,

    // Character->Account Transmission
    RSMSG_REQUEST_ACCOUNT_CHARACTER_MAPPING     = 12,
    RCMSG_ACCOUNT_CHARACTER_MAPPING_REPLY       = 13,

    // Update Character->Account Mapping
    RCMSG_UPDATE_CHARACTER_MAPPING_COUNT        = 14,
    RSMSG_DISCONNECT_ACCOUNT                    = 15,

    // Console auth
    RCMSG_TEST_CONSOLE_LOGIN                    = 16,
    RSMSG_CONSOLE_LOGIN_RESULT                  = 17,

    // DB modifying
    RCMSG_MODIFY_DATABASE                       = 18,

    // count
    RMSG_COUNT                                  = 19,
};

#endif
