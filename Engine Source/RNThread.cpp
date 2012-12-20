//
//  RNThread.cpp
//  Rayne
//
//  Copyright 2012 by Felix Pohl and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNThread.h"
#include "RNMutex.h"
#include "RNArray.h"
#include "RNContext.h"

namespace RN
{
	static Mutex       *__ThreadMutex = 0;
	static ObjectArray *__ThreadArray = 0;
	
#if RN_PLATFORM_POSIX
	pthread_t __ThreadMainThread;
#endif
#if RN_PLATFORM_WINDOWS
	DWORD __ThreadMainThread;
#endif
	
	Thread::Thread(ThreadEntry entry)
	{
		_detached = false;
		_entry = entry;
		_mutex = new Mutex();
		_context = 0;
		
		RN::Assert(_entry != 0 && _mutex != 0);
	}
	
	Thread::Thread()
	{
		_detached = true;
		_entry = 0;
		_mutex = new Mutex();
		_context = 0;
		
		RN::Assert(_mutex != 0);
		
#if RN_PLATFORM_POSIX
		_thread = pthread_self();
#endif
#if RN_PLATFORM_WINDOWS
		_thread = GetCurrentThread();
		_id     = GetCurrentThreadId();
#endif
	}
	
	Thread::~Thread()
	{
		_mutex->Release();
		
		if(_context)
			_context->DeactiveContext();
	}
	
	Thread *Thread::CurrentThread()
	{
		__ThreadMutex->Lock();
		
		for(machine_uint i=0; i<__ThreadArray->Count(); i++)
		{
			Thread *thread = (Thread *)__ThreadArray->ObjectAtIndex(i);
			if(thread->OnThread())
			{
				__ThreadMutex->Unlock();
				return thread;
			}
		}
		
#if RN_PLATFORM_POSIX
		if(pthread_equal(__ThreadMainThread, pthread_self()))
		{
			Thread *thread = new Thread();
			
			__ThreadArray->AddObject(thread);
			__ThreadMutex->Unlock();
			
			return thread;
		}
#endif
#if RN_PLATFORM_WINDOWS
		if(__ThreadMainThread == GetCurrentThreadId())
		{
			Thread *thread = new Thread();
			
			__ThreadArray->AddObject(thread);
			__ThreadMutex->Unlock();
			
			return thread;
		}
#endif
		
		__ThreadMutex->Unlock();
		return 0;
	}
	
	
	void *Thread::Entry(void *object)
	{
		Thread *thread = (Thread *)object;
		
		__ThreadMutex->Lock();
		__ThreadArray->AddObject(thread);
		__ThreadMutex->Unlock();
		
		thread->_entry(thread);
		
		__ThreadMutex->Lock();
		__ThreadArray->RemoveObject(thread);
		__ThreadMutex->Unlock();
		
		thread->Release();
		return NULL;
	}
	
	bool Thread::OnThread() const
	{
		if(!_detached)
			return false;
		
#if RN_PLATFORM_POSIX
		return pthread_equal(_thread, pthread_self());
#endif
#if RN_PLATFORM_WINDOWS
		return (_id == GetCurrentThreadId());
#endif
	}
	
	void Thread::Detach()
	{
		_mutex->Lock();
		
		if(!_detached)
		{
			Retain();
			_detached = true;
		
#if RN_PLATFORM_POSIX
			int result = pthread_create(&_thread, NULL, &Thread::Entry, this);
			if(result)
			{
				Release();
				_detached = false;
			}
#endif
#if RN_PLATFORM_WINDOWS
			_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Thread::Entry, this, 0, &_id);
			if(_thread == NULL)
			{
				Release();
				_detached = false;
			}
#endif
		}
		
		_mutex->Unlock();
	}
	
	void Thread::Join(Thread *other)
	{
		other->_mutex->Lock();
		RN::Assert(other->_detached);
		other->_mutex->Unlock();
		
#if RN_PLATFORM_POSIX
		pthread_join(other->_thread, NULL);
#endif
#if RN_PLATFORM_WINDOWS
		WaitForSingleObject(other->_thread, INFINITE);
#endif
	}
	
	RN_INITIALIZER(__ThreadInitializer)
	{
		__ThreadMutex = new Mutex();
		__ThreadArray = new ObjectArray();
		
#if RN_PLATFORM_POSIX
		__ThreadMainThread = pthread_self();
#endif
#if RN_PLATFORM_WINDOWS
		__ThreadMainThread = GetCurrentThreadId();
#endif
		
		RN::Assert(__ThreadMutex != 0 && __ThreadArray != 0);
	}
}
