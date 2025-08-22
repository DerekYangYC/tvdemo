#ifndef _H_PROFILE_
#define _H_PROFILE_
/***************************************************************************************************
**
** profile.h
**
** Real-Time Hierarchical Profiling for Game Programming Gems 3
**
** by Greg Hjelstrom & Byon Garrabrant
**
***************************************************************************************************/

/*
** A node in the Profile Hierarchy Tree
*/
#include <string>
#include <map>
#include <vector>
#if !defined(__GNUC__)
#include <windows.h>
#endif
#include <time.h>
#include "profile_result.h"

using namespace std;

#define BEGIN_PROFILE   CProfileManager::Begin_Profile()
#define END_PROFILE   CProfileManager::End_Profile()
#define PROFILE(name)   CProfileSample __profile( name )
#define PROFILE_FUNC   CProfileSample __profile( __FUNCTION__)
#define EXPORT_PROFILE(name) CProfileResult::Instance()->ExportToXML(name); \
	CProfileManager::Reset()


#define BEGIN_THREAD_PROFILE   g_pThreadProfileManager->Begin_Profile()
//#define END_THREAD_PROFILE   g_pThreadProfileManager->End_Profile()
#define SET_PROFILE_THREAD_NUM(num) g_pThreadProfileManager->SetThreadNum(num)
#define THREAD_PROFILE(name)   CThreadProfileSample __profile( name )
#define THREAD_PROFILE_FUNC   CThreadProfileSample __profile( __FUNCTION__)
#define EXPORT_THREAD_PROFILE(name) CProfileResult::Instance()->ExportThreadToXML(name); \
	g_pThreadProfileManager->Reset()

// using namespace Common;
// using namespace Common::ProcessInfo;

class	CProfileNode {

public:
	CProfileNode( const char * name, CProfileNode * parent );
	~CProfileNode( void );

	CProfileNode * Get_Sub_Node( const char * name );

	CProfileNode * Get_Parent( void )		{ return Parent; }
	CProfileNode * Get_Sibling( void )		{ return Sibling; }
	CProfileNode * Get_Child( void )			{ return Child; }

	void				Reset( void );
	void				Call( void );
	bool				Return( void );

	string			    Get_Name( void )				{ return Name; }
	int				    Get_Total_Calls( void )		{ return TotalCalls; }
	float				Get_Total_Time( void )		{ return TotalTime; }
	float               Get_Max_Time(void)          { return MaxTime; }

	float               Get_Time_Per_Call();
	float               Get_Percent_of_Parent();
	float               Get_Percent_of_Total();
	int                 Get_Child_Count();

	void				Merge(CProfileNode * pNode);

public:

	string			Name;
	int				TotalCalls;
	float			TotalTime;  // ms
#if !defined(__GNUC__)
	LARGE_INTEGER   StartTime;
#else
	timespec		StartTime;
#endif
	float           MaxTime;    // ms
	int				RecursionCounter;

	CProfileNode *	Parent;
	CProfileNode *	Child;
	CProfileNode *	Sibling;
};

/*
** An iterator to navigate through the tree
*/
class CProfileIterator
{
    friend class CProfileNode;
public:
	// Access all the children of the current parent
	void				First(void);
	void				Next(void);
	bool				Is_Done(void);

	void				Enter_Child( int index );		// Make the given child the new parent
	void				Enter_Largest_Child( void );	// Make the largest child the new parent
	void				Enter_Parent( void );			// Make the current parent's parent the new parent

	// Access the current child
	string				Get_Current_Name( void )			{ return CurrentChild->Get_Name(); }
	int					Get_Current_Total_Calls( void )	{ return CurrentChild->Get_Total_Calls(); }
	float				Get_Current_Total_Time( void )	{ return CurrentChild->Get_Total_Time(); }

	// Access the current parent
	string				Get_Current_Parent_Name( void )			{ return CurrentParent->Get_Name(); }
	int					Get_Current_Parent_Total_Calls( void )	{ return CurrentParent->Get_Total_Calls(); }
	float				Get_Current_Parent_Total_Time( void )	{ return CurrentParent->Get_Total_Time(); }

protected:

	CProfileNode *	CurrentParent;
	CProfileNode *	CurrentChild;

	CProfileIterator( CProfileNode * start );
	friend	class		CProfileManager;
	friend  class       CThreadProfileManager;
	friend	class		CThreadProfileNode;
};


/*
** The Manager for the Profile system
*/
class	CProfileManager {
    friend class CProfileResult;
public:
    static  void                        Begin_Profile();
    static  void                        End_Profile();
	static	void						Start_Profile( const char * name );
	static	void						Stop_Profile( void );

	static	void						Increment_Frame_Counter( void );
	static	int						Get_Frame_Count_Since_Reset( void )		{ return FrameCounter; }
	//static	timespec						Get_Time_Since_Reset( void );
	static	void						Reset( void );

	static	CProfileIterator *	Get_Iterator( void )	{ return new CProfileIterator( &Root ); }
	static	void						Release_Iterator( CProfileIterator * iterator ) { delete iterator; }

private:
	static	CProfileNode			Root;
	static	CProfileNode *			CurrentNode;
	static	int						FrameCounter;
#if !defined(__GNUC__)
	static	__int64				ResetTime;
#else
	static	timespec				ResetTime;
#endif
	static  bool                    ProfileFlag;
};

class	CThreadProfileNode {
	friend class CProfileResult;
public:
	void                        Begin_Profile();
	void                        End_Profile();
	void						Start_Profile(const char * name);
	void						Stop_Profile(void);

	void						Increment_Frame_Counter(void);
	int							Get_Frame_Count_Since_Reset(void) { return FrameCounter; }
	//static	timespec						Get_Time_Since_Reset( void );
	void						Reset(void);

	CProfileIterator *			Get_Iterator(void) { return new CProfileIterator(CurrentNode); }
	void						Release_Iterator(CProfileIterator * iterator) { delete iterator; }

public:
	vector<CProfileNode *>		FinishNodes;
	CProfileNode *				CurrentNode;
	int							FrameCounter;
#if !defined(__GNUC__)
	static	__int64				ResetTime;
#else
	static	timespec			ResetTime;
#endif
	bool						ProfileFlag;
};

class CThreadProfileMananger
{
	friend class CProfileResult;
public:
	CThreadProfileMananger() : Root("Root", NULL)
	{
#if !defined(__GNUC__)
		InitializeCriticalSection(&m_sec);
#endif
		nTotalThreadNum = 10000;
	}
	void Begin_Profile();
	void Stop_Profile(); // 停止所有线程的监控 
	void Start_Profile(const char* name, int nThreadID);
	void Stop_Profile(int nThreadID);

	void SetThreadNum(int num)
	{
		nTotalThreadNum = num;
	}

	void Reset()
	{
		Root.Reset();
		map<int, CThreadProfileNode>::iterator it;
		for (it = ThreadNodes.begin(); it != ThreadNodes.end(); ++it)
		{
			it->second.Reset();
		}
	}

	void GetAllData();

	map<int, CThreadProfileNode> ThreadNodes;
	int nTotalThreadNum;
	CProfileNode Root;

#if !defined(__GNUC__)
	CRITICAL_SECTION m_sec;
#endif
};
/*
** ProfileSampleClass is a simple way to profile a function's scope
** Use the PROFILE macro at the start of scope to time
*/
class	CProfileSample {
public:
	CProfileSample( const char * name )
	{
		CProfileManager::Start_Profile( name );
	}

	~CProfileSample( void )
	{
		CProfileManager::Stop_Profile();
	}
};

class   CThreadProfileSample
{
public:
	CThreadProfileSample(const char* name);
	~CThreadProfileSample();
};

extern CThreadProfileMananger* g_pThreadProfileManager;
#endif

