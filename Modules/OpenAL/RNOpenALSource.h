//
//  RNOpenALSource.h
//  Rayne-OpenAL
//
//  Copyright 2017 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#ifndef __RAYNE_OPENALSOURCE_H_
#define __RAYNE_OPENALSOURCE_H_

#include "RNOpenAL.h"

namespace RN
{
	class OpenALWorld;
	class OpenALSource : public SceneNode
	{
	public:
		friend OpenALWorld;
			
		OALAPI OpenALSource(AudioAsset *asset);
		OALAPI ~OpenALSource() override;
			
		OALAPI void Update(float delta) override;

		OALAPI void SetAudioAsset(AudioAsset *asset);
		OALAPI void Play();
		OALAPI void Stop();
		OALAPI void Pause();
		OALAPI void Seek(float time);
		OALAPI void SetRepeat(bool repeat);
		OALAPI void SetPitch(float pitch);
		OALAPI void SetGain(float gain);
		OALAPI void SetRange(float min, float max);
		OALAPI void SetSelfdestruct(bool selfdestruct);
			
		OALAPI bool IsPlaying() { return _isPlaying; }
		OALAPI bool HasEnded() { return _hasEnded; }
		OALAPI bool IsRepeating() { return _isRepeating; }
			
	private:
		OpenALWorld *_owner;
		AudioAsset *_asset;
			
		uint32 _source;
		Vector3 _oldPosition;
			
		bool _isPlaying;
		bool _isRepeating;
		bool _isSelfdestructing;
		bool _hasEnded;
		
		size_t _currentBuffer;
		uint32 _ringBuffersID[3];
		int16 *_ringBufferTemp;
			
		RNDeclareMetaAPI(OpenALSource, OALAPI)
	};
}

#endif /* defined(__RAYNE_OPENALSOURCE_H_) */
