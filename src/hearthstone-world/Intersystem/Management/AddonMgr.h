/***
 * Demonstrike Core
 */

#pragma once

struct AddonEntry
{
    std::string name;
    uint32 crc;
    bool banned;
    bool isNew;
    bool showinlist;
};

class AddonMgr :  public Singleton < AddonMgr >
{
public:
    AddonMgr();
    ~AddonMgr();

    void LoadFromDB();
    void SaveToDB();

    void SendAddonInfoPacket(WorldPacket *source, WorldSession *m_session);

private:
    std::map<std::string, AddonEntry*> KnownAddons;
    map<string, ByteBuffer> AddonData;

    bool IsAddonBanned(uint32 crc, std::string name = "");
    bool IsAddonBanned(std::string name, uint32 crc = 0);
    bool ShouldShowInList(std::string name, uint32 crc);
};

#define sAddonMgr AddonMgr::getSingleton()
