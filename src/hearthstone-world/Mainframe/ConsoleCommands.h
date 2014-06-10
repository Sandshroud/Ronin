/***
 * Demonstrike Core
 */

#pragma once

void reloadgossipscripts();

bool HandleInfoCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleSuicideCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleGMsCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleWAnnounceCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleKickCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleQuitCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleRestartCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleCancelCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUptimeCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleBanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUnbanAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleMOTDCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleOnlinePlayersCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandlePlayerInfoCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleCreateAccountCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleRehashCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleBackupDBCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleSaveAllCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleWhisperCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleNameHashCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleReloadAllScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUnloadSingleScriptCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleUnloadAllScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleLoadNewScriptsCommand(BaseConsole * pConsole, int argc, const char * argv[]);
bool HandleLoadSingleScriptCommand(BaseConsole * pConsole, int argc, const char * argv[]);
