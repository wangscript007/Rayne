//
//  RNSpinLock.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_SPINLOCK_H__
#define __RAYNE_SPINLOCK_H__

#include "RNDefines.h"

namespace RN
{
	class SpinLock
	{
	public:
#if _MSC_VER <= 1800
		SpinLock()
		{
			_flag.clear();
		}
#else
		SpinLock() :
			_flag(ATOMIC_FLAG_INIT)
		{}
#endif
		
		void Lock()
		{
			while(_flag.test_and_set(std::memory_order_acquire))
			{}
		}
		
		void Unlock()
		{
			_flag.clear(std::memory_order_release);
		}
		
		bool TryLock()
		{
			return (!_flag.test_and_set(std::memory_order_acquire));
		}
		
	private:
		std::atomic_flag _flag;
	};
	
	class Thread;
	class RecursiveSpinLock
	{
	public:
#if _MSC_VER <= 1800
		RecursiveSpinLock() :
			_locks(0)
		{
			_flag.clear();
			assert(_locks.is_lock_free()); // We can't pull RN_ASSERT() in here
		}
#else
		RecursiveSpinLock() :
			_flag(ATOMIC_FLAG_INIT),
			_locks(0)
		{
			assert(_locks.is_lock_free()); // We can't pull RN_ASSERT() in here
		}
#endif
		
		
		RNAPI void Lock();
		RNAPI void Unlock();
		RNAPI bool TryLock();
		
	private:
		std::atomic_flag _flag;
		std::thread::id _owner;
		std::atomic<uint32> _locks;
	};
}

#endif /* __RAYNE_SPINLOCK_H__ */
