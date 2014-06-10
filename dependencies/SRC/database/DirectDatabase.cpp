/***
 * Demonstrike Core
 */

#include "Database.h"

DirectDatabase::DirectDatabase()
{
    _counter = 0;
    m_connections = NULL;
    mConnectionCount = -1;   // Not connected.
    ThreadRunning = true;
}

DirectDatabase::~DirectDatabase()
{
    for(int32 i = 0; i < mConnectionCount; ++i)
    {
        if( m_connections[i].conn != NULL )
            mysql_close(m_connections[i].conn);
    }

    delete [] m_connections;
}

bool DirectDatabase::Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32 ConnectionCount, uint32 BufferSize)
{
    uint32 i;
    MYSQL *temp = NULL, *temp2 = NULL;
    my_bool my_true = true;

    mHostname = strdup(Hostname);
    mConnectionCount = ConnectionCount;
    mUsername = strdup(Username);
    mPassword = strdup(Password);
    mDatabaseName = strdup(DatabaseName);

    bLog.Notice("MySQLDatabase", "Connecting to `%s`, database `%s`...", Hostname, DatabaseName);

    m_connections = new DatabaseConnection[ConnectionCount];
    for( i = 0; i < ConnectionCount; ++i )
    {
        temp = mysql_init( NULL );
        if(temp == NULL)
            continue;

        if(mysql_options(temp, MYSQL_SET_CHARSET_NAME, "utf8"))
            bLog.Error("MySQLDatabase", "Could not set utf8 character set.");

        if (mysql_options(temp, MYSQL_OPT_RECONNECT, &my_true))
            bLog.Error("MySQLDatabase", "MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.");

        temp2 = mysql_real_connect( temp, Hostname, Username, Password, DatabaseName, port, NULL, 0 );
        if( temp2 == NULL )
        {
            bLog.Error("MySQLDatabase", "Connection failed due to: `%s`", mysql_error( temp ) );
            return false;
        }

        m_connections[i].conn = temp2;
    }

    // Spawn Database thread
    sDBEngine.AddDatabase(this);
//  ThreadPool.ExecuteTask("Database Execute Thread", this);

    // launch the query thread
    qt = new QueryThread(this);
    sDBEngine.AddQueryThread(qt);
//  ThreadPool.ExecuteTask("QueryThread", qt);
    return true;
}

DatabaseConnection * DirectDatabase::GetFreeConnection()
{
    uint32 i = 0;
    for(;;)
    {
        DatabaseConnection * con = &m_connections[ ((i++) % mConnectionCount) ];
        if(con->Busy.AttemptAcquire())
            return con;

        // sleep every 20 iterations, otherwise this can cause 100% cpu if the db link goes dead
        if( !(i % 20) )
            Sleep(10);
    }

    // shouldn't be reached
    return NULL;
}

QueryResult * DirectDatabase::Query(const char* QueryString, ...)
{   
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    // Send the query
    QueryResult * qResult = NULL;
    DatabaseConnection * con = GetFreeConnection();

    if(_SendQuery(con, sql, false))
        qResult = _StoreQueryResult( con );
    
    con->Busy.Release();
    return qResult;
}

QueryResult * DirectDatabase::QueryNA(const char* QueryString)
{   
    // Send the query
    QueryResult * qResult = NULL;
    DatabaseConnection * con = GetFreeConnection();

    if( _SendQuery( con, QueryString, false ) )
        qResult = _StoreQueryResult( con );

    con->Busy.Release();
    return qResult;
}

QueryResult * DirectDatabase::FQuery(const char * QueryString, DatabaseConnection * con)
{   
    // Send the query
    QueryResult * qResult = NULL;
    if( _SendQuery( con, QueryString, false ) )
        qResult = _StoreQueryResult( con );

    return qResult;
}

void DirectDatabase::FWaitExecute(const char * QueryString, DatabaseConnection * con)
{   
    // Send the query
    _SendQuery( con, QueryString, false );
}

void QueryBuffer::AddQuery(const char * format, ...)
{
    char query[16384];
    va_list vlist;
    va_start(vlist, format);
    vsnprintf(query, 16384, format, vlist);
    va_end(vlist);

    size_t len = strlen(query);
    char * pBuffer = new char[len+1];
    memcpy(pBuffer, query, len + 1);

    queries.push_back(pBuffer);
}

void QueryBuffer::AddQueryNA( const char * str )
{
    size_t len = strlen(str);
    char * pBuffer = new char[len+1];
    memcpy(pBuffer, str, len + 1);

    queries.push_back(pBuffer);
}

void QueryBuffer::AddQueryStr(const std::string& str)
{
    size_t len = str.size();
    char * pBuffer = new char[len+1];
    memcpy(pBuffer, str.c_str(), len + 1);

    queries.push_back(pBuffer);
}

void DirectDatabase::PerformQueryBuffer(QueryBuffer * b, DatabaseConnection * ccon)
{
    if(!b->queries.size())
        return;

    DatabaseConnection * con = ccon;
    if( ccon == NULL )
        con = GetFreeConnection();

    int result = 1;
    for(std::vector<char*>::iterator itr = b->queries.begin(); itr != b->queries.end(); itr++)
    {
        result = _SendQuery(con, *itr, false);
        if(!result)
            bLog.Error("Database","Sql query failed due to [%s], Query: [%s]", mysql_error( con->conn ), *itr);
    }

    for(std::vector<char*>::iterator itr = b->queries.begin(); itr != b->queries.end(); itr++)
        delete[](*itr);
    b->queries.clear();

    if( ccon == NULL )
        con->Busy.Release();
}

bool DirectDatabase::Execute(const char* QueryString, ...)
{
    char query[16384];

    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(query, 16384, QueryString, vlist);
    va_end(vlist);

    if(!ThreadRunning)
        return WaitExecuteNA(query);

    size_t len = strlen(query);
    char * pBuffer = new char[len+1];
    memcpy(pBuffer, query, len + 1);

    queries_queue.push(pBuffer);
    return true;
}

bool DirectDatabase::ExecuteNA(const char* QueryString)
{
    if(!ThreadRunning)
        return WaitExecuteNA(QueryString);

    size_t len = strlen(QueryString);
    char * pBuffer = new char[len+1];
    memcpy(pBuffer, QueryString, len + 1);

    queries_queue.push(pBuffer);
    return true;
}

//this will wait for completion
bool DirectDatabase::WaitExecute(const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    DatabaseConnection * con = GetFreeConnection();
    bool Result = _SendQuery(con, sql, false);
    con->Busy.Release();
    return Result;
}

bool DirectDatabase::WaitExecuteNA(const char* QueryString)
{
    DatabaseConnection * con = GetFreeConnection();
    bool Result = _SendQuery(con, QueryString, false);
    con->Busy.Release();
    return Result;
}

void DirectDatabase::Update()
{
    DatabaseConnection* con = GetFreeConnection();
    char* q = queries_queue.pop_nowait();
    while(q != NULL)
    {
        _SendQuery(con, q, false);
        delete [] q;
        q = queries_queue.pop_nowait();
    }

    if(con != NULL)
        con->Busy.Release();
}

void AsyncQuery::AddQuery(const char * format, ...)
{
    AsyncQueryResult res;
    va_list ap;
    char buffer[10000];
    size_t len;
    va_start(ap, format);
    vsnprintf(buffer, 10000, format, ap);
    va_end(ap);
    len = strlen(buffer);
    ASSERT(len);
    res.query = new char[len+1];
    res.query[len] = 0;
    memcpy(res.query, buffer, len);
    res.result = NULL;
    queries.push_back(res);
}

void AsyncQuery::Perform()
{
    DatabaseConnection * conn = db->GetFreeConnection();
    for(std::vector<AsyncQueryResult>::iterator itr = queries.begin(); itr != queries.end(); ++itr)
        itr->result = db->FQuery(itr->query, conn);

    conn->Busy.Release();
    func->run(queries);

    delete this;
}

AsyncQuery::~AsyncQuery()
{
    delete func;
    for(std::vector<AsyncQueryResult>::iterator itr = queries.begin(); itr != queries.end(); ++itr)
    {
        if(itr->result)
            delete itr->result;

        delete[] itr->query;
    }
    queries.clear();
}

void DirectDatabase::EndThreads()
{
    ThreadRunning = false;
    Update();
    thread_proc_query();
}

void QueryThread::Update()
{
    db->thread_proc_query();
}

QueryThread::~QueryThread()
{
    db->qt = NULL;
}

void DirectDatabase::thread_proc_query()
{
    DatabaseConnection * con = GetFreeConnection();
    QueryBuffer * q = query_buffer.pop_nowait();
    while(q != NULL)
    {
        PerformQueryBuffer(q, con);
        delete q;
        q = query_buffer.pop_nowait();
    }

    con->Busy.Release();
}

void DirectDatabase::QueueAsyncQuery(AsyncQuery * query)
{
    query->db = this;
    query->Perform();
}

void DirectDatabase::AddQueryBuffer(QueryBuffer * b)
{
    if( qt != NULL )
        query_buffer.push( b );
    else
    {
        PerformQueryBuffer( b, NULL );
        delete b;
    }
}

void DirectDatabase::FreeQueryResult(QueryResult * p)
{
    delete p;
}

std::string DirectDatabase::EscapeString(std::string Escape)
{
    char a2[16384] = {0};

    DatabaseConnection * con = GetFreeConnection();
    const char * ret;
    if(mysql_real_escape_string(con->conn, a2, Escape.c_str(), (unsigned long)Escape.length()) == 0)
        ret = Escape.c_str();
    else
        ret = a2;

    con->Busy.Release();
    return std::string(ret);
}

void DirectDatabase::EscapeLongString(const char * str, uint32 len, std::stringstream& out)
{
    char a2[65536*3] = {0};

    DatabaseConnection * con = GetFreeConnection();
    const char * ret;
    if(mysql_real_escape_string(con->conn, a2, str, (unsigned long)len) == 0)
        ret = str;
    else
        ret = a2;

    out.write(a2, (std::streamsize)strlen(a2));
    con->Busy.Release();
}

std::string DirectDatabase::EscapeString(const char * esc, DatabaseConnection * con)
{
    char a2[16384] = {0};
    const char * ret;
    if(mysql_real_escape_string(con->conn, a2, (char*)esc, (unsigned long)strlen(esc)) == 0)
        ret = esc;
    else
        ret = a2;

    return std::string(ret);
}

bool DirectDatabase::_SendQuery(DatabaseConnection *con, const char* Sql, bool Self)
{
    //dunno what it does ...leaving untouched 
    int result = mysql_query(con->conn, Sql);
    if(result > 0)
    {
        if( Self == false && _HandleError(con, mysql_errno( con->conn ) ) )
        {
            // Re-send the query, the connection was successful.
            // The true on the end will prevent an endless loop here, as it will
            // stop after sending the query twice.
            result = _SendQuery(con, Sql, true);
        }
        else
            bLog.Error("Database","Sql query failed due to [%s], Query: [%s]\n", mysql_error( con->conn ), Sql);
    }

    return (result == 0 ? true : false);
}

bool DirectDatabase::_HandleError(DatabaseConnection * con, uint32 ErrorNumber)
{
    // Handle errors that should cause a reconnect to the Database.
    switch(ErrorNumber)
    {
    case 2006:  // Mysql server has gone away
    case 2008:  // Client ran out of memory
    case 2013:  // Lost connection to sql server during query
    case 2055:  // Lost connection to sql server - system error
        {
            // Let's instruct a reconnect to the db when we encounter these errors.
            return _Reconnect( con );
        }break;
    }

    return false;
}

QueryResult::QueryResult(MYSQL_RES *res, uint32 fields, uint32 rows) : mResult(res), mFieldCount(fields), mRowCount(rows)
{
    mCurrentRow = new Field[fields];
}

QueryResult::~QueryResult()
{
    mysql_free_result(mResult);
    delete [] mCurrentRow;
}

bool QueryResult::NextRow()
{
    MYSQL_ROW row = mysql_fetch_row(mResult);
    if(row == NULL)
        return false;

    for(uint32 i = 0; i < mFieldCount; ++i)
        mCurrentRow[i].SetValue(row[i]);

    return true;
}

QueryResult * DirectDatabase::_StoreQueryResult(DatabaseConnection * con)
{
    QueryResult *res;
    MYSQL_RES * pRes = mysql_store_result( con->conn );
    uint32 uRows = (uint32)mysql_affected_rows( con->conn );
    uint32 uFields = (uint32)mysql_field_count( con->conn );

    if( uRows == 0 || uFields == 0 || pRes == 0 )
    {
        if( pRes != NULL )
            mysql_free_result( pRes );

        return NULL;
    }

    res = new QueryResult( pRes, uFields, uRows );
    res->NextRow();

    return res;
}

bool DirectDatabase::_Reconnect(DatabaseConnection * conn)
{
    MYSQL * temp, *temp2;

    temp = mysql_init( NULL );
    temp2 = mysql_real_connect( temp, mHostname.c_str(), mUsername.c_str(), mPassword.c_str(), mDatabaseName.c_str(), mPort, NULL , 0 );
    if( temp2 == NULL )
    {
        bLog.Error("Database", "Could not reconnect to database because of `%s`", mysql_error( temp ) );
        mysql_close( temp );
        return false;
    }

    if( conn->conn != NULL )
        mysql_close( conn->conn );

    conn->conn = temp;
    return true;
}

void DirectDatabase::CleanupLibs()
{
    mysql_library_end();
}

DirectDatabase *DirectDatabase::Create()
{
    return new DirectDatabase();
}

void DirectDatabase::Shutdown()
{
    mysql_library_end();
}
