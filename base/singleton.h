#pragma once

#include <stdlib.h>

#if defined(__GNUC__)
#include <pthread.h>
#endif

template<typename T>
class Singleton
{
public:
	static T& Instance()
	{
#if defined(__GNUC__)
		pthread_once(&ponce_, &Singleton::init);
#endif

		if (NULL == value_)
		{
			value_ = new T();
		}
		return *value_;
	}

private:
	Singleton();
	~Singleton();

	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

	static void init()
	{
		value_ = new T();
		::atexit(destroy);
	}

	static void destroy()
	{
		delete value_;
	}

private:
#if defined(__GNUC__)
	static pthread_once_t ponce_;
#endif
	static T*             value_;
};

#if defined(__GNUC__)
template<typename T>
pthread_once_t Singleton<T>::ponce_ = PTHREAD_ONCE_INIT;
#endif

template<typename T>
T* Singleton<T>::value_ = NULL;