//
//  RNApplication.cpp
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNApplication.h"

namespace RN
{
	Application::Application()
	{
	}
	
	Application::~Application()
	{
	}
	
	
	
	void Application::Start()
	{}
	
	bool Application::CanExit()
	{
		return true;
	}
	
	void Application::WillExit()
	{}
	
	
	
	void Application::GameUpdate(float delta)
	{
	}
	
	void Application::WorldUpdate(float delta)
	{
	}
	
	void Application::SetTitle(const std::string& title)
	{
		_title = title;
	}
}
