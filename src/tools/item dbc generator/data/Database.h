/***
 * Demonstrike Core
 */

#pragma once

#include <string>
#include "mysql\mysql.h"

using namespace std;
class QueryResult;

class Database
{
public:
    Database();
    virtual ~Database();

    /************************************************************************/
    /* Virtual Functions                                                    */
    /************************************************************************/
    bool Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32 BufferSize);

    QueryResult* Query(const char* QueryString, ...);
    QueryResult* QueryNA(const char* QueryString);
    bool WaitExecute(const char* QueryString, ...);//Wait For Request Completion
    bool WaitExecuteNA(const char* QueryString);//Wait For Request Completion
    bool Execute(const char* QueryString, ...);
    bool ExecuteNA(const char* QueryString);

    HEARTHSTONE_INLINE const string& GetHostName() { return mHostname; }
    HEARTHSTONE_INLINE const string& GetDatabaseName() { return mDatabaseName; }

    string EscapeString(string Escape);

    void FreeQueryResult(QueryResult * p);

    MYSQL *conn;

    static void CleanupLibs();
    static Database *Create();

protected:

    // actual query function
    bool _SendQuery(MYSQL *con, const char* Sql, bool Self);
    QueryResult * _StoreQueryResult(MYSQL * con);
    bool _HandleError(MYSQL *conn, uint32 ErrorNumber);
    bool _Reconnect(MYSQL *conn);

    uint32 _counter;

    // For reconnecting a broken connection
    string mHostname;
    string mUsername;
    string mPassword;
    string mDatabaseName;
    uint32 mPort;
};

class QueryResult
{
public:
    QueryResult(MYSQL_RES *res, uint32 fields, uint32 rows);
    ~QueryResult();

    bool NextRow();
    void Delete() { delete this; }

    HEARTHSTONE_INLINE Field* Fetch() { return mCurrentRow; }
    HEARTHSTONE_INLINE uint32 GetFieldCount() const { return mFieldCount; }
    HEARTHSTONE_INLINE uint32 GetRowCount() const { return mRowCount; }

protected:
    uint32 mFieldCount;
    uint32 mRowCount;
    Field *mCurrentRow;
    MYSQL_RES *mResult;
};
