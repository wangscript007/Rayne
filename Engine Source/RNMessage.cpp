//
//  RNMessage.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNMessage.h"

namespace RN
{
	RNDeclareMeta(Message)
	
	Message::Message(String *name, class Object *object, Dictionary *info)
	{
		_name   = name ? name->Retain() : nullptr;
		_object = object ? object->Retain() : nullptr;
		_info   = info ? info->Retain() : nullptr;
	}
	
	Message::~Message()
	{
		if(_name)
			_name->Release();
		
		if(_object)
			_object->Release();
		
		if(_info)
			_info->Release();
	}
	
	
	
	void MessageCenter::PostMessage(Message *message)
	{
		String *name = message->Name();
		machine_hash hash = name->Hash();
		
		_lock.Lock();
		
		for(auto i=_observer.begin(); i!=_observer.end(); i++)
		{
			if(i->name->Hash() == hash && i->name->IsEqual(name))
			{
				i->callback(message);
			}
		}
		
		_lock.Unlock();
	}
	
	void MessageCenter::PostMessage(String *name, Object *object, Dictionary *info)
	{
		Message *message = new Message(name, object, info);
		PostMessage(message);
		message->Release();
	}
	
	
	
	
	void MessageCenter::AddObserver(String *name, CallbackType callback, void *cookie)
	{
		MessageObserverProxy proxy;
		proxy.cookie = cookie;
		proxy.name = name->Retain();
		proxy.callback = std::move(callback);
		
		_lock.Lock();
		_observer.push_back(std::move(proxy));
		_lock.Unlock();
	}
	
	void MessageCenter::RemoveObserver(void *cookie)
	{
		_lock.Lock();
		
		for(auto i=_observer.begin(); i!=_observer.end();)
		{
			if(i->cookie == cookie)
			{
				i = _observer.erase(i);
				continue;
			}
			
			i++;
		}
		
		_lock.Unlock();
	}
	
	void MessageCenter::RemoveObserver(void *cookie, String *name)
	{
		machine_hash hash = name->Hash();
		
		_lock.Lock();
		
		for(auto i=_observer.begin(); i!=_observer.end();)
		{
			if(i->cookie == cookie && i->name->Hash() == hash && i->name->IsEqual(name))
			{
				i = _observer.erase(i);
				continue;
			}
			
			i++;
		}
		
		_lock.Unlock();
	}
}
