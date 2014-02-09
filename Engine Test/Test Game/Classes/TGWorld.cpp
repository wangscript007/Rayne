//
//  TGWorld.cpp
//  Game
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "TGWorld.h"

#define TGWorldFeatureBloom	1
#define TGWorldFeatureSSAO  0

namespace TG
{
	World::World() :
		RN::World("OctreeSceneManager"),
		_exposure(1.0f),
		_whitepoint(5.0f),
		_frameCapturing(false),
		_camera(nullptr),
		_refractCamera(nullptr),
		_sunLight(nullptr)
	{
		SetReleaseSceneNodesOnDestruction(true);
	}
	
	World::~World()
	{
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void World::HandleInputEvent(RN::Event *event)
	{
		if(event->GetType() == RN::Event::Type::KeyDown)
		{
			switch(event->GetCharacter())
			{
				case 'c':
					ToggleFrameCapturing();
					break;
					
				case 'p':
				{
					RN::Vector3 position = _camera->GetWorldPosition();
					RN::Vector3 euler    = _camera->GetWorldEulerAngle();
					
					RNDebug("{%f, %f, %f}", position.x, position.y, position.z);
					RNDebug("{%f, %f, %f}", euler.x, euler.y, euler.z);
					break;
				}
					
				default:
					break;
			}
		}
	}
	
	void World::ToggleFrameCapturing()
	{
		if(!_frameCapturing)
		{
			_frameCapturing = true;
			_frameCount     = 0;
			
			RN::Kernel::GetSharedInstance()->SetFixedDelta(1.0f / 30.0f);
			RecordFrame();
		}
		else
		{
			_frameCapturing = false;
			RN::Kernel::GetSharedInstance()->SetFixedDelta(0.0f);
		}
	}
	
	void World::RecordFrame()
	{
		if(!_frameCapturing)
			return;
		
		RN::Renderer::GetSharedInstance()->RequestFrameCapture([=](RN::FrameCapture *capture) {
			
			RN::Data *data = capture->GetData(RN::FrameCapture::Format::RGBA8888);
			std::stringstream file;
			
			std::string path = RN::FileManager::GetSharedInstance()->GetNormalizedPathFromFullpath("~/Desktop");
			
			file << path << "/Capture/Capture_" << _frameCount << ".bmp";
			std::string base = RN::PathManager::Basepath(file.str());
			
			if(!RN::PathManager::PathExists(base))
				RN::PathManager::CreatePath(base);
			
			
			uint8 fileHeader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
			uint8 infoHeader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
			
			size_t w = capture->GetWidth();
			size_t h = capture->GetHeight();
			
			size_t padding   = (4 - w % 4) % 4;
			size_t dataSize  = w * h * 3 + (h * padding);
			size_t totalSize = dataSize + 14 + 40;
			
			fileHeader[2] = (totalSize) & 0xff;
			fileHeader[3] = (totalSize >> 8) & 0xff;
			fileHeader[4] = (totalSize >> 16) & 0xff;
			fileHeader[5] = (totalSize >> 24) & 0xff;
			
			infoHeader[ 4] = (w) & 0xff;
			infoHeader[ 5] = (w >> 8) & 0xff;
			infoHeader[ 6] = (w >> 16) & 0xff;
			infoHeader[ 7] = (w >> 24) & 0xff;
			infoHeader[ 8] = (h) & 0xff;
			infoHeader[ 9] = (h >> 8) & 0xff;
			infoHeader[10] = (h >> 16) & 0xff;
			infoHeader[11] = (h >> 24) & 0xff;
			
			RN::Data *bmpdata = new RN::Data();
			bmpdata->Append(fileHeader, 14);
			bmpdata->Append(infoHeader, 40);
			
			uint8 *pixelData = reinterpret_cast<uint8 *>(data->GetBytes());
			uint8 *row = new uint8[capture->GetWidth() * 3];
			uint8 pad[3] = { 0 };
			
			for(size_t i = 0; i < capture->GetHeight(); i ++)
			{
				uint32 *pixel = reinterpret_cast<uint32 *>(pixelData + (w * 4) * ((h - i) - 1));
				uint8  *temp  = row;
				
				for(size_t j = 0; j < capture->GetWidth(); j ++)
				{
					*temp ++ = ((*pixel) >> 16) & 0xff;
					*temp ++ = ((*pixel) >> 8) & 0xff;
					*temp ++ = ((*pixel) >> 0) & 0xff;
					
					pixel ++;
				}
				
				bmpdata->Append(row, capture->GetWidth() * 3);
				bmpdata->Append(pad, padding);
			}
			
			bmpdata->WriteToFile(file.str());
			bmpdata->Release();
		});
		
		_frameCount ++;
	}
	
	void World::Update(float delta)
	{
		RecordFrame();
		RN::Input *input = RN::Input::GetSharedInstance();

		RN::Vector3 translation;
		RN::Vector3 rotation;
		
		const RN::Vector2& mouseDelta = input->GetMouseDelta();
		
		if(!(input->GetModifierKeys() & RN::KeyModifier::KeyControl))
		{
			rotation.x = mouseDelta.x;
			rotation.y = mouseDelta.y;
		}
		
		translation.x = (input->IsKeyPressed('d') - input->IsKeyPressed('a')) * 16.0f;
		translation.z = (input->IsKeyPressed('s') - input->IsKeyPressed('w')) * 16.0f;
		
		translation *= (input->GetModifierKeys() & RN::KeyModifier::KeyShift) ? 2.0f : 1.0f;
		
		_camera->Rotate(rotation);
		_camera->TranslateLocal(translation * delta);
		
		
		_exposure   += (input->IsKeyPressed('u') - input->IsKeyPressed('j')) * delta*2.0f;
		_whitepoint += (input->IsKeyPressed('i') - input->IsKeyPressed('k')) * delta;
		
		_whitepoint = std::min(std::max(0.01f, _whitepoint), 10.0f);
		_exposure   = std::min(std::max(0.01f, _exposure), 10.0f);
		
		RN::Renderer::GetSharedInstance()->SetHDRExposure(_exposure);
		RN::Renderer::GetSharedInstance()->SetHDRWhitePoint(_whitepoint);
		
		
		if(_sunLight)
		{
			RN::Color color = _sunLight->GetAmbientColor();
			RN::Color ambient(0.127f, 0.252f, 0.393f, 1.0f);
			
			_camera->SetAmbientColor(color * (ambient * 5.0f));
			_camera->SetFogColor(_sunLight->GetFogColor());
			_camera->GetChildren()->GetObjectAtIndex<RN::Camera>(0)->SetAmbientColor(_camera->GetAmbientColor());
			_camera->GetChildren()->GetObjectAtIndex<RN::Camera>(0)->SetFogColor(_sunLight->GetFogColor());
		}
	}
	
	void World::CreateCameras()
	{
		RN::Model *sky = RN::Model::WithSkyCube("textures/sky_up.png", "textures/sky_down.png", "textures/sky_left.png", "textures/sky_right.png", "textures/sky_front.png", "textures/sky_back.png", "shader/rn_Sky2");
		for(int i = 0; i < 6; i++)
		{
			sky->GetMaterialAtIndex(0, i)->SetAmbientColor(RN::Color(6.0f, 6.0f, 6.0f, 1.0f));
			sky->GetMaterialAtIndex(0, i)->SetOverride(RN::Material::Override::Shader);
			sky->GetMaterialAtIndex(0, i)->Define("RN_ATMOSPHERE");
		}
		
		RN::RenderStorage *storage = new RN::RenderStorage(RN::RenderStorage::BufferFormatComplete);
		storage->AddRenderTarget(RN::Texture::Format::RGB16F);
		storage->SetDepthTarget(RN::Texture::Format::Depth24I);
		
		_camera = new RN::Camera(RN::Vector2(), storage, RN::Camera::Flags::Defaults);
		_camera->SetFlags(_camera->GetFlags() | RN::Camera::Flags::NoFlush);
		_camera->SetRenderGroups(_camera->GetRenderGroups() | RN::Camera::RenderGroups::Group1 | RN::Camera::RenderGroups::Group3);
		_camera->SetSky(sky);
		
#if TGWorldFeatureSSAO
		PPActivateSSAO(_camera);
#endif
		
		RN::PostProcessingPipeline *waterPipeline = _camera->AddPostProcessingPipeline("water", 0);
		
		RN::Material *refractCopy = new RN::Material(RN::Shader::WithFile("shader/rn_PPCopy"));
		refractCopy->Define("RN_COPYDEPTH");
		_refractCamera = new RN::Camera(_camera->GetFrame().Size(), RN::Texture::Format::RGBA32F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		_refractCamera->SetMaterial(refractCopy);
		waterPipeline->AddStage(_refractCamera, RN::RenderStage::Mode::ReUsePreviousStage);
		
		RN::Camera *waterStage = new RN::Camera(RN::Vector2(_camera->GetFrame().Size()), storage, RN::Camera::Flags::Defaults);
		waterStage->SetClearMask(0);
		waterStage->SetRenderGroups(RN::Camera::RenderGroups::Group2 | RN::Camera::RenderGroups::Group3);
		//waterPipeline->AddStage(waterStage, RN::RenderStage::Mode::ReRender);
		
		waterStage->SetBlitShader(RN::Shader::WithFile("shader/rn_DrawFramebufferTonemap"));
		waterStage->SetPriority(-100);
		
		_camera->AddChild(waterStage);
		
#if TGWorldFeatureBloom
		PPActivateBloom(waterStage);
#endif
		
		PPActivateFXAA(waterStage);
	}
	
	void World::PPActivateSSAO(RN::Camera *cam)
	{
		RN::Shader *combineShader = RN::Shader::WithFile("shader/rn_PPCombine");
		RN::Shader *blurShader = RN::Shader::WithFile("shader/rn_BoxBlur");
		RN::Shader *updownShader = RN::Shader::WithFile("shader/rn_PPCopy");
		
		RN::Material *blurXMaterial = new RN::Material(blurShader);
		blurXMaterial->Define("RN_BLURX");
		
		RN::Material *blurYMaterial = new RN::Material(blurShader);
		blurYMaterial->Define("RN_BLURY");
		
		RN::Material *downMaterial = new RN::Material(updownShader);
		downMaterial->Define("RN_DOWNSAMPLE");
		
		// Surface normals
		RN::Shader *surfaceShader = RN::Shader::WithFile("shader/rn_SurfaceNormals");
		RN::Material *surfaceMaterial = new RN::Material(surfaceShader);
		
		RN::Camera *normalsCamera = new RN::Camera(RN::Vector2(), RN::Texture::Format::RGB16F, RN::Camera::Flags::Inherit | RN::Camera::Flags::NoSky | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatComplete);
		normalsCamera->SetMaterial(surfaceMaterial);
		//normalsCamera->GetStorage()->SetDepthTarget(_depthtex);
		//normalsCamera->SetClearMask(RN::Camera::ClearFlagColor);
		
		// SSAO stage
		RN::Texture *ssaoNoise = RN::Texture::WithFile("textures/rn_SSAONoise.png");
		
		RN::Shader *ssaoShader = RN::Shader::WithFile("shader/rn_SSAO");
		RN::Material *ssaoMaterial = new RN::Material(ssaoShader);
		ssaoMaterial->AddTexture(ssaoNoise);
		
		RN::Camera *ssaoCamera  = new RN::Camera(RN::Vector2(), RN::Texture::Format::R8, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		ssaoCamera->SetMaterial(ssaoMaterial);
		
		// Blur X
		RN::Camera *ssaoBlurX = new RN::Camera(RN::Vector2(), RN::Texture::Format::R8, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		ssaoBlurX->SetMaterial(blurXMaterial);
		
		// Blur Y
		RN::Camera *ssaoBlurY = new RN::Camera(RN::Vector2(), RN::Texture::Format::R8, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		ssaoBlurY->SetMaterial(blurYMaterial);
		
		// Combine stage
		RN::Material *ssaoCombineMaterial = new RN::Material(combineShader);
		ssaoCombineMaterial->AddTexture(ssaoBlurY->GetStorage()->GetRenderTarget());
		ssaoCombineMaterial->Define("MODE_GRAYSCALE");
		
		RN::Camera *ssaoCombineCamera  = new RN::Camera(RN::Vector2(), RN::Texture::Format::RGB16F, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		ssaoCombineCamera->SetMaterial(ssaoCombineMaterial);
		
		// PP pipeline
		RN::PostProcessingPipeline *ssaoPipeline = cam->AddPostProcessingPipeline("SSAO", 0);
		ssaoPipeline->AddStage(normalsCamera, RN::RenderStage::Mode::ReRender);
		ssaoPipeline->AddStage(ssaoCamera, RN::RenderStage::Mode::ReUsePreviousStage);
		ssaoPipeline->AddStage(ssaoBlurX, RN::RenderStage::Mode::ReUsePreviousStage);
		ssaoPipeline->AddStage(ssaoBlurY, RN::RenderStage::Mode::ReUsePreviousStage);
		ssaoPipeline->AddStage(ssaoCombineCamera, RN::RenderStage::Mode::ReUsePipeline);
	}
	
	void World::PPActivateBloom(RN::Camera *cam)
	{
		RN::Shader *combineShader = RN::Shader::WithFile("shader/rn_PPCombine");
		RN::Shader *blurShader = RN::Shader::WithFile("shader/rn_GaussBlur");
		RN::Shader *updownShader = RN::Shader::WithFile("shader/rn_PPCopy");
		
		float blurWeights[10];
		float sigma = 4.5/3;
		sigma *= sigma;
		int n = 0;
		float gaussfactor = 1.0f/(sqrt(2.0f*RN::k::Pi*sigma));
		float weightsum = 0;
		for(float i = -4.5f; i <= 4.6f; i += 1.0f)
		{
			blurWeights[n] = exp(-(i*i/2.0f*sigma))*gaussfactor;
			weightsum += blurWeights[n];
			n++;
		}
		for(int i = 0; i < n; i++)
		{
			blurWeights[i] /= weightsum;
		}
		
		RN::Material *blurXMaterial = new RN::Material(blurShader);
		blurXMaterial->Define("RN_BLURX");
		blurXMaterial->AddShaderUniform("kernelWeights", RN::Material::ShaderUniform::Type::Float1, blurWeights, 10, true);
		
		RN::Material *blurYMaterial = new RN::Material(blurShader);
		blurYMaterial->Define("RN_BLURY");
		blurYMaterial->AddShaderUniform("kernelWeights", RN::Material::ShaderUniform::Type::Float1, blurWeights, 10, true);
		
		RN::Material *downMaterial = new RN::Material(updownShader);
		downMaterial->Define("RN_DOWNSAMPLE");
		
		// Filter bright
		RN::Shader *filterBrightShader = RN::Shader::WithFile("shader/rn_FilterBright");
		RN::Material *filterBrightMaterial = new RN::Material(filterBrightShader);
		RN::Camera *filterBright = new RN::Camera(cam->GetFrame().Size() / 2.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		filterBright->SetMaterial(filterBrightMaterial);
		
		// Down sample
		RN::Camera *downSample4x = new RN::Camera(cam->GetFrame().Size() / 4.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		downSample4x->SetMaterial(downMaterial);
		
		// Down sample
		RN::Camera *downSample8x = new RN::Camera(cam->GetFrame().Size() / 8.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		downSample8x->SetMaterial(downMaterial);
		
		// Blur X
		RN::Camera *bloomBlurXlow = new RN::Camera(cam->GetFrame().Size() / 8.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		bloomBlurXlow->SetMaterial(blurXMaterial);
		
		// Blur Y
		RN::Camera *bloomBlurYlow = new RN::Camera(cam->GetFrame().Size() / 8.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		bloomBlurYlow->SetMaterial(blurYMaterial);
		
		// Blur X
		RN::Camera *bloomBlurXhigh = new RN::Camera(cam->GetFrame().Size() / 4.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		bloomBlurXhigh->SetMaterial(blurXMaterial);
		
		// Blur Y
		RN::Camera *bloomBlurYhigh = new RN::Camera(cam->GetFrame().Size() / 4.0f, RN::Texture::Format::RGB16F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		bloomBlurYhigh->SetMaterial(blurYMaterial);
		
		// Combine
		RN::Material *bloomCombineMaterial = new RN::Material(combineShader);
		bloomCombineMaterial->AddTexture(bloomBlurYhigh->GetStorage()->GetRenderTarget());
		
		RN::Camera *bloomCombine = new RN::Camera(RN::Vector2(0.0f), RN::Texture::Format::RGB16F, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		bloomCombine->SetMaterial(bloomCombineMaterial);
		
		RN::PostProcessingPipeline *bloom = cam->AddPostProcessingPipeline("Bloom", 0);
		bloom->AddStage(filterBright, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(downSample4x, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(downSample8x, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(bloomBlurXlow, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(bloomBlurYlow, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(bloomBlurXhigh, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(bloomBlurYhigh, RN::RenderStage::Mode::ReUsePreviousStage);
		bloom->AddStage(bloomCombine, RN::RenderStage::Mode::ReUsePipeline);
	}
	
	void World::PPActivateFXAA(RN::Camera *cam)
	{
		RN::Shader *tonemappingShader = RN::Shader::WithFile("shader/rn_DrawFramebufferTonemap");
		RN::Shader *fxaaShader = RN::Shader::WithFile("shader/rn_FXAA");
		
		// FXAA
		RN::Material *tonemappingMaterial = new RN::Material(tonemappingShader);
		RN::Material *fxaaMaterial = new RN::Material(fxaaShader);
		
		RN::Camera *tonemappingCam = new RN::Camera(RN::Vector2(0.0f), RN::Texture::Format::RGB16F, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		tonemappingCam->SetMaterial(tonemappingMaterial);
		
		RN::Camera *fxaaCam = new RN::Camera(RN::Vector2(0.0f), RN::Texture::Format::RGB16F, RN::Camera::Flags::Inherit | RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		fxaaCam->SetMaterial(fxaaMaterial);
		
		RN::PostProcessingPipeline *fxaa = cam->AddPostProcessingPipeline("FXAA", 0);
		fxaa->AddStage(tonemappingCam, RN::RenderStage::Mode::ReUsePipeline);
		fxaa->AddStage(fxaaCam, RN::RenderStage::Mode::ReUsePipeline);
	}

	void World::LoadLevelJSON(const std::string &file)
	{
		std::string path = RN::FileManager::GetSharedInstance()->GetFilePathWithName(file);
		
		RN::Data *data = RN::Data::WithContentsOfFile(path);
		RN::Array *objects = RN::JSONSerialization::JSONObjectFromData<RN::Array>(data);
		
		RN::Dictionary *settings = new RN::Dictionary();
		settings->SetObjectForKey(RN::Number::WithBool(true), RNCSTR("recalculateNormals"));
		
		objects->Enumerate<RN::Dictionary>([&](RN::Dictionary *dictionary, size_t indes, bool &stop) {
			
			RN::String *file = dictionary->GetObjectForKey<RN::String>(RNCSTR("model"));
			RN::Model *model = RN::ResourceCoordinator::GetSharedInstance()->GetResourceWithName<RN::Model>(file, settings);
			
			RN::Entity *entity = new RN::Entity(model);
			
			RN::Array *position = dictionary->GetObjectForKey<RN::Array>(RNCSTR("position"));
			RN::Array *rotation = dictionary->GetObjectForKey<RN::Array>(RNCSTR("rotation"));
			RN::Array *discard  = dictionary->GetObjectForKey<RN::Array>(RNCSTR("discard"));
			RN::Number *scale   = dictionary->GetObjectForKey<RN::Number>(RNSTR("scale"));
			RN::Number *occluder = dictionary->GetObjectForKey<RN::Number>(RNCSTR("occluder"));
			
			if(position)
			{
				float x = position->GetObjectAtIndex<RN::Number>(0)->GetFloatValue();
				float y = position->GetObjectAtIndex<RN::Number>(1)->GetFloatValue();
				float z = position->GetObjectAtIndex<RN::Number>(2)->GetFloatValue();
				
				entity->SetPosition(RN::Vector3(x, y, z));
			}
			
			if(rotation)
			{
				float x = rotation->GetObjectAtIndex<RN::Number>(0)->GetFloatValue();
				float y = rotation->GetObjectAtIndex<RN::Number>(1)->GetFloatValue();
				float z = rotation->GetObjectAtIndex<RN::Number>(2)->GetFloatValue();
				
				entity->Rotate(RN::Vector3(x, y, z));
			}
			
			if(scale)
				entity->SetScale(RN::Vector3(scale->GetFloatValue()));
			
			if(discard)
			{
				discard->Enumerate<RN::Number>([&](RN::Number *number, size_t index, bool &stop) {
					
					index = number->GetUint32Value();
					model->GetMaterialAtIndex(0, index)->SetDiscard(true);
					
				});
			}
			
			if(!occluder || occluder->GetBoolValue())
				_obstacles.push_back(entity->GetBoundingBox());
		});
		
		settings->Release();
	}
}
