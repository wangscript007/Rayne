//
//  TGLoadingScreen.cpp
//  Test Game
//
//  Created by Sidney Just on 08/01/14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "TGLoadingScreen.h"

namespace TG
{
	LoadingScreen::LoadingScreen(RN::Progress *progress) :
		RN::UI::Widget(RN::UI::Widget::StyleBorderless) // No chrome around the widget
	{
		UpdateSize(nullptr);
		const RN::Rect &frame = GetFrame();
		
		
		// Create the UI elements
		_backdrop = new RN::UI::ImageView(RN::UI::Image::WithFile("textures/Raynedrop.png"));
		_backdrop->SetScaleMode(RN::UI::ScaleMode::ProportionallyDown); // We only scale down, not up
		_backdrop->SetAutoresizingMask(0xffffff);
		_backdrop->SetFrame(RN::Rect((frame.width / 2.0) - 512.0f, (frame.height / 2.0) - 512.0f, 1024.0f, 1024.0f));
		
		_progressIndicator = new RN::UI::ProgressIndicator();
		_progressIndicator->SetFrame(RN::Rect((frame.width / 2.0) - 256.0f, frame.height - 20.0f, 512.0f, 10.0f));
		_progressIndicator->SetProgress(progress);
		_progressIndicator->SetAutoresizingMask(RN::UI::View::AutoresizingFlexibleLeftMargin | RN::UI::View::AutoresizingFlexibleRightMargin | RN::UI::View::AutoresizingFlexibleTopMargin);
		
		GetContentView()->AddSubview(_backdrop);
		GetContentView()->AddSubview(_progressIndicator);
		
		GetContentView()->SetBackgroundColor(RN::Color::White());
		
		// Listen to UI Server resizes and the finish of thw world coordinator loading
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNUIServerDidResizeMessage, std::bind(&LoadingScreen::UpdateSize, this, std::placeholders::_1), this);
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNWorldCoordinatorDidFinishLoading, [this](RN::Message *message) {
			Close();
		}, this);
	}
	
	LoadingScreen::~LoadingScreen()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
		
		_backdrop->Release();
		_progressIndicator->Release();
	}
	
	void LoadingScreen::UpdateSize(RN::Message *message)
	{
		RN::UI::Server *server = RN::UI::Server::GetSharedInstance();
		SetFrame(RN::Rect(0.0f, 0.0f, server->GetWidth(), server->GetHeight()));
	}
}