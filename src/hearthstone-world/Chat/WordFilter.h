/***
 * Demonstrike Core
 */

#pragma once

struct WordFilterMatch
{
    char * szMatch;
    char * szIgnoreMatch;
    void* pCompiledExpression;
    void* pCompiledIgnoreExpression;
    void* pCompiledExpressionOptions;
    void* pCompiledIgnoreExpressionOptions;
    int iType;
};

class WordFilter
{
    WordFilterMatch ** m_filters;
    size_t m_filterCount;

    bool CompileExpression(const char * szExpression, void** pOutput, void** pExtraOutput);

public:
    WordFilter() : m_filters(NULL),m_filterCount(0) {}
    ~WordFilter();

    void Load(const char * szTableName);
    bool Parse(std::string& sMessage, bool bAllowReplace = true);
};

extern WordFilter * g_characterNameFilter;
extern WordFilter * g_chatFilter;
