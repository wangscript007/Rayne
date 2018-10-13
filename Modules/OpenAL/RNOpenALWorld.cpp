//
//  RNOpenALWorld.cpp
//  Rayne-OpenAL
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNOpenALWorld.h"

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"

//static LPALCGETSTRINGISOFT alcGetStringiSOFT;

namespace RN
{
	RNDefineMeta(OpenALWorld, SceneAttachment)
		
	OpenALWorld::OpenALWorld(String *deviceName) :
		_audioListener(nullptr)
	{
		if(deviceName)
			_device = alcOpenDevice(deviceName->GetUTF8String());
		else
			_device = alcOpenDevice(nullptr);
		if(!_device)
		{
			RNDebug("rayne-openal: Could not open audio device.");
			return;
		}

/*		if(!alcIsExtensionPresent(_device, "ALC_SOFT_HRTF"))
		{
			RNDebug("Error: ALC_SOFT_HRTF not supported");
			return;
		}

		int num_hrtf = 0;
		alcGetIntegerv(_device, ALC_NUM_HRTF_SPECIFIERS_SOFT, 1, &num_hrtf);
		if(!num_hrtf)
			RNDebug("No HRTFs found");
		else
		{
			RNDebug("Available HRTFs:");
			for(int i = 0; i < num_hrtf; i++)
			{
				const ALCchar *name = alcGetStringiSOFT(_device, ALC_HRTF_SPECIFIER_SOFT, i);
				RNDebug("    " << i << ": " << name);
			}
		}*/

		//Enable HRTF
		int attributes[3] = {ALC_HRTF_SOFT, ALC_TRUE, 0};
			
		_context = alcCreateContext(_device, attributes);
		alcMakeContextCurrent(_context);
		if(!_context)
		{
			RNDebug("rayne-openal: Could not create audio context.");
			return;
		}

		int hrtf_state = 0;
		alcGetIntegerv(_device, ALC_HRTF_SOFT, 1, &hrtf_state);
		if(!hrtf_state)
			RNDebug("HRTF not enabled!\n");
		else
		{
			const ALchar *name = alcGetString(_device, ALC_HRTF_SPECIFIER_SOFT);
			RNDebug("HRTF enabled, using " << name);
		}
	}
		
	OpenALWorld::~OpenALWorld()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(_context);
		alcCloseDevice(_device);
	}

	Array *OpenALWorld::GetDeviceNames()
	{
		const char *bytes = static_cast<const char*>(alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER));
		Array *devices = new Array();
		String *deviceString = String::WithString(bytes, true);
		while(deviceString->GetLength() > 0)
		{
			devices->AddObject(deviceString);
			bytes += deviceString->GetLength() + 1;
			deviceString = String::WithString(bytes, true);
		}
		
		return devices;
	}
		
	void OpenALWorld::Update(float delta)
	{
			
	}
		
	void OpenALWorld::SetListener(OpenALListener *attachment)
	{
		if(_audioListener)
			_audioListener->RemoveFromWorld();
			
		_audioListener = attachment;
			
		if(_audioListener)
			_audioListener->InsertIntoWorld(this);
	}
		
	OpenALSource *OpenALWorld::PlaySound(AudioAsset *resource)
	{
		if(_audioListener)
		{
			OpenALSource *source = new OpenALSource(resource);
			_audioListener->GetParent()->AddChild(source);
			source->SetSelfdestruct(true);
			source->Play();
			return source;
		}
		return nullptr;
	}
}
