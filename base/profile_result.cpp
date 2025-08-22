#include "profile_result.h"

#if !defined(__GNUC__)
#include <windows.h>
#endif

#include <stdarg.h>
#include <stdio.h>
//#include <fstream>
#include <string>
#include <sstream>
#include <string.h>
#include "profile.h"

//fstream g_profile_stream;
FILE* g_profile_stream;

string  g_strKeys[7] = {
    "time_percent_of_parent",
    "time_percent_of_total",
    "frame_time_ms",
    "time_ms",
    "call_count",
    "missing_time",
    "frame_max_time_ms"
};

CProfileResult* CProfileResult::m_pInstance = NULL;


bool profile_log(const char* format, ...)
{
    // if(!g_profile_stream.is_open())
    // {
        // return false;
    // }

    va_list args;
    va_start(args, format);
    const int MAX_BUFFER_SIZE = 1024;
    int nBuf = 0;
    char szBuffer[MAX_BUFFER_SIZE] = {0};

#if !defined(__GNUC__)
    nBuf += _vsnprintf_s(szBuffer, MAX_BUFFER_SIZE, MAX_BUFFER_SIZE, format, args);
#else
    nBuf += vsnprintf(szBuffer, MAX_BUFFER_SIZE, format, args);
#endif
    if(nBuf > MAX_BUFFER_SIZE - 10)
    {
        nBuf = MAX_BUFFER_SIZE - 10;
    }
    int retval = fwrite(szBuffer, 1, strnlen(szBuffer, sizeof(szBuffer)), g_profile_stream);
    (void)retval;
    //g_profile_stream.write(szBuffer, nBuf);
    //g_profile_stream.flush();

    va_end(args);
    return true;
}

CProfileResult::CProfileResult()
{
    //ctor
    m_pNextNode = NULL;
    m_pLastNode = NULL;
}

CProfileResult::~CProfileResult()
{
    //dtor
}

CProfileResult* CProfileResult::Instance()
{
    if(NULL == m_pInstance)
    {
        m_pInstance = new CProfileResult;
    }
    return m_pInstance;
}

void CProfileResult::ExportToXML(const char* szFile /* = "profile.xml" */)
{
#if defined(__GNUC__)
	g_profile_stream = fopen(szFile, "w+");	
#else
	fopen_s(&g_profile_stream, szFile, "w+");
#endif
    //g_profile_stream.open(szFile, ios::out);
    profile_log("<?xml version='1.0' encoding='UTF8' ?>\n");
    DFS(CProfileManager::Root.Get_Child());
    //g_profile_stream.close();
    fclose(g_profile_stream);
    g_profile_stream = NULL;
}

void CProfileResult::ExportThreadToXML(const char* szFile /* = "profile.xml" */)
{
#if defined(__GNUC__)
	g_profile_stream = fopen(szFile, "w+");
#else
	fopen_s(&g_profile_stream, szFile, "w+");
#endif
	//g_profile_stream.open(szFile, ios::out);
	profile_log("<?xml version='1.0' encoding='UTF8' ?>\n");

	g_pThreadProfileManager->GetAllData();
	DFS(&(g_pThreadProfileManager->Root));
	//g_profile_stream.close();
	fclose(g_profile_stream);
	g_profile_stream = NULL;
}

void CProfileResult::DFS(CProfileNode* pNode)
{
    float fPer1, fPer2;
    CProfileNode* pSubNode;
    if(!pNode)
    {
        return;
    }

    string strNodeName = pNode->Get_Name();
    string::size_type pos = strNodeName.find("::");
    if(pos != string::npos)
    {
        strNodeName.replace(pos, 2, ".");
    }

    profile_log("<%s>", strNodeName.c_str());
    PrintNode(pNode);

    fPer1 = 100.0f;
    fPer2 = pNode->Get_Percent_of_Total();

    pSubNode = pNode->Get_Child();

    while(pSubNode != NULL)
    {
        DFS(pSubNode);

        fPer1 -= pSubNode->Get_Percent_of_Parent();
        fPer2 -= pSubNode->Get_Percent_of_Total();

        pSubNode = pSubNode->Get_Sibling();
    }

    if(NULL != pNode->Get_Child())
    {
        PrintNotRecord(fPer1, fPer2);
    }

    profile_log("</%s>\n", strNodeName.c_str());

}

void CProfileResult::PrintNode(CProfileNode* pNode)
{
    if(!pNode)
        return;

    float fPer1, fPer2;
    fPer1 = pNode->Get_Percent_of_Parent();
    fPer2 = pNode->Get_Percent_of_Total();
    //snprintf("<%s>%.2f%s</%s>\n", g_strKeys[0], fPer1, "%", g_strKeys[0]);
    profile_log("<%s>%.2f%s</%s>\n", g_strKeys[0].c_str(), fPer1, "%", g_strKeys[0].c_str());
    profile_log("<%s>%.2f%s</%s>\n", g_strKeys[1].c_str(), fPer2, "%", g_strKeys[1].c_str());
    profile_log("<%s>%8.3fms</%s>\n", g_strKeys[2].c_str(), pNode->Get_Time_Per_Call(), g_strKeys[2].c_str());
    profile_log("<%s>%8.3fms</%s>\n", g_strKeys[3].c_str(), pNode->Get_Total_Time(), g_strKeys[3].c_str());
    profile_log("<%s>%6d</%s>\n", g_strKeys[4].c_str(), pNode->Get_Total_Calls(), g_strKeys[4].c_str());
    profile_log("<%s>%8.3fms</%s>\n", g_strKeys[6].c_str(), pNode->Get_Max_Time(), g_strKeys[6].c_str());


}

void CProfileResult::PrintNotRecord(float f1, float f2)
{
    profile_log("<%s>", g_strKeys[5].c_str());
    profile_log("<%s>%.2f%s</%s>\n", g_strKeys[0].c_str(), f1, "%", g_strKeys[0].c_str());
    profile_log("<%s>%.2f%s</%s>\n", g_strKeys[1].c_str(), f2, "%", g_strKeys[1].c_str());
    profile_log("</%s>", g_strKeys[5].c_str());
}



