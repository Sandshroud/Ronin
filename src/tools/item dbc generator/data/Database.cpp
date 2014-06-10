/***
 * Demonstrike Core
 */

#include "DatabaseEnv.h"

Database::Database()
{
    conn = NULL;
    _counter = 0;
}

Database::~Database()
{
    mysql_library_end();
    mysql_close(conn);
    conn = NULL;
}

bool Database::Initialize(const char* Hostname, unsigned int port, const char* Username, const char* Password, const char* DatabaseName, uint32 BufferSize)
{
    MYSQL *temp = NULL;
    my_bool my_true = true;

    mHostname = string(Hostname);
    mUsername = string(Username);
    mPassword = string(Password);
    mDatabaseName = string(DatabaseName);

    temp = mysql_init( NULL );
    if(temp == NULL)
        return false;

    if(mysql_options(temp, MYSQL_SET_CHARSET_NAME, "utf8"))
        printf("MySQLDatabase: Could not set utf8 character set.\n");

    if (mysql_options(temp, MYSQL_OPT_RECONNECT, &my_true))
        printf("MySQLDatabase: MYSQL_OPT_RECONNECT could not be set, connection drops may occur but will be counteracted.\n");

    conn = mysql_real_connect( temp, Hostname, Username, Password, DatabaseName, port, NULL, 0 );
    if( conn == NULL )
    {
        printf("MySQLDatabase: Connection failed due to: `%s`\n", mysql_error( temp ) );
        return false;
    }

    return true;
}

QueryResult * Database::Query(const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    // Send the query
    QueryResult * qResult = NULL;
    if(_SendQuery(conn, sql, false))
        qResult = _StoreQueryResult( conn );
    return qResult;
}

QueryResult * Database::QueryNA(const char* QueryString)
{   
    // Send the query
    QueryResult * qResult = NULL;
    if( _SendQuery( conn, QueryString, false ) )
        qResult = _StoreQueryResult( conn );

    return qResult;
}

//this will wait for completion
bool Database::WaitExecute(const char* QueryString, ...)
{
    char sql[16384];
    va_list vlist;
    va_start(vlist, QueryString);
    vsnprintf(sql, 16384, QueryString, vlist);
    va_end(vlist);

    bool Result = _SendQuery(conn, sql, false);
    return Result;
}

bool Database::WaitExecuteNA(const char* QueryString)
{
    bool Result = _SendQuery(conn, QueryString, false);
    return Result;
}

void Database::FreeQueryResult(QueryResult * p)
{
    delete p;
}

string Database::EscapeString(std::string Escape)
{
    char a2[16384] = {0};

    const char * ret;
    if(mysql_real_escape_string(conn, a2, Escape.c_str(), (unsigned long)Escape.length()) == 0)
        ret = Escape.c_str();
    else
        ret = a2;
    return string(ret);
}

bool Database::_SendQuery(MYSQL *con, const char* Sql, bool Self)
{
    //dunno what it does ...leaving untouched 
    int result = mysql_query(con, Sql);
    if(result > 0)
    {
        if( Self == false && _HandleError(con, mysql_errno( con ) ) )
        {
            // Re-send the query, the connection was successful.
            // The true on the end will prevent an endless loop here, as it will
            // stop after sending the query twice.
            result = _SendQuery(con, Sql, true);
        }
        else
            printf("Database: Sql query failed due to [%s], Query: [%s]\n", mysql_error( con ), Sql);
    }

    return (result == 0 ? true : false);
}

bool Database::_HandleError(MYSQL * con, uint32 ErrorNumber)
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

QueryResult * Database::_StoreQueryResult(MYSQL * con)
{
    QueryResult *res;
    MYSQL_RES * pRes = mysql_store_result( con );
    uint32 uRows = (uint32)mysql_affected_rows( con );
    uint32 uFields = (uint32)mysql_field_count( con );

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

bool Database::_Reconnect(MYSQL * con)
{
    MYSQL * temp, *temp2;

    temp = mysql_init( NULL );
    temp2 = mysql_real_connect( temp, mHostname.c_str(), mUsername.c_str(), mPassword.c_str(), mDatabaseName.c_str(), mPort, NULL , 0 );
    if( temp2 == NULL )
    {
        printf("Database: Could not reconnect to database because of `%s`\n", mysql_error( temp ) );
        mysql_close( temp );
        return false;
    }

    if( con != NULL )
        mysql_close( con );

    con = temp;
    return true;
}

void Database::CleanupLibs()
{
    mysql_library_end();
}

Database *Database::Create()
{
    return new Database();
}
