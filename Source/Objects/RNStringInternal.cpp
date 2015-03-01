//
//  RNStringInternal.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include <codecvt>
#include "../Math/RNAlgorithm.h"
#include "RNStringInternal.h"
#include "RNString.h"
#include "RNData.h"

namespace RN
{
	static std::unordered_map<const void *, UTF8String *> _stringTable;
	static SpinLock _stringTableLock;

	UTF8String *StringPool::CreateUTF8String(const void *string)
	{
		LockGuard<SpinLock> lock(_stringTableLock);
		UTF8String *source = _stringTable[string];

		if(!source)
		{
			source = new UTF8String(reinterpret_cast<const uint8 *>(string), kRNNotFound, false);
			_stringTable[string] = source;
		}

		lock.Unlock();

		UTF8String *temp = new UTF8String(source->_constStorage, source->_length, source->_hash, source->_flags);
		return temp;
	}


	RNDefineMeta(UTF8String, Object)

	static const char UTF8TrailingBytes[256] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, 3,3,3,3,3,3,3,3,4,4,4,4,5,5,5,5
	};
	
	static const uint8 UTF8FirstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
	
	RN_INLINE bool IsLegalUTF8(const uint8 *sequence)
	{
		size_t length = UTF8TrailingBytes[*sequence] + 1;
		const uint8 *end = sequence + length;
		uint8 character;
		
		switch(length)
		{
			case 4:
				if((character = (*--end)) < 0x80 || character > 0xBF)
					return false;
			case 3:
				if((character = (*--end)) < 0x80 || character > 0xBF)
					return false;
			case 2:
				if((character = (*--end)) > 0xBF)
					return false;
				
				switch(*sequence)
				{
					case 0xe0:
						if(character < 0xa0)
							return false;
						break;
						
					case 0xed:
						if(character > 0x9f)
							return false;
						break;
						
					case 0xf0:
						if(character < 0x90)
							return false;
						break;
						
					case 0xf4:
						if(character > 0x8f)
							return false;
						break;
						
					default:
						if(character < 0x80)
							return false;
						break;
				}
				
			case 1:
				if(*sequence >= 0x80 && *sequence < 0xc2)
					return false;
				break;
				
			default:
				return false;
		}
		
		if(*sequence > 0xf4)
			return false;
		
		return true;
	}
	
	RN_INLINE size_t UTF8ByteLengthForUnichar(UniChar character)
	{
		if(character < 0x80)
			return 1;
		
		if(character < 0x800)
			return 2;
		
		if(character < 0x10000)
			return 3;
		
		return 4;
	}
	
	RN_INLINE UniChar UTF8ToUnicode(const uint8 *bytes)
	{
		size_t length = UTF8TrailingBytes[*bytes] + 1;
		UniChar result;
		
		switch(length)
		{
			case 1:
				result = *bytes;
				break;
				
			case 2:
				result = (*bytes ^ 0xc0);
				break;
				
			case 3:
				result = (*bytes ^ 0xe0);
				break;
				
			case 4:
				result = (*bytes ^ 0xf0);
				break;
				
			default:
				throw Exception(Exception::Type::InconsistencyException, "");
		}
		
		bytes ++;
		
		for(size_t i = length; i > 1; i--)
		{
			result <<= 6;
			result |= (*bytes ^ 0x80);
			
			bytes ++;
		}
		
		return result;
	}
	
	RN_INLINE void UnicodeToUTF8(UniChar character, uint8 *bytes)
	{
		const UniChar byteMask = 0xbf;
		const UniChar byteMark = 0x80;
		
		size_t toWrite = UTF8ByteLengthForUnichar(character);
		bytes += toWrite;
		
		switch(toWrite)
		{
			case 4:
				*--bytes = static_cast<uint8>(((character | byteMark) & byteMask));
				character >>= 6;
			case 3:
				*--bytes = static_cast<uint8>(((character | byteMark) & byteMask));
				character >>= 6;
			case 2:
				*--bytes = static_cast<uint8>(((character | byteMark) & byteMask));
				character >>= 6;
			case 1:
				*--bytes = static_cast<uint8>((character | UTF8FirstByteMark[toWrite]));
		}
	}
	
	
	RN_INLINE size_t UTF8StringByteLength(const uint8 *data)
	{
		const uint8 *temp = data;
		
		while(*data != '\0')
		{
			size_t trailing = (UTF8TrailingBytes[*data] + 1);
			data += trailing;
		}
		
		return data - temp;
	}
	
	RN_INLINE size_t UTF16StringByteLength(const uint16 *data)
	{
		const uint16 *temp = data;
		
		while(*data != '\0')
		{
			size_t trailing = (UTF8TrailingBytes[*data] + 1);
			data += trailing;
		}
		
		return (data - temp) * 2;
	}
	
	
	
	
	
	UTF8String::UTF8String() :
		_flags(Flags::ASCII | Flags::ConstStorage),
		_constStorage(reinterpret_cast<const uint8 *>("")),
		_length(0)
	{
		RecalcuateHash();
	}

	UTF8String::UTF8String(const uint8 *storage, size_t length, machine_hash hash, Flags flags) :
		_flags(flags),
		_constStorage(storage),
		_length(length),
		_hash(hash)
	{}
	
	UTF8String::UTF8String(const UTF8String *other) :
		_flags(other->_flags),
		_length(other->_length),
		_hash(other->_hash),
		_constStorage(other->_constStorage),
		_storage(other->_storage)
	{}
	
	UTF8String::UTF8String(const uint8 *data, size_t bytes, bool mutableStorage) :
		UTF8String(data, bytes, Encoding::UTF8, mutableStorage)
	{}
	
	UTF8String::UTF8String(const void *data, size_t bytes, Encoding encoding, bool mutableStorage)
	{
		switch(encoding)
		{
			case Encoding::ASCII:
			case Encoding::UTF8:
			{
				const uint8 *temp = reinterpret_cast<const uint8 *>(data);
				
				if(bytes == kRNNotFound)
					bytes = UTF8StringByteLength(temp);
				
				WakeUpWithUTF8String(temp, bytes, mutableStorage);
				break;
			}
				
			case Encoding::UTF16LE:
			{
				if(bytes == kRNNotFound)
					bytes = UTF16StringByteLength(static_cast<const uint16 *>(data));
				
				std::u16string string(static_cast<const char16_t *>(data), (bytes / 2));
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::codecvt_mode::little_endian>, char16_t> converter;
				
				
				std::string bstring = converter.to_bytes(string);
				const char *utf8 = bstring.c_str();
				
				WakeUpWithUTF8String(reinterpret_cast<const uint8 *>(utf8), bstring.length(), true);
				break;
			}
				
			case Encoding::UTF16BE:
			{
				if(bytes == kRNNotFound)
					bytes = UTF16StringByteLength(static_cast<const uint16 *>(data));
				
				std::u16string string(static_cast<const char16_t *>(data), (bytes / 2));
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff>, char16_t> converter;
				
				
				std::string bstring = converter.to_bytes(string);
				const char *utf8 = bstring.c_str();
				
				WakeUpWithUTF8String(reinterpret_cast<const uint8 *>(utf8), bstring.length(), true);
				break;
			}
				
			case Encoding::UTF32:
			{
				const uint32 *utf32 = reinterpret_cast<const uint32 *>(data);
				
				if(bytes == kRNNotFound)
				{
					bytes = 0;
					
					const uint32 *temp = utf32;
					while(*temp ++)
						bytes += 4;
						
				}
				
				
				uint8 *tdata = new uint8[bytes];
				uint8 *temp = tdata;
				
				size_t length = bytes / 4;
				
				for(size_t i = 0; i < length; i ++)
				{
					if(utf32[i] == 0)
						break;
					
					UnicodeToUTF8(utf32[i], temp);
					temp += UTF8ByteLengthForUnichar(utf32[i]);
				}
				
				WakeUpWithUTF8String(tdata, temp - tdata, true);
				
				delete [] tdata;
				break;
			}
		}
	}

	
	void UTF8String::WakeUpWithUTF8String(const uint8 *data, size_t count, bool mutableStorage)
	{
		// Initialize
		_length = 0;
		_flags = Flags::ASCII;
		_constStorage = nullptr;
		
		// Get the storage ready
		if(count >= 0xfff)
			mutableStorage = true;
		
		if(!mutableStorage)
		{
			_constStorage = data;
			_flags |= (static_cast<int>(count) | Flags::ConstStorage);
		}
		else
		{
			_storage.insert(_storage.end(), data, data + count);
		}
		
		// Calculate the length of the string
		const uint8 *bytes = GetBytes();
		const uint8 *end    = bytes + count;
		
		while(bytes != end)
		{
			if(*bytes == '\0')
			{
				count = bytes - GetBytes();
				
				if(_flags & Flags::ConstStorage)
				{
					_flags &= ~Flags::ByteCountMask;
					_flags |= static_cast<int>(count);
				}
				else
				{
					_storage.erase(_storage.begin() + count, _storage.end());
				}
				
				break;
			}
			
			if(*bytes > 0x7F)
				_flags &= ~Flags::ASCII;
			
			bytes += (UTF8TrailingBytes[*bytes] + 1);
			_length ++;
		}
		
		RecalcuateHash();
	}
	
	UTF8String *UTF8String::MutableCopy() const
	{
		return new UTF8String(GetBytes(), GetBytesCount(), true);
	}
	
	void UTF8String::MakeMutable()
	{
		_storage.insert(_storage.end(), _constStorage, _constStorage + GetBytesCount());
		_flags &= ~(Flags::ConstStorage | Flags::ByteCountMask);
	}
	
	bool UTF8String::ValidateUTF8() const
	{
		if(_flags & Flags::ASCII)
			return true;
		
		const uint8 *data = GetBytes();
		
		for(size_t i = 0; i < _length; i ++)
		{
			if(!IsLegalUTF8(data))
				return false;
			
			data += (UTF8TrailingBytes[*data] + 1);
		}
		
		return true;
	}
	
	
	
	const uint8 *UTF8String::GetBytes() const
	{
		return (_flags & Flags::ConstStorage) ? _constStorage : _storage.data();
	}
	
	size_t UTF8String::GetBytesCount() const
	{
		return (_flags & Flags::ConstStorage) ? (_flags & Flags::ByteCountMask) : _storage.size();
	}
	
	void UTF8String::RecalcuateHash()
	{
		_hash = 0;
		
		const uint8 *bytes = GetBytes();
		
		for(size_t i = 0; i < _length; i ++)
		{
			HashCombine(_hash, UTF8ToUnicode(bytes));
			bytes += (UTF8TrailingBytes[*bytes] + 1);
		}
	}
	
	
	
	size_t UTF8String::SkipCharacters(const uint8 *string, size_t skip) const
	{
		if(_flags & Flags::ASCII)
			return skip;
		
		const uint8 *temp = string;
		
		for(size_t i = 0; i < skip; i ++)
			string += (UTF8TrailingBytes[*string] + 1);
		
		return (string - temp);
	}
	
	UniChar UTF8String::GetCharacterAtIndex(size_t index) const
	{
		if(_flags & Flags::ASCII)
			return GetBytes()[index];
		
		const uint8 *data = GetBytes();
		index = SkipCharacters(data, index);
		
		return UTF8ToUnicode(data + index);
	}
	
	void UTF8String::GetCharactersInRange(UniChar *buffer, const Range &range) const
	{
		const uint8 *data = GetBytes();
		
		if(_flags & Flags::ASCII)
		{
			data += range.origin;
			
			for(size_t i = 0; i < range.length; i ++)
				buffer[i] = data[i];
			
			return;
		}
		
		
		for(size_t i = 0; i < range.origin; i ++)
			data += (UTF8TrailingBytes[*data] + 1);
		
		for(size_t i = 0; i < range.length; i ++)
		{
			buffer[i] = UTF8ToUnicode(data);
			data += (UTF8TrailingBytes[*data] + 1);
		}
	}
	
	UTF8String *UTF8String::GetSubstring(const Range &range) const
	{
		if(!IsMutable())
		{
			size_t start = SkipCharacters(_constStorage, range.origin);
			size_t end   = start + SkipCharacters(_constStorage + start, range.length);
			
			UTF8String *substring = new UTF8String(_constStorage + start, end - start, false);
			return substring->Autorelease();
		}
		
		const uint8 *data = GetBytes();
		
		size_t start = SkipCharacters(data, range.origin);
		size_t end   = start + SkipCharacters(data + start, range.length);
		
		UTF8String *substring = new UTF8String(data + start, end - start, true);
		return substring->Autorelease();
	}
	
	
	
	
	void UTF8String::ReplaceCharactersInRange(const Range &range, UTF8String *string)
	{
		if(!IsMutable())
			MakeMutable();
		
		if(string)
		{
			size_t gap    = SkipCharacters(_storage.data(), range.origin);
			size_t gapEnd = gap + SkipCharacters(_storage.data() + gap, range.length);
			
			
			const uint8 *data = string->GetBytes();
			size_t size       = string->GetBytesCount();
			
			_storage.erase(_storage.begin() + gap, _storage.begin() + gapEnd);
			_storage.insert(_storage.begin() + gap, data, data + size);
			
			_length = (_length - range.length) + string->GetLength();
			
			// Validate if we can keep the ASCII flag
			if(_flags & Flags::ASCII)
			{
				for(size_t i = 0; i < size; i ++)
				{
					if(data[i] > 0x7F)
					{
						_flags &= ~Flags::ASCII;
						break;
					}
				}
			}
		}
		else
		{
			size_t gap    = SkipCharacters(_storage.data(), range.origin);
			size_t gapEnd = gap + SkipCharacters(_storage.data() + gap, range.length);
			
			_storage.erase(_storage.begin() + gap, _storage.begin() + gapEnd);
			_length -= range.length;
		}
		
		RecalcuateHash();
	}
	
	
	void *UTF8String::GetBytesWithEncoding(Encoding encoding, bool lossy, size_t &length) const
	{
		char *data = nullptr;
		
		switch(encoding)
		{
			case Encoding::ASCII:
			{
				if(_flags & Flags::ASCII)
				{
					const uint8 *bytes = GetBytes();
					data = new char[_length + 1];
					
					std::copy(bytes, bytes + _length, data);
					data[_length] = '\0';
					
					length = _length;
				}
				else if(lossy)
				{
					const uint8 *bytes = GetBytes();
					data = new char[_length + 1];
					
					for(size_t i = 0; i < _length; i ++)
					{
						size_t trailing = (UTF8TrailingBytes[*bytes] + 1);
						
						data[i] = (trailing > 1) ? '?' : static_cast<char>(*bytes);
						bytes += trailing;
					}
					
					length = _length;
				}
			}
				
			case Encoding::UTF8:
			{
				const uint8 *bytes = GetBytes();
				length = GetBytesCount();
				
				data = new char[length + 1];
				
				std::copy(bytes, bytes + length, data);
				data[length] = '\0';
				break;
			}
				
			case Encoding::UTF16LE:
			{
				const char *start = reinterpret_cast<const char *>(GetBytes());
				const char *end   = start + GetBytesCount();
				
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff, std::codecvt_mode::little_endian>, char16_t> converter;
				std::u16string string = converter.from_bytes(start, end);
				
				data = new char[(string.length() + 1) * 2];
				length = string.length() * 2;
				
				std::copy(string.begin(), string.end(), reinterpret_cast<uint16 *>(data));
				break;
			}
				
			case Encoding::UTF16BE:
			{
				const char *start = reinterpret_cast<const char *>(GetBytes());
				const char *end   = start + GetBytesCount();
				
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t, 0x10ffff>, char16_t> converter;
				std::u16string string = converter.from_bytes(start, end);
				
				data = new char[(string.length() + 1) * 2];
				length = string.length() * 2;
				
				std::copy(string.begin(), string.end(), reinterpret_cast<uint16 *>(data));
				break;
			}
				
			case Encoding::UTF32:
			{
				data = new char[(_length + 1) * 4];
				uint32 *temp = reinterpret_cast<uint32 *>(data);
				
				const uint8 *bytes = GetBytes();
				
				for(size_t i = 0; i < _length; i ++)
				{
					size_t trailing = (UTF8TrailingBytes[*bytes] + 1);
					
					temp[i] = UTF8ToUnicode(bytes);
					bytes += trailing;
				}
				
				temp[_length] = 0;
				length = _length * 4;
				
				break;
			}
		}
		
		if(data)
		{
			Data *temp = new Data(data, length, true, true);
			temp->Autorelease();
			
			return temp->GetBytes();
		}
		
		return nullptr;
	}
}
