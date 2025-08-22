/***************************************************************************************************
**
** profile.cpp
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

#include "profile.h"
#include <iostream>

#if !defined(__GNUC__)

inline float Profile_Get_Tick_Rate(void)
{
	static float _CPUFrequency = -1.0f;

	if (_CPUFrequency == -1.0f) {
		__int64 curr_rate = 0;
		::QueryPerformanceFrequency ((LARGE_INTEGER *)&curr_rate);
		_CPUFrequency = (float)curr_rate;
	} 

	return _CPUFrequency;
}

#else

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if((end.tv_nsec-start.tv_nsec)<0)
	{
		temp.tv_sec = end.tv_sec - start.tv_sec - 1;
		temp.tv_nsec = 1000*1000*1000+end.tv_nsec-start.tv_nsec;
	}
	else
	{
		temp.tv_sec = end.tv_sec - start.tv_sec;
		temp.tv_nsec = end.tv_nsec - start.tv_nsec;
	}

	return temp;
}

#endif

/***************************************************************************************************
**
** CProfileNode
**
***************************************************************************************************/

/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - pointer to a static string which is the name of this profile node                    *
 * parent - parent pointer                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The name is assumed to be a static pointer, only the pointer is stored and compared for     *
 * efficiency reasons.                                                                         *
 *=============================================================================================*/
CProfileNode::CProfileNode( const char * name, CProfileNode * parent ) :
	Name( name ),
	TotalCalls( 0 ),
	TotalTime( 0 ),
	MaxTime(0),
	RecursionCounter( 0 ),
	Parent( parent ),
	Child( NULL ),
	Sibling( NULL )
{
	Reset();
#if defined(__GNUC__)
	clock_gettime(CLOCK_REALTIME, &StartTime);
#endif
}


CProfileNode::~CProfileNode( void )
{
	delete Child;
	delete Sibling;
}


/***********************************************************************************************
 * INPUT:                                                                                      *
 * name - static string pointer to the name of the node we are searching for                   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * All profile names are assumed to be static strings so this function uses pointer compares   *
 * to find the named node.                                                                     *
 *=============================================================================================*/
CProfileNode * CProfileNode::Get_Sub_Node( const char * name )
{
	// Try to find this sub node
	CProfileNode * child = Child;
	while ( child ) {
		if ( child->Name == name ) {
			return child;
		}
		child = child->Sibling;
	}

	// We didn't find it, so add it
	CProfileNode * node = new CProfileNode( name, this );
	node->Sibling = Child;
	Child = node;
	return node;
}


void	CProfileNode::Reset( void )
{
	TotalCalls = 0;
	TotalTime = 0.0f;
	MaxTime = 0;
	RecursionCounter = 0;

	if ( Child ) {
		Child->Reset();
	}
	if ( Sibling ) {
		Sibling->Reset();
	}

	Child = NULL;
	Sibling = NULL;
}


void	CProfileNode::Call( void )
{
	TotalCalls++;
	if (RecursionCounter++ == 0) {
#if defined(__GNUC__)
		clock_gettime(CLOCK_REALTIME, &StartTime); 
#else
		::QueryPerformanceCounter(&StartTime);
#endif
	}
}


bool	CProfileNode::Return( void )
{
	if ( --RecursionCounter == 0 && TotalCalls != 0 ) {
#if !defined(__GNUC__)
		LARGE_INTEGER time;
		::QueryPerformanceCounter(&time);
		float t = (float)(time.QuadPart - StartTime.QuadPart);
		t = 1000*t / Profile_Get_Tick_Rate();
#else
		timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
		timespec temp;
		temp = diff(StartTime, time);
		float t = (temp.tv_sec * 1000 * 1000 * 1000 + temp.tv_nsec) /(1000.0f * 1000.0f);
#endif
		TotalTime += t;

		if(t > MaxTime)
		{
		    MaxTime = t;
		}
	}
	return ( RecursionCounter == 0 );
}

float   CProfileNode::Get_Time_Per_Call()
{
    float fTime = 0.0f;
    if(Get_Total_Calls() > 0)
    {
        fTime = Get_Total_Time() / Get_Total_Calls();
    }
    return fTime;
}

float   CProfileNode::Get_Percent_of_Parent()
{
    float fPer = 100.0f;
    CProfileNode* pNode = Get_Parent();
    if(pNode)
    {
        if(pNode->Get_Total_Time() > 1)
        {
            fPer = 100 * (Get_Total_Time() / pNode->Get_Total_Time());
        }
    }
    return fPer;
}

float   CProfileNode::Get_Percent_of_Total()
{
    float fPer = 100.0f;

    CProfileIterator *it = CProfileManager::Get_Iterator();
    it->First();
    CProfileNode* pNode = it->CurrentChild;
    if(pNode)
    {
        cout << "pNode->Get_Total_Time() = " << pNode->Get_Total_Time() << endl;
        cout << "Get_Total_Time() = " << Get_Total_Time() << endl;
        if(pNode->Get_Total_Time() > 1)
        {
            fPer = 100 * (Get_Total_Time() / pNode->Get_Total_Time());
        }
        cout << "fPer = " << fPer << endl;
    }
    return fPer;
}

int CProfileNode::Get_Child_Count()
{
    int iCount = 0;

    CProfileNode* child = Child;
    while(child)
    {
        iCount++;
        child = child->Sibling;
    }
    return iCount;
}

void  CProfileNode::Merge(CProfileNode * pNode)
{
	if (pNode == NULL)
	{
		return;
	}
	TotalCalls += pNode->TotalCalls;
	TotalTime += pNode->TotalTime;
	if (pNode->MaxTime > MaxTime)
	{
		MaxTime = pNode->MaxTime;
	}

	CProfileNode* child = pNode->Child;
	while (child)
	{
		CProfileNode* sibling = child->Sibling;
		CProfileNode* myChild = Child;
		bool bFound = false;
		while (myChild)
		{
			if (myChild->Name == child->Name)
			{
				myChild->Merge(child);
				bFound = true;
				break;
			}
		}
		if (bFound == false)
		{
			child->Sibling = Child;
			Child = child;
		}
		child = sibling;
	}
}

/***************************************************************************************************
**
** CProfileIterator
**
***************************************************************************************************/
CProfileIterator::CProfileIterator( CProfileNode * start )
{
	CurrentParent = start;
	CurrentChild = CurrentParent->Get_Child();
}


void	CProfileIterator::First(void)
{
	CurrentChild = CurrentParent->Get_Child();
}


void	CProfileIterator::Next(void)
{
	CurrentChild = CurrentChild->Get_Sibling();
}


bool	CProfileIterator::Is_Done(void)
{
	return CurrentChild == NULL;
}


void	CProfileIterator::Enter_Child( int index )
{
	CurrentChild = CurrentParent->Get_Child();
	while ( (CurrentChild != NULL) && (index != 0) ) {
		index--;
		CurrentChild = CurrentChild->Get_Sibling();
	}

	if ( CurrentChild != NULL ) {
		CurrentParent = CurrentChild;
		CurrentChild = CurrentParent->Get_Child();
	}
}


void	CProfileIterator::Enter_Parent( void )
{
	if ( CurrentParent->Get_Parent() != NULL ) {
		CurrentParent = CurrentParent->Get_Parent();
	}
	CurrentChild = CurrentParent->Get_Child();
}


/***************************************************************************************************
**
** CProfileManager
**
***************************************************************************************************/

CProfileNode	CProfileManager::Root( "Root", NULL );
CProfileNode *	CProfileManager::CurrentNode = &CProfileManager::Root;
int				CProfileManager::FrameCounter = 0;
#if !defined(__GNUC__)
__int64			CProfileManager::ResetTime;
#else
timespec		CProfileManager::ResetTime;
#endif
bool            CProfileManager::ProfileFlag = false;


/***********************************************************************************************
 * CProfileManager::Start_Profile -- Begin a named profile                                    *
 *                                                                                             *
 * Steps one level deeper into the tree, if a child already exists with the specified name     *
 * then it accumulates the profiling; otherwise a new child node is added to the profile tree. *
 *                                                                                             *
 * INPUT:                                                                                      *
 * name - name of this profiling record                                                        *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * The string used is assumed to be a static string; pointer compares are used throughout      *
 * the profiling code for efficiency.                                                          *
 *=============================================================================================*/

void    CProfileManager::Begin_Profile()
{
    ProfileFlag = true;
}

void    CProfileManager::End_Profile()
{
    ProfileFlag = false;
}

void	CProfileManager::Start_Profile( const char * name )
{
    if(!ProfileFlag)
    {
        return;
    }

	if (name != CurrentNode->Get_Name()) {
		CurrentNode = CurrentNode->Get_Sub_Node( name );
	}

	CurrentNode->Call();
}


/***********************************************************************************************
 * CProfileManager::Stop_Profile -- Stop timing and record the results.                       *
 *=============================================================================================*/
void	CProfileManager::Stop_Profile( void )
{
	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentNode->Return()) {
		CurrentNode = CurrentNode->Get_Parent();
	}
}


/***********************************************************************************************
 * CProfileManager::Reset -- Reset the contents of the profiling system                       *
 *                                                                                             *
 *    This resets everything except for the tree structure.  All of the timing data is reset.  *
 *=============================================================================================*/
void	CProfileManager::Reset( void )
{
	Root.Reset();
	FrameCounter = 0;
	//clock_gettime(CLOCK_REALTIME, &ResetTime);
}


/***********************************************************************************************
 * CProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
 *=============================================================================================*/
void CProfileManager::Increment_Frame_Counter( void )
{
	FrameCounter++;
}


/***********************************************************************************************
 * CProfileManager::Get_Time_Since_Reset -- returns the elapsed time since last reset         *
 *=============================================================================================*/
//float CProfileManager::Get_Time_Since_Reset( void )
//{
//#ifdef WIN32
//	__int64 time;
//	Profile_Get_Ticks(&time);
//	time -= ResetTime;
//	return (float)time / Profile_Get_Tick_Rate();
//#else
//	timespec time;
//	clock_gettime(CLOCK_REALTIME, &time);
//	return diff(ResetTime, time);
//#endif
//}

void    CThreadProfileNode::Begin_Profile()
{
	ProfileFlag = true;
}

void    CThreadProfileNode::End_Profile()
{
	//ProfileFlag = false;

	if (CurrentNode == NULL)
	{
		return;
	}

	while (CurrentNode != NULL)
	{
		if (CurrentNode->Return()) {
			if (CurrentNode->Get_Parent() == NULL)
			{
				bool bFound = false;
				for (size_t i = 0; i < FinishNodes.size(); ++i)
				{
					if (FinishNodes[i]->Get_Name() == CurrentNode->Get_Name())
					{
						FinishNodes[i]->Merge(CurrentNode);
						bFound = true;
						break;
					}
				}

				if (bFound == false)
				{
					FinishNodes.push_back(CurrentNode);
				}

				CurrentNode = NULL;
				return;
			}

			CurrentNode = CurrentNode->Get_Parent();
		}
	}	
}

void	CThreadProfileNode::Start_Profile(const char * name)
{
	if (!ProfileFlag)
	{
		return;
	}

	if (CurrentNode == NULL)
	{
		CurrentNode = new CProfileNode(name, NULL);
	}
	else if (name != CurrentNode->Get_Name()) {
		CurrentNode = CurrentNode->Get_Sub_Node(name);
	}

	CurrentNode->Call();
}


/***********************************************************************************************
* CProfileManager::Stop_Profile -- Stop timing and record the results.                       *
*=============================================================================================*/
void	CThreadProfileNode::Stop_Profile(void)
{
	// Return will indicate whether we should back up to our parent (we may
	// be profiling a recursive function)
	if (CurrentNode == NULL)
	{
		return;
	}

	if (CurrentNode->Return()) {
		if (CurrentNode->Get_Parent() == NULL)
		{
			bool bFound = false;
			for (size_t i = 0; i < FinishNodes.size(); ++i)
			{
				if (FinishNodes[i]->Get_Name() == CurrentNode->Get_Name())
				{
					FinishNodes[i]->Merge(CurrentNode);
					bFound = true;
					break;
				}
			}

			if (bFound == false)
			{
				FinishNodes.push_back(CurrentNode);
			}

			CurrentNode = NULL;
			return;
		}

		CurrentNode = CurrentNode->Get_Parent();		
	}
}


/***********************************************************************************************
* CProfileManager::Reset -- Reset the contents of the profiling system                       *
*                                                                                             *
*    This resets everything except for the tree structure.  All of the timing data is reset.  *
*=============================================================================================*/
void	CThreadProfileNode::Reset(void)
{
	FrameCounter = 0;
	FinishNodes.clear();
	//clock_gettime(CLOCK_REALTIME, &ResetTime);
}


/***********************************************************************************************
* CProfileManager::Increment_Frame_Counter -- Increment the frame counter                    *
*=============================================================================================*/
void CThreadProfileNode::Increment_Frame_Counter(void)
{
	FrameCounter++;
}



void  CThreadProfileMananger::Begin_Profile()
{
	int nThreadID = 0;

#if !defined(__GNUC__)
	EnterCriticalSection(&m_sec);
	nThreadID = GetCurrentThreadId();
#endif
	if (ThreadNodes.size() >= nTotalThreadNum)
	{
		return;
	}

	ThreadNodes[nThreadID].Begin_Profile();

#if !defined(__GNUC__)
	LeaveCriticalSection(&m_sec);
#endif
}

void CThreadProfileMananger::Start_Profile(const char* name, int nThreadID)
{
	if (ThreadNodes.size() < nTotalThreadNum)
	{
		return;
	}
	if (ThreadNodes.find(nThreadID) == ThreadNodes.end())
	{
		return;
	}

	ThreadNodes[nThreadID].Start_Profile(name);
}

void CThreadProfileMananger::Stop_Profile(int nThreadID)
{
	if (ThreadNodes.size() < nTotalThreadNum)
	{
		return;
	}
	if (ThreadNodes.find(nThreadID) == ThreadNodes.end())
	{
		return;
	}

	ThreadNodes[nThreadID].Stop_Profile();
}

void CThreadProfileMananger::Stop_Profile()
{
	map<int, CThreadProfileNode>::iterator it;
	for (it = ThreadNodes.begin(); it != ThreadNodes.end(); ++it)
	{
		it->second.End_Profile();
	}
}

void CThreadProfileMananger::GetAllData()
{
	map<int, CThreadProfileNode>::iterator it;
	for (it = ThreadNodes.begin(); it != ThreadNodes.end(); ++it)
	{
		vector<CProfileNode *>		&FinishNodes = it->second.FinishNodes;
		for (size_t i = 0; i < FinishNodes.size(); ++i)
		{
			if (FinishNodes[i] == NULL)
			{
				continue;
			}

			//CProfileNode* pNode = new CProfileNode(*FinishNodes[i]);

			CProfileNode* child = Root.Get_Child();
			bool bFound = false;
			while (child)
			{
				if (child->Get_Name() == FinishNodes[i]->Get_Name())
				{
					child->Merge(FinishNodes[i]);
					bFound = true;
					break;
				}

				
				child = child->Get_Sibling();
			}

			if (bFound == false)
			{
				FinishNodes[i]->Sibling = Root.Get_Child();
				Root.Child = FinishNodes[i];
				Root.TotalTime += FinishNodes[i]->TotalTime;
				FinishNodes[i]->Parent = &Root;
			}
		
		}
		it->second.FinishNodes.clear();
	}
}

CThreadProfileMananger* g_pThreadProfileManager = new CThreadProfileMananger;

CThreadProfileSample::CThreadProfileSample(const char* name)
{
	int nThreadID = 0;

#if !defined(__GNUC__)
	nThreadID = GetCurrentThreadId();
#endif
	g_pThreadProfileManager->Start_Profile(name, nThreadID);
}

CThreadProfileSample::~CThreadProfileSample()
{
	int nThreadID = 0;

#if !defined(__GNUC__)
	nThreadID = GetCurrentThreadId();
#endif
	g_pThreadProfileManager->Stop_Profile(nThreadID);
}