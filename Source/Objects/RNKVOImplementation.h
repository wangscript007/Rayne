//
//  RNKVOImplementation.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_KVOIMPLEMENTATION__
#define __RAYNE_KVOIMPLEMENTATION__

#include "../Base/RNBase.h"
#include "RNKVO.h"
#include "RNObject.h"
#include "RNNumber.h"
#include "RNValue.h"

namespace RN
{
	template<class T, class Target, class GetterType = T, class SetterType = T>
	class ObservableScalar;

	template<class T, class Target, class GetterType = const T &, class SetterType = const T &>
	class ObservableValue;

	template<class T, class Target, class GetterType = T, class SetterType = T>
	class ObservableObject;

#define __ObservableScalar(type, kvotype) \
	template<class Target, class GetterType, class SetterType> \
	class ObservableScalar<type, Target, GetterType, SetterType> : public ObservableProperty \
	{ \
	public: \
		using Setter = void (Target::*)(SetterType); \
		using Getter = GetterType (Target::*)() const; \
		ObservableScalar(const char *name, Getter getter = nullptr, Setter setter = nullptr) : \
			ObservableProperty(name, TypeTranslator<type>::value), \
			_setter(setter), \
			_getter(getter) \
		{} \
		ObservableScalar(const char *name, const type &initial, Getter getter = nullptr, Setter setter = nullptr) : \
			ObservableScalar(name, getter, setter) \
		{ \
			_storage = initial; \
		} \
		void SetValue(Object *value) override \
		{ \
			RN_ASSERT(value->IsKindOfClass(Number::GetMetaClass()), ""); \
			Number *number = static_cast<Number *>(value); \
			WillChangeValue(); \
			if(_setter) \
			{ \
				(static_cast<Target *>(_owner)->*_setter)(number->Get##kvotype##Value()); \
			} \
			else \
			{ \
				_storage = number->Get##kvotype##Value(); \
			} \
			DidChangeValue();  \
		} \
		Object *GetValue() const override \
		{ \
			return _getter ? Number::With##kvotype ((static_cast<Target *>(_owner)->*_getter)()) : Number::With##kvotype (_storage); \
		} \
		bool operator == (const type &other) const \
		{ \
			return (_storage == other); \
		} \
		bool operator != (const type &other) const \
		{ \
			return (_storage != other); \
		} \
		\
		operator type& () \
		{ \
			return _storage; \
		} \
		operator type() const \
		{ \
			return _storage; \
		} \
		\
		type &operator= (const type &other) \
		{ \
			_storage = other; \
			 \
			return _storage; \
		} \
		\
		type &operator+= (const type &other) \
		{ \
			_storage += other; \
			 \
			return _storage; \
		} \
		type &operator-= (const type &other) \
		{ \
			_storage -= other; \
			 \
			return _storage; \
		} \
		type &operator*= (const type &other) \
		{ \
			_storage *= other; \
			 \
			return _storage; \
		} \
		type &operator/= (const type &other) \
		{ \
			_storage /= other; \
			 \
			return _storage; \
		} \
		\
		type operator+ (const type &other) const \
		{ \
			return _storage + other; \
		} \
		type operator- (const type &other) const \
		{ \
			return _storage - other; \
		} \
		type operator* (const type &other) const \
		{ \
			return _storage * other; \
		} \
		type operator/ (const type &other) const \
		{ \
			return _storage / other; \
		} \
		\
		type &operator ++() \
		{ \
			++ _storage; \
			 \
			return _storage; \
		} \
		type operator ++(int) \
		{ \
			type result = _storage; \
			\
			++ _storage; \
			\
			return result; \
		} \
		type &operator --() \
		{ \
			-- _storage; \
			 \
			return _storage; \
		} \
		type operator --(int) \
		{ \
			type result = _storage; \
			\
			-- _storage; \
			\
			return result; \
		} \
	private: \
		Setter _setter; \
		Getter _getter; \
		type _storage; \
	};

	
#define __ObservableValueBegin(type) \
	template<class Target, class GetterType, class SetterType> \
	class ObservableValue<type, Target, GetterType, SetterType> : public ObservableProperty \
	{ \
	public: \
		using Setter = void (Target::*)(SetterType); \
		using Getter = GetterType (Target::*)() const; \
		ObservableValue(const char *name, Getter getter = nullptr, Setter setter = nullptr) : \
			ObservableProperty(name, TypeTranslator<type>::value), \
			_setter(setter), \
			_getter(getter) \
		{} \
		ObservableValue(const char *name, const type &initial, Getter getter = nullptr, Setter setter = nullptr) : \
			ObservableValue(name, getter, setter) \
		{ \
			_storage = initial; \
		} \
		ObservableValue(const char *name, type &&initial, Getter getter = nullptr, Setter setter = nullptr) : \
			ObservableValue(name, getter, setter) \
		{ \
			_storage = std::move(initial); \
		} \
		\
		void SetValue(Object *tvalue) override \
		{ \
			RN_ASSERT(tvalue->IsKindOfClass(Value::GetMetaClass()), ""); \
			Value *value = static_cast<Value *>(tvalue); \
			WillChangeValue(); \
			if(_setter) \
			{ \
				(static_cast<Target *>(_owner)->*_setter)(value->GetValue<type>()); \
			} \
			else \
			{ \
				_storage = value->GetValue<type>(); \
			} \
			DidChangeValue();  \
		} \
		Object *GetValue() const override \
		{ \
			if(_getter) \
				return Value::With##type ((static_cast<Target *>(_owner)->*_getter)()); \
			return Value::With##type (_storage); \
		} \
		operator const type& () const \
		{ \
			return _storage; \
		} \
		type &operator= (const type &other) \
		{ \
			_storage = other; \
			\
			return _storage; \
		} \
	private: \
		Setter _setter; \
		Getter _getter; \
		type _storage; \
	public:
	

#define __ObservableValueComparison(type) \
	bool operator == (const type &other) const \
	{ \
		return (_storage == other); \
	} \
	bool operator != (const type &other) const \
	{ \
		return (_storage != other); \
	} \

#define __ObservableValuePointerLikes(type) \
	type operator *() \
	{ \
		return _storage; \
	} \
	const type &operator *() const \
	{ \
		return _storage; \
	} \
	type *operator ->() \
	{ \
		return &_storage; \
	} \
	const type *operator ->() const \
	{ \
		return &_storage; \
	}

#define __ObservableValueBinaryArithmeticAddition(type) \
	type &operator+= (const type &other) \
	{ \
		_storage += other; \
		\
		return _storage; \
	} \
	type operator+ (const type &other) const \
	{ \
		return _storage + other; \
	}

#define __ObservableValueBinaryArithmeticSubtraction(type) \
	type &operator-= (const type &other) \
	{ \
		_storage -= other; \
		\
		return _storage; \
	} \
	type operator- (const type &other) const \
	{ \
		return _storage - other; \
	}

#define __ObservableValueBinaryArithmeticMultiplication(type) \
	type &operator*= (const type &other) \
	{ \
		_storage *= other; \
		\
		return _storage; \
	} \
	type operator* (const type &other) const \
	{ \
		return _storage * other; \
	}

#define __ObservableValueBinaryArithmeticDivision(type) \
	type &operator/= (const type &other) \
	{ \
		_storage /= other; \
		\
		return _storage; \
	} \
	type operator/ (const type &other) const \
	{ \
		return _storage / other; \
	}
	
#define __ObservableValueBinaryArithmetic(type) \
	__ObservableValueBinaryArithmeticAddition(type) \
	__ObservableValueBinaryArithmeticSubtraction(type) \
	__ObservableValueBinaryArithmeticMultiplication(type) \
	__ObservableValueBinaryArithmeticDivision(type)
	

#define __ObservableValueEnd() \
	};
	
	template<class T, class Target, class GetterType, class SetterType>
	class ObservableObject<T *, Target, GetterType, SetterType> : public ObservableProperty
	{
	public:
		using Setter = void (Target::*)(SetterType);
		using Getter = GetterType (Target::*)() const;

		ObservableObject(const char *name, Object::MemoryPolicy policy, Getter getter = nullptr, Setter setter = nullptr) :
			ObservableProperty(name, TypeTranslator<Object *>::value),
			_setter(setter),
			_getter(getter),
			_policy(policy),
			_storage(nullptr),
			_meta(T::GetMetaClass())
		{}
		
		void SetValue(Object *object) override
		{
			RN_DEBUG_ASSERT(IsWritable(), "Observable must be writable");

			WillChangeValue();
			
			if(!_setter)
			{
				switch(_policy)
				{
					case Object::MemoryPolicy::Assign:
						_storage = static_cast<T *>(object);
						break;
						
					case Object::MemoryPolicy::Retain:
						SafeRelease(_storage);
						_storage = static_cast<T *>(SafeRetain(object));
						break;
						
					case Object::MemoryPolicy::Copy:
						SafeRelease(_storage);
						_storage = static_cast<T *>(SafeCopy(object));
						break;
				}
			}
			else
			{
				(static_cast<Target *>(_owner)->*_setter)(static_cast<T *>(object));
			}
		
			DidChangeValue();
		}
	
		Object *GetValue() const override
		{
			if(_getter)
				return (static_cast<Target *>(_owner)->*_getter)();
				
			return _storage;
		}

		MetaClass *GetMetaClass() const override
		{
			return _meta;
		}

		bool operator== (T *other)
		{
			if(!_storage)
				return (other == nullptr);
			
			return (_storage->IsEqual(other));
		}

		bool operator!= (T *other)
		{
			if(!_storage)
				return (other != nullptr);
			
			return !(_storage->IsEqual(other));
		}

		operator bool()
		{
			return (_storage != nullptr);
		}

		operator T* () const
		{
			return _storage;
		}

		T *operator= (T *other)
		{
			_storage = other;
			
			return _storage;
		}

		T *operator ->() const
		{
			return _storage;
		}

	private:
		Setter _setter;
		Getter _getter;
		Object::MemoryPolicy _policy;
		T *_storage;
		MetaClass *_meta;
	};


	template<class Target, class GetterType, class SetterType>
	class ObservableScalar<bool, Target, GetterType, SetterType> : public ObservableProperty
	{
	public:
		using Setter = void (Target::*)(SetterType);
		using Getter = GetterType (Target::*)() const;

		ObservableScalar(const char *name, Getter getter = nullptr, Setter setter = nullptr) :
			ObservableProperty(name, TypeTranslator<bool>::value),
			_setter(setter),
			_getter(getter)
		{}
		ObservableScalar(const char *name, bool initial, Getter getter = nullptr, Setter setter = nullptr) :
			ObservableScalar(name, getter, setter)
		{
			_storage = initial;
		}
		
		void SetValue(Object *value) override
		{
			RN_ASSERT(value->IsKindOfClass(Number::GetMetaClass()), "Value must be a of type RN::Number");
			RN_DEBUG_ASSERT(IsWritable(), "Observable must be writable");
			
			Number *number = static_cast<Number *>(value);
			WillChangeValue();
			if(_setter)
			{
				(static_cast<Target *>(_owner)->*_setter)(number->GetBoolValue());
			}
			else
			{
				_storage = number->GetBoolValue();
			}
			DidChangeValue();
		}
		Object *GetValue() const override
		{
			return _getter ? Number::WithBool((static_cast<Target *>(_owner)->*_getter)()) : Number::WithBool(_storage);
		}
		
		bool operator== (const bool other)
		{
			return (_storage == other);
		}

		bool operator!= (const bool other)
		{
			return (_storage != other);
		}

		bool &operator= (const bool &other)
		{
			_storage = other;
			
			return _storage;
		}

		operator bool()
		{
			return _storage;
		}

		operator bool() const
		{
			return _storage;
		}
	private:
		Setter _setter;
		Getter _getter;
		bool _storage;
	};

	__ObservableScalar(int8, Int8)
	__ObservableScalar(int16, Int16)
	__ObservableScalar(int32, Int32)
	__ObservableScalar(int64, Int64)
	
	__ObservableScalar(uint8, Uint8)
	__ObservableScalar(uint16, Uint16)
	__ObservableScalar(uint32, Uint32)
	__ObservableScalar(uint64, Uint64)
	
	__ObservableScalar(float, Float)
	__ObservableScalar(double, Double)
	
	__ObservableValueBegin(Vector2)
	__ObservableValueComparison(Vector2)
	__ObservableValueBinaryArithmetic(Vector2)
	__ObservableValuePointerLikes(Vector2)
	__ObservableValueEnd()

	__ObservableValueBegin(Vector3)
	__ObservableValueComparison(Vector3)
	__ObservableValueBinaryArithmetic(Vector3)
	__ObservableValuePointerLikes(Vector3)
	__ObservableValueEnd()
	
	__ObservableValueBegin(Vector4)
	__ObservableValueComparison(Vector4)
	__ObservableValueBinaryArithmetic(Vector4)
	__ObservableValuePointerLikes(Vector4)
	__ObservableValueEnd()
	
	__ObservableValueBegin(Color)
	__ObservableValueComparison(Color)
	__ObservableValueBinaryArithmetic(Color)
	__ObservableValuePointerLikes(Color)
	__ObservableValueEnd()
	
	__ObservableValueBegin(Matrix)
	__ObservableValueBinaryArithmeticMultiplication(Matrix)
	__ObservableValuePointerLikes(Matrix)
	__ObservableValueEnd()
	
	__ObservableValueBegin(Quaternion)
	__ObservableValueBinaryArithmetic(Quaternion)
	__ObservableValuePointerLikes(Quaternion)
	__ObservableValueEnd()
}

#endif /* __RAYNE_KVOIMPLEMENTATION */
