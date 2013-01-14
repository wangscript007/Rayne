//
//  RNContext.h
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_CONTEXT_H__
#define __RAYNE_CONTEXT_H__

#include "RNBase.h"
#include "RNObject.h"
#include "RNThread.h"
#include "RNRenderingResource.h"

namespace RN
{
	class Window;
	class Context : public Object, public RenderingResource
	{
	friend class Window;
	public:
		RNAPI Context(Context *shared=0);		
		RNAPI virtual ~Context();
		
		RNAPI void MakeActiveContext();
		RNAPI void DeactivateContext();
		
		RNAPI virtual void Flush();
		RNAPI virtual void SetName(const char *name);
		
		static Context *ActiveContext();
		
	protected:
		void Activate();
		void Deactivate();
		
	private:
		bool _active;
		Thread *_thread;
		Context *_shared;
		
#if RN_PLATFORM_MAC_OS
		NSOpenGLContext *_oglContext;
		NSOpenGLPixelFormat *_oglPixelFormat;
		CGLContextObj _cglContext;
#endif
		
#if RN_PLATFORM_IOS
		EAGLContext *_oglContext;
#endif

#if RN_PLATFORM_WINDOWS
		PIXELFORMATDESCRIPTOR _oglPixelformat;
		HGLRC _oglContext;
#endif
	};
}

#endif /* __RAYNE_CONTEXT_H__ */
