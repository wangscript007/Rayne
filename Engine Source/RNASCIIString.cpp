//
//  RNASCIIString.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNASCIIString.h"
#include "RNData.h"

#define kMutableASCIIStringIncreaseSize 32

namespace RN
{
	RNDeclareMeta(ConstantASCIIString)
	RNDeclareMeta(ASCIIString)
	
	ConstantASCIIString::ConstantASCIIString()
	{}
	
	ConstantASCIIString::ConstantASCIIString(const char *string)
	{
		_string = const_cast<char *>(string);
		_length = strlen(string);
	}
	
	UniChar ConstantASCIIString::CharacterAtIndex(size_t index) const
	{
		if(index > _length)
			throw Exception(Exception::Type::RangeException, "Index must be within string bounds");
		
		return static_cast<UniChar>(_string[index]);
	}
	
	void ConstantASCIIString::CharactersInRange(UniChar *buffer, const Range& range) const
	{
		if(range.End() > _length)
			throw Exception(Exception::Type::RangeException, "");
		
		for(size_t i=0; i<range.length; i++)
		{
			buffer[i] = static_cast<UniChar>(_string[i + range.origin]);
		}
	}
	
	size_t ConstantASCIIString::Length() const
	{
		return _length;
	}
	
	void *ConstantASCIIString::Data() const
	{
		return _string;
	}
	
	void *ConstantASCIIString::BytesWithEncoding(Encoding encoding, bool lossy, size_t *length) const
	{
		char *data = nullptr;
		size_t size;
		
		switch(encoding)
		{
			case Encoding::ASCII:
			case Encoding::UTF8:
				size = _length;
				data = new char[_length + 1];
				
				data[_length] = '\0';
				std::copy(_string, _string + _length, data);
				break;
				
			default:
				throw Exception(Exception::Type::GenericException, "");
		}
		
		if(data)
		{
			if(length)
				*length = size;
			
			RN::Data *temp = new RN::Data(data, size, true, true);
			temp->Autorelease();
			
			return temp->Bytes();
		}
		
		return nullptr;
	}
	
	bool ConstantASCIIString::IsMutable() const
	{
		return false;
	}
	
	Encoding ConstantASCIIString::CharacterEncoding() const
	{
		return Encoding::ASCII;
	}
	
	BasicString *ConstantASCIIString::Substring(const Range& range) const
	{
		return new ASCIIString(&_string[range.origin], range.length);
	}
	
	
	
	ASCIIString::ASCIIString(const char *string)
	{
		_length = strlen(string);
		_size   = _length;
		_string = new char[_length + 1];
		
		strcpy(_string, string);
	}
	
	ASCIIString::ASCIIString(const char *string, size_t length)
	{
		_length = length;
		_size   = length;
		
		_string = new char[_length + 1];
		_string[_length] = '\0';
		
		strcpy(_string, string);
	}
	
	ASCIIString::~ASCIIString()
	{
		delete [] _string;
	}
	
	void ASCIIString::AllocateSpace(size_t size)
	{
		if(size > _size)
		{
			size = size + kMutableASCIIStringIncreaseSize;
			
			char *temp = new char[size + 1];
			temp[_length] = '\0';
			
			std::copy(_string, _string + _length, temp);
			delete [] _string;
			
			_size   = size;
			_string = temp;
		}
	}
	
	void ASCIIString::ReplaceCharactersInRange(const Range& range, BasicString *string)
	{
		if(string)
		{
			if(string->Length() != range.length)
			{
				size_t size = (_length - range.length) + string->Length();
				AllocateSpace(size);
				
				std::copy(_string + range.origin + range.length, _string + _length, _string + range.origin + string->Length());
				
				_length = size;
			}
			
			char *data = static_cast<char *>(string->Data());
			std::copy(data, data + string->Length(), _string + range.origin);
		}
		else
		{
			size_t size = _length - range.length;
			
			_string[size] = '\0';
			_length = size;
			
			std::copy(_string + range.origin + range.length, _string + _length, _string + range.origin);
		}
	}
	
	bool ASCIIString::IsMutable() const
	{
		return true;
	}
}
