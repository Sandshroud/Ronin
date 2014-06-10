/***
 * Demonstrike Core
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
