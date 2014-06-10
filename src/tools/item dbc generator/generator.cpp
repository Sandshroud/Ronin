
#include "generator.h"
#include "Data/DatabaseEnv.h"
#include "Data/dbcfile.h"

using namespace std;
#define WaitForInput() { char cmd[300]; memset( cmd, 0, sizeof( cmd ) ); fgets( cmd, 300, stdin ); }

struct ItemEntry
{
    uint32 ID;
    uint32 Class;
    uint32 SubClass;
    int32  Unk0;
    int32  Material;
    uint32 DisplayId;
    uint32 InventoryType;
    uint32 Sheath;
};

Database* db;
map<uint32, ItemEntry*> m_ItemDataMap;
map<uint32, ItemEntry*> m_DBCItemMap;
map<uint32, ItemEntry*> m_ItemEntryMap;
void _OnSignal(int s)
{
    switch (s)
    {
    case SIGINT:
    case SIGTERM:
    case SIGABRT:
#ifdef _WIN32
    case SIGBREAK:
#endif
        if(db != NULL)
            delete db;

        ItemEntry *buff = NULL;
        if(m_DBCItemMap.size())
        {
            for(map<uint32, ItemEntry*>::iterator itr = m_DBCItemMap.begin(), itr2; itr != m_DBCItemMap.end();)
            {
                itr2 = itr++;
                buff = itr2->second;
                m_DBCItemMap.erase(itr2);
                delete buff;
                buff = NULL;
            }
        }
        if(m_ItemDataMap.size())
        {
            for(map<uint32, ItemEntry*>::iterator itr = m_ItemDataMap.begin(), itr2; itr != m_ItemDataMap.end();)
            {
                itr2 = itr++;
                buff = itr2->second;
                m_ItemDataMap.erase(itr2);
                delete buff;
                buff = NULL;
            }
        }
        if(m_ItemEntryMap.size())
        {
            for(map<uint32, ItemEntry*>::iterator itr = m_ItemEntryMap.begin(), itr2; itr != m_ItemEntryMap.end();)
            {
                itr2 = itr++;
                buff = itr2->second;
                m_ItemEntryMap.erase(itr2);
                delete buff;
                buff = NULL;
            }
        }

        HANDLE pH = OpenProcess( PROCESS_TERMINATE, TRUE, GetCurrentProcessId() );
        TerminateProcess( pH, 1 );
        CloseHandle( pH );
        break;
    }

    signal(s, _OnSignal);
}

char* tableNames[4] =
{
    "items",
    "items",
    "item_template",
    "item_template"
};

int main(int argc, char * arg[])
{
    db = NULL;
    signal( SIGINT, _OnSignal );
    signal( SIGTERM, _OnSignal );
    signal( SIGABRT, _OnSignal );
    signal( SIGBREAK, _OnSignal );

    map<uint32, ItemEntry*> m_DBCItemMap;
    printf("Reading original item.dbc...");
    DBCFile item("original_item.dbc");
    item.open();
    printf("\n");
    if(item.getRecordCount() == 0 || item.getRecordCount() != 46096)
    {
        printf("Original Item DBC not found or unavailable!\nPress enter to quit:");
        WaitForInput();
        return 1;
    }

    for(DBCFile::Iterator itr = item.begin(); itr != item.end(); ++itr)
    {
        ItemEntry* dbcItem = new ItemEntry();
        uint8 f = 0;
        dbcItem->ID = (*itr).getInt(f++);
        dbcItem->Class = (*itr).getInt(f++);
        dbcItem->SubClass = (*itr).getInt(f++);
        dbcItem->Unk0 = (*itr).getInt(f++);
        dbcItem->Material = (*itr).getInt(f++);
        dbcItem->DisplayId = (*itr).getInt(f++);
        dbcItem->InventoryType = (*itr).getInt(f++);
        dbcItem->Sheath = (*itr).getInt(f++);
        m_DBCItemMap.insert(make_pair(dbcItem->ID, dbcItem));
    }

    printf("%u item records loaded.\n", item.getRecordCount());

    char _coreType[255];
    memset( _coreType, 0, sizeof( _coreType ) );
    printf("Select core type:\n1.ArcEmu[Trunk](5f9054bc539ee989859dae950e5a9954e22ed20a)\n2.Sandshroud[0xAE86]\n3.Mangos[master](519842b1983c3a093b3398fa025dade84611fdbe)\n4.TrinityCore[master](c05c16a8dee97a485e0d6a4e73c750e1d140f227)\nSpecify: ");
    fgets( _coreType, 255, stdin );
    printf("\n");
    uint32 coreType = atol(_coreType);
    if(!coreType || coreType > 4)
    {
        printf("Unsupported core type specified!\nPress enter to quit:");
        WaitForInput();
        return 2;
    }
    printf("Core type %u selected\n", coreType);

    char _Adress[255], _port[255], _username[255], _password[255], _dbname[255];
    memset( _Adress, 0, sizeof( _Adress ) );
    memset( _port, 0, sizeof( _port ) );
    memset( _username, 0, sizeof( _username ) );
    memset( _password, 0, sizeof( _password ) );
    memset( _dbname, 0, sizeof( _dbname ) );

    //////////////////
    printf("Enter database address: ");
    fgets( _Adress, 255, stdin );
    printf("\n");

    //////////////////
    printf("Enter database port(usually 3306): ");
    fgets( _port, 255, stdin );
    printf("\n");

    //////////////////
    printf("Enter database username: ");
    fgets( _username, 255, stdin );
    printf("\n");

    //////////////////
    printf("Enter database password: ");
    fgets( _password, 255, stdin );
    printf("\n");

    //////////////////
    printf("Enter database name: ");
    fgets( _dbname, 255, stdin );
    printf("\n");

    string temp, address(_Adress), port(_port), username(_username), password(_password), dbname(_dbname);

    temp = address.substr(0, address.size()-1);
    address = temp;
    temp = port.substr(0, port.size()-1);
    port = temp;
    temp = username.substr(0, username.size()-1);
    username = temp;
    temp = password.substr(0, password.size()-1);
    password = temp;
    temp = dbname.substr(0, dbname.size()-1);
    dbname = temp;

    printf("Connecting with information %s:%s | %s:%s to db %s...\n", address.c_str(), port.c_str(), username.c_str(), password.c_str(), dbname.c_str());
    db = new Database();
    if(!db->Initialize(address.c_str(), atol(port.c_str()), username.c_str(), password.c_str(), dbname.c_str(), 500000))
    {
        printf("Connection failed! Press enter to exit.\n");
        WaitForInput();
        return 3;
    }
    printf("Connection Successful!\nQuerying db...\n");

    char header[4];
    unsigned int na = 0, nb = 8, es = sizeof(ItemEntry), ss = 0;
    header[0] = 'W'; header[1] = 'D';
    header[2] = 'B'; header[3] = 'C';
    stringstream sstream;
    sstream << "SELECT ";
    switch(coreType)
    {
    case 1:
    case 2:
        {
            sstream << "entry, class, subclass, displayid, inventorytype, sheathID";
        }break;
    case 3:
    case 4:
        {
            sstream << "entry, class, subclass, displayid, InventoryType, sheath";
        }break;
    }

    sstream << " FROM `" << tableNames[coreType-1] << "`";

    QueryResult* res = db->Query(sstream.str().c_str());
    if(res == NULL)
    {
        printf("Query failed, make sure %s table exists in selected DB!\n", tableNames[coreType-1]);
        delete db;
        WaitForInput();
        return 4;
    }

    do
    {
        Field* f = res->Fetch();
        ItemEntry* dbcItem = new ItemEntry();
        dbcItem->ID = f[0].GetUInt32();
        dbcItem->Class = f[1].GetUInt32();
        dbcItem->SubClass = f[2].GetUInt32();
        dbcItem->Unk0 = -1;
        dbcItem->Material = -1;
        if(m_DBCItemMap.find(dbcItem->ID) != m_DBCItemMap.end())
            dbcItem->Material = m_DBCItemMap.at(dbcItem->ID)->Material;
        dbcItem->DisplayId = f[3].GetUInt32();
        dbcItem->InventoryType = f[4].GetUInt32();
        dbcItem->Sheath = f[5].GetUInt32();
        m_ItemDataMap.insert(make_pair(dbcItem->ID, dbcItem));
    }while(res->NextRow());
    printf("Query successful, building item.dbc...\n");

    FILE* f = fopen("item.dbc", "wb");
    fwrite(header, 4, 1, f);

    ItemEntry *iE = NULL, *it = NULL, *IEntryV2 = NULL;
    for(uint32 i = 0; i < 200000; i++)
    {
        if(m_ItemDataMap.find(i) != m_ItemDataMap.end())
            it = m_ItemDataMap.at(i);
        if(m_DBCItemMap.find(i) != m_DBCItemMap.end())
            iE = m_DBCItemMap.at(i);
        if(it == NULL && iE == NULL)
            continue;

        IEntryV2 = new ItemEntry();
        if(it == NULL)
        {
            IEntryV2->ID = iE->ID;
            IEntryV2->Class = iE->Class;
            IEntryV2->SubClass = iE->SubClass;
            IEntryV2->Unk0 = iE->Unk0;
            IEntryV2->Material = iE->Material;
            IEntryV2->DisplayId = iE->DisplayId;
            IEntryV2->InventoryType = iE->InventoryType;
            IEntryV2->Sheath = iE->Sheath;
        }
        else
        {
            IEntryV2->ID = it->ID;
            IEntryV2->Class = it->Class;
            IEntryV2->SubClass = it->SubClass;
            IEntryV2->Unk0 = -1;
            IEntryV2->Material = it->Material;
            IEntryV2->DisplayId = it->DisplayId;
            IEntryV2->InventoryType = it->InventoryType;
            IEntryV2->Sheath = it->Sheath;
        }
        m_ItemEntryMap.insert(make_pair(i, IEntryV2));
        na++;
        IEntryV2 = NULL;
        it = NULL;
        iE = NULL;
    }

    fwrite(&na, 4, 1, f);
    fwrite(&nb, 4, 1, f);
    fwrite(&es, 4, 1, f);
    fwrite(&ss, 4, 1, f);
    for(map<uint32, ItemEntry*>::iterator itr = m_ItemEntryMap.begin(); itr != m_ItemEntryMap.end(); itr++)
        fwrite(((uint8*)(itr->second)), es, 1, f);

    fclose(f);

    ItemEntry *buff = NULL;
    if(m_DBCItemMap.size())
    {
        for(map<uint32, ItemEntry*>::iterator itr = m_DBCItemMap.begin(), itr2; itr != m_DBCItemMap.end();)
        {
            itr2 = itr++;
            buff = itr2->second;
            m_DBCItemMap.erase(itr2);
            delete buff;
            buff = NULL;
        }
    }
    if(m_ItemDataMap.size())
    {
        for(map<uint32, ItemEntry*>::iterator itr = m_ItemDataMap.begin(), itr2; itr != m_ItemDataMap.end();)
        {
            itr2 = itr++;
            buff = itr2->second;
            m_ItemDataMap.erase(itr2);
            delete buff;
            buff = NULL;
        }
    }
    if(m_ItemEntryMap.size())
    {
        for(map<uint32, ItemEntry*>::iterator itr = m_ItemEntryMap.begin(), itr2; itr != m_ItemEntryMap.end();)
        {
            itr2 = itr++;
            buff = itr2->second;
            m_ItemEntryMap.erase(itr2);
            delete buff;
            buff = NULL;
        }
    }

    delete db;
    printf("Build success! Press enter to exit:");
    WaitForInput();
    return 0;
}
