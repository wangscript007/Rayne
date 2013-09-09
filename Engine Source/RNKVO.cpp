//
//  RNKVO.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNKVO.h"
#include "RNKVOImplementation.h"
#include "RNDictionary.h"
#include "RNString.h"
#include "RNLockGuard.h"

namespace RN
{
	// ---------------------
	// MARK: -
	// MARK: ObservableBase
	// ---------------------
	
	ObservableBase::ObservableBase(const char *name, ObservableType type) :
		_name(name),
		_observable(nullptr),
		_type(type)
	{
		_writable = true;
	}
	
	ObservableBase::~ObservableBase()
	{
	}
	
	void ObservableBase::WillChangeValue()
	{
		_observable->WillChangeValueForVariable(this);
	}
	
	void ObservableBase::DidChangeValue()
	{
		_observable->DidChangeValueForVariable(this);
	}
	
	
	
	// ---------------------
	// MARK: -
	// MARK: ObservableContainer
	// ---------------------
	
	ObservableContainer::ObservableContainer()
	{
	}
	
	ObservableContainer::~ObservableContainer()
	{
		for(ObservableBase *base : _createdObservers)
			delete base;
	}
	
	ObservableBase *ObservableContainer::GetObservableForKey(const std::string& key) const
	{
		auto iterator = _variables.find(key.c_str());
		if(iterator == _variables.end())
			throw Exception(Exception::Type::InconsistencyException, "No Observable for key " + key);
		
		return iterator->second;
	}
	
	
	
	void ObservableContainer::SetValueForKey(const std::string& key, Object *value)
	{
		ObservableBase *observable = GetObservableForKey(key);
		if(!observable->IsWritable())
			throw Exception(Exception::Type::InconsistencyException, "Can't set value for read-only key");
		
		observable->SetValue(value);
	}

	Object *ObservableContainer::GetValueForKey(const std::string& key) const
	{
		ObservableBase *observable = GetObservableForKey(key);
		return observable->GetValue();
	}
	
	Object *ObservableContainer::GetValueForKeyPath(const std::string& key) const
	{
		return nullptr;
	}
	
	
	
	ObservableBase *ObservableContainer::AddObservable(void *ptr, const char *name, ObservableType type)
	{
		ObservableBase *base = nullptr;
		
#define TypeCase(kvotype, type) \
		case ObservableType::kvotype: \
			base = new __ObservableBase<type>(static_cast<type *>(ptr), name); \
			break;
		
		switch(type)
		{
			TypeCase(Int8, int8)
			TypeCase(Int16, int16)
			TypeCase(Int32, int32)
			TypeCase(Int64, int64)
				
			TypeCase(Uint8, uint8)
			TypeCase(Uint16, uint16)
			TypeCase(Uint32, uint32)
			TypeCase(Uint64, uint64)
				
			TypeCase(Float, float)
			TypeCase(Double, double)
		}
		
#undef TypeCase
		
		if(base)
		{
			_createdObservers.push_back(base);
			AddObservable(base);
		}
		
		return base;
	}
	
	void ObservableContainer::AddObservable(ObservableBase *core)
	{
		_lock.Lock();
		core->_observable = this;
		_variables.insert(std::unordered_map<const char *, ObservableBase *>::value_type(core->_name, core));
		_observerPool.push_back(core);
		_lock.Unlock();
	}
	
	
	std::vector<ObservableContainer::Observer *>& ObservableContainer::GetObserversForKey(const std::string& key)
	{
		auto iterator = _observerMap.find(key);
		if(iterator == _observerMap.end())
		{
			std::vector<Observer *> observer = {};
			_observerMap.insert(std::unordered_map<std::string, std::vector<Observer *>>::value_type(key, observer));
			
			iterator = _observerMap.find(key);
		}
			
		return iterator->second;
	}
	
	void ObservableContainer::AddObserver(const std::string& key, CallbackType callback, void *cookie)
	{
		_lock.Lock();
		
		_observer.emplace_back(Observer(key, callback, cookie));
		Observer& observer = _observer.back();
		
		std::vector<Observer *>& vector = GetObserversForKey(key);
		vector.push_back(&observer);
		
		_lock.Unlock();
	}
	
	void ObservableContainer::RemoveObserver(const std::string& key, void *cookie)
	{
		_lock.Lock();
		
		std::vector<Observer *>& vector = GetObserversForKey(key);
		for(auto i = vector.begin(); i != vector.end();)
		{
			if((*i)->cookie == cookie)
			{
				i = vector.erase(i);
				continue;
			}
			
			i ++;
		}
		
		for(auto i = _observer.begin(); i != _observer.end();)
		{
			if(i->cookie == cookie && i->key == key)
			{
				i = _observer.erase(i);
				continue;
			}
			
			i ++;
		}
		
		_lock.Unlock();
	}
	
	
	
	void ObservableContainer::WillChangeValueForkey(const std::string& key)
	{
		ObservableBase *base = GetObservableForKey(key);
		if(base)
			WillChangeValueForVariable(base);
	}
	
	void ObservableContainer::DidChangeValueForKey(const std::string& key)
	{
		ObservableBase *base = GetObservableForKey(key);
		if(base)
			DidChangeValueForVariable(base);
	}
	
	void ObservableContainer::WillChangeValueForVariable(ObservableBase *core)
	{}
	
	void ObservableContainer::DidChangeValueForVariable(ObservableBase *core)
	{
		LockGuard<SpinLock> lock(_lock);
		
		if(_createdObservers.empty())
			return;
		
		std::vector<Observer *> observers = GetObserversForKey(core->_name);
		
		lock.Unlock();
		
		if(observers.size() > 0)
		{
			std::string key = core->_name;
			Dictionary *changes = new Dictionary();
			
			changes->SetObjectForKey(core->GetValue(), kRNObservableNewValueKey);
			
			for(Observer *observer : observers)
			{
				observer->callback(this, key, changes);
			}
			
			changes->Release();
		}
	}
}