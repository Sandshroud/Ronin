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

class QueryResult;
class QueryThread;
class DirectDatabase;

struct DatabaseConnection
{
    Mutex Busy;
    MYSQL *conn;
};

struct SERVER_DECL AsyncQueryResult
{
    QueryResult * result;
    char * query;
};

class SERVER_DECL AsyncQuery
{
    friend class DirectDatabase;
    SQLCallbackBase * func;
    std::vector<AsyncQueryResult> queries;
    DirectDatabase * db;
public:
    AsyncQuery(SQLCallbackBase * f) : func(f) {}
    ~AsyncQuery();
    void AddQuery(const char * format, ...);
    void Perform();
    RONIN_INLINE void SetDB(DirectDatabase * dbb) { db = dbb; }
};

class SERVER_DECL QueryBuffer
{
    std::vector<char*> queries;
public:
    friend class DirectDatabase;
    void AddQuery( const char * format, ... );
    void AddQueryNA( const char * str );
    void AddQueryStr(const std::string& str);
};

class SERVER_DECL DirectDatabase
{
    friend class QueryThread;
    friend class AsyncQuery;

public:
    DirectDatabase();
    virtual ~DirectDatabase();

    /************************************************************************/
    /* Thread Stuff                                                         */
    /************************************************************************/
    void Update();
    bool ThreadRunning;

    /************************************************************************/
    /* Virtual Functions                                                    */
    /************************************************************************/
    bool Initialize(const char* Hostname, unsigned int port,
        const char* Username, const char* Password, const char* DatabaseName,
        uint32 ConnectionCount, uint32 BufferSize);
    
    void Shutdown();

    QueryResult* Query(const char* QueryString, ...);
    QueryResult* QueryNA(const char* QueryString);
    QueryResult * FQuery(const char * QueryString, DatabaseConnection *con);
    void FWaitExecute(const char * QueryString, DatabaseConnection *con);
    bool WaitExecute(const char* QueryString, ...);//Wait For Request Completion
    bool WaitExecuteNA(const char* QueryString);//Wait For Request Completion
    bool Execute(const char* QueryString, ...);
    bool ExecuteNA(const char* QueryString);

    RONIN_INLINE const std::string& GetHostName() { return mHostname; }
    RONIN_INLINE const std::string& GetDatabaseName() { return mDatabaseName; }
    RONIN_INLINE const uint32 GetQueueSize() { return queries_queue.get_size(); }

    std::string EscapeString(std::string Escape);
    void EscapeLongString(const char * str, uint32 len, std::stringstream& out);
    std::string EscapeString(const char * esc, DatabaseConnection *con);

    void QueueAsyncQuery(AsyncQuery * query);
    void EndThreads();
    
    void thread_proc_query();
    void FreeQueryResult(QueryResult * p);

    void AssignThreadConnection();
    void ReleaseThreadConnection();

    DatabaseConnection *GetFreeConnection();

    void PerformQueryBuffer(QueryBuffer * b, DatabaseConnection *ccon);
    void AddQueryBuffer(QueryBuffer * b);

    static void CleanupLibs();
    static DirectDatabase *Create();

protected:

    // actual query function
    bool _SendQuery(DatabaseConnection *con, const char* Sql, bool Self);
    QueryResult * _StoreQueryResult(DatabaseConnection * con);
    bool _HandleError(DatabaseConnection *conn, uint32 ErrorNumber);
    bool _Reconnect(DatabaseConnection *conn);

    ////////////////////////////////
    FQueue<QueryBuffer*> query_buffer;

    ////////////////////////////////
    FQueue<char*> queries_queue;
    DatabaseConnection *m_connections;
    std::map<uint32, DatabaseConnection*> m_assignedConnections;
    
    uint32 _counter;
    ///////////////////////////////

    int32 mConnectionCount;

    // For reconnecting a broken connection
    std::string mHostname;
    std::string mUsername;
    std::string mPassword;
    std::string mDatabaseName;
    uint32 mPort;

    QueryThread * qt;
};

class SERVER_DECL QueryResult
{
public:
    QueryResult(MYSQL_RES *res, uint32 fields, uint32 rows);
    ~QueryResult();

    bool NextRow();
    void Delete() { delete this; }

    RONIN_INLINE Field* Fetch() { return mCurrentRow; }
    RONIN_INLINE uint32 GetFieldCount() const { return mFieldCount; }
    RONIN_INLINE uint32 GetRowCount() const { return mRowCount; }

protected:
    uint32 mFieldCount;
    uint32 mRowCount;
    Field *mCurrentRow;
    MYSQL_RES *mResult;
};

class SERVER_DECL QueryThread
{
    friend class DirectDatabase;
    DirectDatabase * db;
public:
    QueryThread(DirectDatabase * d) : db(d) {}
    ~QueryThread();
    void Update();
};
