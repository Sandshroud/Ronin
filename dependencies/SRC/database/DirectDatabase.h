/***
 * Demonstrike Core
 */

#pragma once

class QueryResult;
class QueryThread;
class DirectDatabase;

struct DatabaseConnection
{
    SmartMutex Busy;
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
    HEARTHSTONE_INLINE void SetDB(DirectDatabase * dbb) { db = dbb; }
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

    HEARTHSTONE_INLINE const std::string& GetHostName() { return mHostname; }
    HEARTHSTONE_INLINE const std::string& GetDatabaseName() { return mDatabaseName; }
    HEARTHSTONE_INLINE const uint32 GetQueueSize() { return queries_queue.get_size(); }

    std::string EscapeString(std::string Escape);
    void EscapeLongString(const char * str, uint32 len, std::stringstream& out);
    std::string EscapeString(const char * esc, DatabaseConnection *con);

    void QueueAsyncQuery(AsyncQuery * query);
    void EndThreads();
    
    void thread_proc_query();
    void FreeQueryResult(QueryResult * p);

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

    HEARTHSTONE_INLINE Field* Fetch() { return mCurrentRow; }
    HEARTHSTONE_INLINE uint32 GetFieldCount() const { return mFieldCount; }
    HEARTHSTONE_INLINE uint32 GetRowCount() const { return mRowCount; }

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
