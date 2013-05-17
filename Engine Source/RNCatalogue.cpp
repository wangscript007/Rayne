//
//  RNCatalogue.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNCatalogue.h"

namespace RN
{
	MetaClass::MetaClass(MetaClass *parent, const std::string& name, const char *namespaceBlob) :
		_name(name),
		_superClass(parent)
	{
		Catalogue::ParsePrettyFunction(namespaceBlob, _namespace);
		
		_namespace.pop_back();
		_namespace.pop_back();
		
		Catalogue::SharedInstance()->AddMetaClass(this);
	}
	
	MetaClass::~MetaClass()
	{
		Catalogue::SharedInstance()->RemoveMetaClass(this);
	}
	
	bool MetaClass::InheritsFromClass(MetaClass *other) const
	{
		if(this == other)
			return true;
		
		if(!_superClass)
			return false;
		
		return _superClass->InheritsFromClass(other);
	}
	
	std::string MetaClass::Fullname() const
	{
		std::string name;
		
		for(auto i=_namespace.begin(); i!=_namespace.end(); i++)
		{
			name += *i;
			name += "::";
		}
		
		name += _name;
		return name;
	}
	
	
	
	MetaClass *Catalogue::__ClassWithName(const std::string& name, const char *namespaceBlob) const
	{
		auto iterator = _metaClasses.find(name);
		if(iterator != _metaClasses.end())
			return iterator->second;
		
		std::vector<std::string> namespaces;
		std::vector<std::string> names;
		
		ParsePrettyFunction(namespaceBlob, namespaces);
		
		for(size_t i=namespaces.size(); i>0; i--)
		{
			std::string temp;
			
			for(size_t j=0; j<i; j++)
			{
				temp += namespaces[j];
				temp += "::";
			}
			
			temp += name;
			names.push_back(std::move(temp));
		}
		
		for(auto i=names.begin(); i!=names.end(); i++)
		{
			auto iterator = _metaClasses.find(*i);
			
			if(iterator != _metaClasses.end())
				return iterator->second;
		}
		
		return 0;
		
		
	}
	
	void Catalogue::AddMetaClass(MetaClass *meta)
	{
		auto iterator = _metaClasses.find(meta->Fullname());
		if(iterator != _metaClasses.end())
			throw ErrorException(0, 0, 0);
		
		_metaClasses.insert(std::unordered_map<std::string, MetaClass *>::value_type(meta->Fullname(), meta));
	}
	
	void Catalogue::RemoveMetaClass(MetaClass *meta)
	{
		_metaClasses.erase(meta->Fullname());
	}
	
	void Catalogue::ParsePrettyFunction(const char *string, std::vector<std::string>& namespaces)
	{
		const char *namespaceEnd = string;
		const char *namespaceBegin = 0;
		
		const char *signature = strpbrk(string, "(");
		
		while(1)
		{
			const char *temp = strstr(namespaceEnd, "::");
			if(!temp || (signature && temp >= signature))
				break;
			
			namespaceEnd = temp + 2;
		}
		
		namespaceEnd -= 2;
		namespaceBegin = namespaceEnd;
		
		while(namespaceBegin > string)
		{
			if(isalnum(*(namespaceBegin - 1)) || *(namespaceBegin - 1) == ':')
			{
				namespaceBegin --;
				continue;
			}
			
			break;
		}
		
		while(namespaceBegin < namespaceEnd)
		{
			const char *temp = strstr(namespaceBegin, "::");
			namespaces.emplace_back(std::string(namespaceBegin, temp - namespaceBegin));
			
			namespaceBegin = temp + 2;
		}
	}
}
