//
//  RNCountedSet.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_COUNTEDSET_H__
#define __RAYNE_COUNTEDSET_H__

#include "../Base/RNBase.h"
#include "RNObject.h"

namespace RN
{
	class Array;
	class CountedSetInternal;
	
	class CountedSet : public Object
	{
	public:
		RNAPI CountedSet();
		RNAPI CountedSet(size_t capacity);
		RNAPI CountedSet(const Array *other);
		RNAPI CountedSet(const CountedSet *other);
		RNAPI CountedSet(Deserializer *deserializer);
		RNAPI ~CountedSet() override;
		
		RNAPI void Serialize(Serializer *serializer) const override;
		RNAPI const String *GetDescription() const override;

		RNAPI bool IsEqual(const Object *other) const override;
		RNAPI size_t GetHash() const override;


		RNAPI void AddObject(Object *object);
		RNAPI void RemoveObject(const Object *object);
		RNAPI void RemoveAllObjects();
		RNAPI bool ContainsObject(const Object *object) const;
		
		RNAPI void Enumerate(const std::function<void (Object *object, size_t count, bool &stop)>& callback) const;
		
		template<class T>
		void Enumerate(const std::function<void (T *object, size_t count, bool &stop)>& callback) const
		{
			Enumerate([&](Object *object, size_t count, bool &stop) {
				callback(static_cast<T *>(object), count, stop);
			});
		}
		
		RNAPI Array *GetAllObjects() const;
		
		RNAPI size_t GetCount() const;
		RNAPI size_t GetCountForObject(const Object *object) const;
		
	private:
		PIMPL<CountedSetInternal> _internals;
		
		__RNDeclareMetaInternal(CountedSet)
	};
	
	RNObjectClass(CountedSet)
}


#endif /* __RAYNE_COUNTEDSET_H__ */
