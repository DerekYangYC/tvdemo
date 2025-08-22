#pragma once
#include <map>

#if !defined(__GNUC__)
#include <windows.h>
#endif

struct SShmItem
{
    SShmItem()
    {
        m_nHandle   = 0;
        m_pShm      = NULL;
        m_nRef      = 0;
        m_nKey      = 0;
        m_dwSize    = 0;
        m_bCreat  = false;
    }

#if defined(__GNUC__)
	int32_t   m_nHandle;
#else
	HANDLE	m_nHandle;
#endif
    void*   m_pShm;
	int32_t   m_nRef;
	int32_t   m_nKey;
	uint32_t  m_dwSize;
    bool    m_bCreat;
};

typedef std::map<int32_t, SShmItem*>  CShmMap;

class CShmMgr
{
    CShmMgr();
    virtual ~CShmMgr();

public:
    static CShmMgr*  Instance();
    
	SShmItem* GetShmItem(int32_t nKey);
    bool AddShmItem(SShmItem* pstShmitem);
	bool DeleteShm(int32_t nKey);
	int32_t GetItemSize();
    void Release();

protected:
    static CShmMgr*      s_poInstance;
    
    CShmMap              m_oShmHandle;
};

extern int ZeaLink_CreateShm(int32_t nKey, uint32_t dwSize, void** ppShmRet);
extern int Zealink_DeleteShm(int32_t nKey);

