#ifndef CPROFILERESULT_H
#define CPROFILERESULT_H

#include <list>
//#include <fstream>
#include <stdio.h>

using namespace std;

// extern fstream g_profile_stream;
extern FILE* g_profile_stream;

class CProfileNode;

class CProfileResult
{
public:
    CProfileResult();
    ~CProfileResult();

    static CProfileResult* Instance();
    void    ExportToXML(const char* szFile = "profile.xml");
	void    ExportThreadToXML(const char* szFile = "profile.xml");
    void    DFS(CProfileNode* pNode);
    void    PrintNotRecord(float f1, float f2);
    void    PrintNode(CProfileNode* pNode);

    void    Visited(CProfileNode* pNode);
    bool    HaveVisited(CProfileNode* pNode);
protected:
private:

    static CProfileResult* m_pInstance;
    list<CProfileNode*> m_VisitedNodes;
    CProfileNode*       m_pNextNode;
    CProfileNode*       m_pLastNode;
};

#endif // CPROFILERESULT_H
