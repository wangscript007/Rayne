//
//  RNContext.cpp
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNContext.h"
#include "RNMutex.h"

#if RN_PLATFORM_WINDOWS
extern void RNRegisterWindow();
#endif

namespace RN
{
	Context::Context(Context *shared) :
		RenderingResource("Context")
	{
		_active = false;
		_thread = 0;
		_shared = shared;
		_shared->Retain();
		
#if RN_PLATFORM_MAC_OS
		static NSOpenGLPixelFormatAttribute formatAttributes[] =
		{
			NSOpenGLPFAMinimumPolicy,
			NSOpenGLPFAAccelerated,
			NSOpenGLPFADoubleBuffer,
			NSOpenGLPFAColorSize, 24,
			NSOpenGLPFAAlphaSize, 8,
			NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
			0
		};
		
		_oglPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:formatAttributes];
		if(!_oglPixelFormat)
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsNoHardware);
		
		_oglContext = [[NSOpenGLContext alloc] initWithFormat:_oglPixelFormat shareContext:_shared ? _shared->_oglContext : nil];
		if(!_oglContext)
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsContextFailed);
		
		_cglContext = (CGLContextObj)[_oglContext CGLContextObj];
		
		if(_shared)
		{
			// Enable the multithreaded OpenGL Engine
			
			CGLEnable(_shared->_cglContext, kCGLCEMPEngine);
			CGLEnable(_cglContext, kCGLCEMPEngine);
			
			if(_shared->_active && shared->_thread->OnThread())
			{
				_shared->Deactivate();
				_shared->Activate();
			}
		}
		
#elif RN_PLATFORM_IOS

		EAGLSharegroup *sharegroup = _shared ? [_shared->_oglContext sharegroup] : nil;
		
		_oglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2 sharegroup:sharegroup];
		if(!_oglContext)
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsContextFailed);

#elif RN_PLATFORM_WINDOWS
		RNRegisterWindow();

		_hWnd = CreateOffscreenWindow();
		_hDC  = GetDC(_hWnd);

		PIXELFORMATDESCRIPTOR descriptor;
		memset(&descriptor, 0, sizeof(PIXELFORMATDESCRIPTOR));

		descriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		descriptor.nVersion = 1;
		descriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		descriptor.iPixelType = PFD_TYPE_RGBA;
		descriptor.cColorBits = 24;
		descriptor.cAlphaBits = 8;
		descriptor.iLayerType = PFD_MAIN_PLANE;

		_pixelFormat = ChoosePixelFormat(_hDC, &descriptor);
		if(!_pixelFormat)
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsNoHardware);

		SetPixelFormat(_hDC, _pixelFormat, &descriptor);

		if(!wglCreateContextAttribsARB)
		{
			// Create a temporary pointer to get the right function pointer

			HGLRC _temp = wglCreateContext(_hDC);
			wglMakeCurrent(_hDC, _temp);

			wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
			if(!wglGetExtensionsStringARB)
				throw ErrorException(kErrorGroupGraphics, 0, kGraphicsNoHardware);

			std::string extensions = std::string((const char *)wglGetExtensionsStringARB(_hDC));

			auto createContext = extensions.find("WGL_ARB_create_context");
			auto coreProfile = extensions.find("WGL_ARB_create_context_profile");

			if(createContext == std::string::npos || coreProfile == std::string::npos)
				throw ErrorException(kErrorGroupGraphics, 0, kGraphicsNoHardware);

			wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");

			wglMakeCurrent(_hDC, 0);
			wglDeleteContext(_temp);
		}

		
		int attributes[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 2,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			0
		};

		_context = wglCreateContextAttribsARB(_hDC, 0, attributes);
		if(!_context)
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsContextFailed);

		wglMakeCurrent(_hDC, _context);

		ShowWindow(_hWnd, SW_SHOW);
		UpdateWindow(_hWnd);
#else
		throw ErrorException(kErrorGroupGraphics, 0, kGraphicsContextFailed);
#endif
	}
	
	Context::~Context()
	{
		if(_shared)
			_shared->Release();
		
#if RN_PLATFORM_MAC_OS
		[_oglContext release];
		[_oglPixelFormat release];
#endif
		
#if RN_PLATFORM_IOS
		[_oglContext release];
#endif
	}

#if RN_PLATFORM_WINDOWS
	HWND Context::CreateOffscreenWindow()
	{
		DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;

		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
		RECT windowRect;

		windowRect.left = 0;
		windowRect.right = 1024;
		windowRect.top = 0;
		windowRect.bottom = 768;

		AdjustWindowRectEx(&windowRect, dwStyle, false, dwExStyle);

		HWND hWnd = CreateWindowEx(0, (LPCWSTR)"RNWindowClass", (LPCWSTR)"Rayne", dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 0, 0, hInstance, 0);
		return hWnd;
	}
#endif
	
	void Context::MakeActiveContext()
	{
		Thread *thread = Thread::CurrentThread();
		RN_ASSERT0(thread);
		
		thread->_mutex->Lock();
		
		if(thread->_context == this)
		{
			thread->_mutex->Unlock();
			return;
		}
		
		if(thread->_context)
		{
			Context *other = thread->_context;
			other->_active = false;
			other->_thread = 0;
			
			other->Flush();
			other->Deactivate();
		}
		
		this->Activate();
		
		this->_active = true;
		this->_thread = thread;
		
		thread->_context = this;
		thread->_mutex->Unlock();
	}
	
	void Context::DeactivateContext()
	{
		Thread *thread = Thread::CurrentThread();
		RN_ASSERT0(thread);
		
		thread->_mutex->Lock();
		
		this->_active = false;
		this->_thread = 0;
		
		this->Deactivate();
		
		thread->_context = 0;
		thread->_mutex->Unlock();
	}
	
	Context *Context::ActiveContext()
	{
		Thread *thread = Thread::CurrentThread();
		RN_ASSERT0(thread);
		
		return thread->_context;
	}
	
	void Context::SetName(const char *name)
	{
		RenderingResource::SetName(name);
		
#if RN_PLATFORM_IOS
		if([_oglContext respondsToSelector:@selector(setDebugLabel:)])
			[_oglContext setDebugLabel:[NSString stringWithUTF8String:name]];
#endif
	}
	
	void Context::Flush()
	{
#if RN_PLATFORM_MAC_OS
		CGLFlushDrawable(_cglContext);
#endif

#if RN_PLATFORM_WINDOWS
		SwapBuffers(_hDC);
#endif
	}
	
	void Context::Activate()
	{
#if RN_PLATFORM_MAC_OS
		CGLLockContext(_cglContext);
		[_oglContext makeCurrentContext];
		
		RN_ASSERT0([NSOpenGLContext currentContext] == _oglContext);		
#endif
		
#if RN_PLATFORM_IOS
		BOOL result = [EAGLContext setCurrentContext:_oglContext];
		RN_ASSERT0(result);
#endif

#if RN_PLATFORM_WINDOWS
		wglMakeCurrent(_hDC, _context);
#endif
	}
	
	void Context::Deactivate()
	{
#if RN_PLATFORM_MAC_OS
		CGLUnlockContext(_cglContext);
#endif
	}
}
