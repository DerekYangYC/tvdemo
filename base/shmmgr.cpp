
#include "shmmgr.h"

#include <sys/types.h>

#if defined(__GNUC__)
#include <sys/shm.h>
#else
#include <windows.h>
#endif

#include "logging.h"

CShmMgr* CShmMgr::s_poInstance = NULL;

CShmMgr* CShmMgr::Instance()
{
    if(NULL == s_poInstance)
    {
        s_poInstance = new CShmMgr;
    }

    return s_poInstance;
}

CShmMgr::CShmMgr()
{
}

CShmMgr::~CShmMgr()
{
    for(CShmMap::iterator it = m_oShmHandle.begin(); it != m_oShmHandle.end(); it++)
    {
        delete it->second;
    }

    m_oShmHandle.clear();
}

void CShmMgr::Release()
{
    if(NULL != s_poInstance)
    {
        delete s_poInstance;
        s_poInstance = NULL;
    }
}

SShmItem* CShmMgr::GetShmItem(int32_t nKey)
{
    CShmMap::iterator it = m_oShmHandle.find(nKey);
    if(it != m_oShmHandle.end() )
    {
        return it->second;
    }
    return NULL;
}

bool CShmMgr::AddShmItem(SShmItem* pstShmitem)
{
    if(NULL == pstShmitem)
    {
        assert(false);
        return false;
    }

    if(GetShmItem(pstShmitem->m_nKey) != NULL)
    {
        return false;
    }

    m_oShmHandle[pstShmitem->m_nKey] = pstShmitem;
    return true;
}

bool CShmMgr::DeleteShm(int32_t nKey)
{
    SShmItem* pstItem = GetShmItem(nKey);
    if(NULL == pstItem)
    {
        return false;
    }

    pstItem->m_nRef--;
    if(pstItem->m_nRef <= 0)
	{

#if defined(__GNUC__)
		shmdt(pstItem->m_pShm);		
#else
		UnmapViewOfFile(pstItem->m_pShm);
#endif
        //
        //只有创建者才删除它
        //
        if(true == pstItem->m_bCreat)
        {
#if defined(__GNUC__)
			shmctl(pstItem->m_nHandle, IPC_RMID, NULL);			
#else
			CloseHandle(pstItem->m_nHandle);
#endif
        }
        delete pstItem;
        m_oShmHandle.erase(nKey);
    }

    return true;
}

int32_t CShmMgr::GetItemSize()
{
	return (int32_t)m_oShmHandle.size();
}

//////////////////////////////////////////////////////////////////////////
// API实现
//////////////////////////////////////////////////////////////////////////

void* CheckExist(int32_t nKey, uint32_t dwSize, int32_t& nRetcode)
{
    SShmItem* pstItem = CShmMgr::Instance()->GetShmItem(nKey);
    if(NULL == pstItem)
    {
        nRetcode = -1;
        return NULL;
    }

    if(dwSize != pstItem->m_dwSize)
    {
		LOG("CheckExist error, size not match(%u, %u)", dwSize, pstItem->m_dwSize);
        nRetcode = 1;
        return NULL;
    }

    pstItem->m_nRef++;

    return pstItem->m_pShm;
}

int ZeaLink_CreateShm(int32_t nKey, uint32_t dwSize, void** ppShmRet)
{
    if(NULL == ppShmRet || 0 == nKey)
    {
        return 0;
    }

    *ppShmRet = NULL;

    if(0 == dwSize)
    {
        return 0;
    }

	void*pVoid      = NULL ;
	int32_t nRetCode = 0;

	//
	// 判断是否已经在本进程存在
	//
	pVoid = CheckExist(nKey, dwSize, nRetCode);
	if(NULL != pVoid)
	{
		*ppShmRet = pVoid;
		return 1;
	}

	if(1 == nRetCode)
	{		
		if (0 == Zealink_DeleteShm(nKey))
		{
			LOG("ZeaLink_CreateShm error : failed to delete unknown share memory");
			return 0;
		}
	}

    int nRet = 1;
    bool bCreater   = true;

#if defined(__GNUC__)
	int32_t nHandle   = shmget((key_t)nKey, dwSize, 0666|IPC_CREAT|IPC_EXCL);
	if(-1 == nHandle)
	{
		if(EEXIST == errno)
		{
			bCreater = false;
			nRet = 1;
		}
		nHandle = shmget((key_t)nKey, dwSize, 0666|IPC_CREAT);
		if(-1 == nHandle)
		{
			if(EINVAL == errno)
			{
				return 0;
			}

			return 0;
		}
	}

	void* pShm = shmat(nHandle, NULL, SHM_R);
	if((void*)-1 == pShm)
	{
		return 0;
	}

#else
	char cszKey[32] = {'\0'};
	sprintf_s(cszKey, "Global\\WMSHM_%d", nKey);

	SECURITY_ATTRIBUTES sa = {0};
	SECURITY_DESCRIPTOR sd = {0};
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE);

	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = FALSE;

	HANDLE nHandle = CreateFileMappingA(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, dwSize, cszKey);
	DWORD dwError = GetLastError();

	if(NULL == nHandle)
	{
		return 0;
	}

	if (ERROR_ALREADY_EXISTS == dwError)
	{
		bCreater = false;
		nRetCode = 1;
	}

	void* pShm = MapViewOfFile(nHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if(NULL == pShm)
	{
		return 0;
	}
#endif

    SShmItem* pstShitem = new SShmItem;
    assert(NULL != pstShitem);

    pstShitem->m_nKey       = nKey;
    pstShitem->m_nRef		= 1 ;
    pstShitem->m_pShm       = pShm;
    pstShitem->m_nHandle    = nHandle;
    pstShitem->m_dwSize     = dwSize;
    pstShitem->m_bCreat   = bCreater;

    CShmMgr::Instance()->AddShmItem(pstShitem);

    *ppShmRet = pShm;

    return nRet;
}

int Zealink_DeleteShm(int32_t nKey)
{
    bool  bRet  = CShmMgr::Instance()->DeleteShm(nKey);
	int32_t nSize = CShmMgr::Instance()->GetItemSize();

    if(0 == nSize)
    {
        CShmMgr::Instance()->Release();
    }

    if(bRet)
    {
        return 1;
    }

    return 0;
}
