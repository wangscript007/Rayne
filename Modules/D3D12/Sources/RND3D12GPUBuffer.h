//
//  RND3D12GPUBuffer.h
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//


#ifndef __RAYNE_D3D12GPUBUFFER_H_
#define __RAYNE_D3D12GPUBUFFER_H_

#include <Rayne.h>

class ID3D12Resource;
namespace RN
{
	class D3D12Renderer;
	class D3D12GPUBuffer : public GPUBuffer
	{
	public:
		friend class D3D12Renderer;

		RNAPI void *GetBuffer() const final;
		RNAPI void InvalidateRange(const Range &range) final;
		RNAPI size_t GetLength() const final;

	private:
		D3D12GPUBuffer(const void *data, size_t length);
		~D3D12GPUBuffer() override;

		ID3D12Resource *_bufferResource;
		size_t _length;

		RNDeclareMeta(D3D12GPUBuffer)
	};
}


#endif /* __RAYNE_D3D12GPUBUFFER_H_ */
