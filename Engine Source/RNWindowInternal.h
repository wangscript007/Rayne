//
//  RNWindowInternal.h
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_WINDOWINTERNAL_H__
#define __RAYNE_WINDOWINTERNAL_H__

#include "RNBaseInternal.h"
#include "RNContext.h"

#if RN_PLATFORM_MAC_OS

@interface RNNativeWindow : NSWindow <NSWindowDelegate>
{
	NSOpenGLView *_openGLView;
}

- (void)setOpenGLContext:(NSOpenGLContext *)context andPixelFormat:(NSOpenGLPixelFormat *)pixelFormat;
- (id)initWithFrame:(NSRect)frame andStyleMask:(NSUInteger)stylemask;

@end

#endif

namespace RN
{
	struct WindowInternals
	{
#if RN_PLATFORM_MAC_OS
		RNNativeWindow *nativeWindow;
#endif
		
#if RN_PLATFORM_IOS
		UIWindow *nativeWindow;
		UIViewController *rootViewController;
		UIView *renderingView;
#endif
		
#if RN_PLATFORM_LINUX
		Display *dpy;
		XID win;
		XRRScreenConfiguration *screenConfig;
		SizeID originalSize;
		Rotation originalRotation;
		Cursor emptyCursor;
#endif
		
#if RN_PLATFORM_WINDOWS
		HWND hWnd;
		HDC hDC;
		bool displayChanged;
		Context *context;
#endif
	};

	LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
}

#endif /* __RAYNE_WINDOWINTERNAL_H__ */
