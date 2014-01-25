//
//  RNLight.cpp
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNLight.h"
#include "RNWorld.h"
#include "RNCamera.h"
#include "RNResourceCoordinator.h"
#include "RNLightManager.h"
#include "RNCameraInternal.h"
#include "RNOpenGLQueue.h"

namespace RN
{
	RNDeclareMeta(Light)
	
	Light::Light(Type lighttype) :
		_lightType(lighttype),
		_color("color", Color(1.0f), std::bind(&Light::GetColor, this), std::bind(&Light::SetColor, this, std::placeholders::_1)),
		_intensity("intensity", 10.0f, std::bind(&Light::GetIntensity, this), std::bind(&Light::SetIntensity, this, std::placeholders::_1)),
		_range("range", 10.0f, std::bind(&Light::GetRange, this), std::bind(&Light::SetRange, this, std::placeholders::_1)),
		_angle("angle", 45.0f, std::bind(&Light::GetAngle, this), std::bind(&Light::SetAngle, this, std::placeholders::_1))
	{
		_suppressShadows = false;
		_shadowTarget  = nullptr;
		
		SetPriority(SceneNode::Priority::UpdateLate);
		
		SetCollisionGroup(25);
		_angleCos = 0.707f;
		
		AddObservable(&_color);
		AddObservable(&_intensity);
		AddObservable(&_range);
		AddObservable(&_angle);
		ReCalculateColor();
	}
	
	Light::~Light()
	{
		RemoveShadowCameras();
	}
	
	bool Light::IsVisibleInCamera(Camera *camera)
	{
		if(_lightType == Type::DirectionalLight)
			return true;
		
		return SceneNode::IsVisibleInCamera(camera);
	}
	
	void Light::Render(Renderer *renderer, Camera *camera)
	{
		LightManager *manager = camera->GetLightManager();
		
		if(manager)
			manager->AddLight(this);
	}
	
	void Light::SetRange(float range)
	{
		_range = range;
		
		SetBoundingSphere(Sphere(Vector3(), range));
		SetBoundingBox(AABB(Vector3(), range), false);
	}
	
	void Light::SetColor(const Color& color)
	{
		_color = color;
		ReCalculateColor();
	}
	
	void Light::SetIntensity(float intensity)
	{
		_intensity = intensity;
		ReCalculateColor();
	}
	
	void Light::SetAngle(float angle)
	{
		_angle = angle;
		_angleCos = cosf(angle*k::DegToRad);
	}
	
	void Light::RemoveShadowCameras()
	{
		_shadowDepthCameras.Enumerate<Camera>([&](Camera *camera, size_t index, bool &stop) {
			RemoveDependency(camera);
		});
		
		_shadowDepthCameras.RemoveAllObjects();
		
		if(_shadowTarget)
		{
			RemoveDependency(_shadowTarget);
			_shadowTarget->Release();
			_shadowTarget = nullptr;
		}
	}
	
	bool Light::ActivateShadows(const ShadowParameter &parameter)
	{
		switch(_lightType)
		{
			case Type::PointLight:
				return ActivatePointShadows(parameter);
				
			case Type::SpotLight:
				return ActivateSpotShadows(parameter);
				
			case Type::DirectionalLight:
				return ActivateDirectionalShadows(parameter);

			default:
				return false;
		}
	}
	
	void Light::DeactivateShadows()
	{
		RemoveShadowCameras();
	}
	
	void Light::SetSuppressShadows(bool suppress)
	{
		_suppressShadows = suppress;
		
		if(_suppressShadows)
		{
			_shadowDepthCameras.Enumerate<Camera>([&](Camera *camera, size_t index, bool &stop) {
				camera->SetFlags(camera->GetFlags()|Camera::Flags::NoRender);
			});
		}
		else
		{
			_shadowDepthCameras.Enumerate<Camera>([&](Camera *camera, size_t index, bool &stop) {
				camera->SetFlags(camera->GetFlags()&~Camera::Flags::NoRender);
			});
		}
	}
	
	
	bool Light::ActivateDirectionalShadows(const ShadowParameter &parameter)
	{
		if(_shadowDepthCameras.GetCount() > 0)
			DeactivateShadows();
		
		RN_ASSERT(parameter.shadowTarget, "Directional shadows need the shadowTarget to be set to a valid value!");
		
		if(_shadowTarget)
		{
			RemoveDependency(_shadowTarget);
			_shadowTarget->Release();
		}
		
		_shadowTarget = parameter.shadowTarget;
		_shadowTarget->Retain();
		AddDependency(_shadowTarget);
			
		_shadowSplits  = static_cast<int>(parameter.splits);
		_shadowDistanceBlendFactor = parameter.distanceBlendFactor;
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Clamp;
		textureParameter.filter = Texture::Filter::Linear;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = true;
		textureParameter.maxMipMaps = 0;
		
		Texture2DArray *depthtex = new Texture2DArray(textureParameter);
		depthtex->SetSize(32, 32, parameter.splits);
		depthtex->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyDirectionalShadowDepthShader, nullptr);
		Material *depthMaterial = new Material(depthShader);
		depthMaterial->polygonOffset = true;
		depthMaterial->polygonOffsetFactor = parameter.biasFactor;
		depthMaterial->polygonOffsetUnits  = parameter.biasUnits;
		
		for(int i = 0; i < _shadowSplits; i++)
		{
			RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
			storage->SetDepthTarget(depthtex, i);
			
			Camera *tempcam = new Camera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::Orthogonal | Camera::Flags::NoFlush, 1.0f);
			tempcam->SetMaterial(depthMaterial);
			tempcam->SetLODCamera(_shadowTarget);
			tempcam->SetLightManager(nullptr);
			tempcam->SetPriority(kRNShadowCameraPriority);
			tempcam->SetClipNear(1.0f);

			_shadowDepthCameras.AddObject(tempcam);
			AddDependency(tempcam);
			
			tempcam->Release();
			storage->Release();
			
			try
			{
				OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
					storage->BindAndUpdateBuffer();
				}, true);
			}
			catch(Exception e)
			{
				RemoveShadowCameras();
				return false;
			}
		}
		
		return true;
	}
	
	bool Light::ActivatePointShadows(const ShadowParameter &parameter)
	{
		if(_shadowDepthCameras.GetCount() > 0)
			DeactivateShadows();
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Repeat;
		textureParameter.filter = Texture::Filter::Nearest;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = false;
		textureParameter.maxMipMaps = 0;
		
		Texture *depthtex = (new TextureCubeMap(textureParameter))->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyPointShadowDepthShader, nullptr);
		Material *depthMaterial = (new Material(depthShader))->Autorelease();
		
		RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
		storage->SetDepthTarget(depthtex, -1);
		
		RN::Camera *shadowcam = new CubemapCamera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::NoFlush, 1.0f);
		shadowcam->Retain();
		shadowcam->Autorelease();
		shadowcam->SetMaterial(depthMaterial);
		shadowcam->SetPriority(kRNShadowCameraPriority);
		shadowcam->SetClipNear(0.01f);
		shadowcam->SetClipFar(_range);
		shadowcam->SetFOV(90.0f);
		shadowcam->SetLightManager(nullptr);
		shadowcam->UpdateProjection();
		shadowcam->SetWorldRotation(Vector3(0.0f, 0.0f, 0.0f));
		
		_shadowDepthCameras.AddObject(shadowcam);
		AddDependency(shadowcam);
		storage->Release();
		
		try
		{
			OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
				storage->BindAndUpdateBuffer();
			}, true);
		}
		catch(Exception e)
		{
			RemoveShadowCameras();
			return false;
		}
		
		return true;
	}
	
	bool Light::ActivateSpotShadows(const ShadowParameter &parameter)
	{
		if(_shadowDepthCameras.GetCount() > 0)
			DeactivateShadows();
		
		Texture::Parameter textureParameter;
		textureParameter.wrapMode = Texture::WrapMode::Clamp;
		textureParameter.filter = Texture::Filter::Linear;
		textureParameter.format = Texture::Format::Depth24I;
		textureParameter.depthCompare = true;
		textureParameter.maxMipMaps = 0;
		
		Texture *depthtex = new Texture2D(textureParameter);
		depthtex->Autorelease();
		
		Shader   *depthShader = ResourceCoordinator::GetSharedInstance()->GetResourceWithName<Shader>(kRNResourceKeyDirectionalShadowDepthShader, nullptr);
		Material *depthMaterial = (new Material(depthShader))->Autorelease();
		depthMaterial->polygonOffset = true;
		depthMaterial->polygonOffsetFactor = parameter.biasFactor;
		depthMaterial->polygonOffsetUnits  = parameter.biasUnits;
		
		RenderStorage *storage = new RenderStorage(RenderStorage::BufferFormatDepth, 0, 1.0f);
		storage->SetDepthTarget(depthtex, -1);
		
		RN::Camera *shadowcam = new Camera(Vector2(parameter.resolution), storage, Camera::Flags::UpdateAspect | Camera::Flags::UpdateStorageFrame | Camera::Flags::NoFlush, 1.0f);
		shadowcam->Retain();
		shadowcam->Autorelease();
		shadowcam->SetMaterial(depthMaterial);
		shadowcam->SetPriority(kRNShadowCameraPriority);
		shadowcam->SetClipNear(0.01f);
		shadowcam->SetClipFar(_range);
		shadowcam->SetFOV(_angle * 2.0f);
		shadowcam->SetLightManager(nullptr);
		shadowcam->UpdateProjection();
		shadowcam->SetWorldRotation(Vector3(0.0f, 0.0f, 0.0f));
		
		_shadowDepthCameras.AddObject(shadowcam);
		AddDependency(shadowcam);
		storage->Release();
		
		try
		{
			OpenGLQueue::GetSharedInstance()->SubmitCommand([&] {
				storage->BindAndUpdateBuffer();
			}, true);
		}
		catch(Exception e)
		{
			RemoveShadowCameras();
			return false;
		}
		
		return true;
	}
	
	
	void Light::Update(float delta)
	{
		SceneNode::Update(delta);
		
		if(_suppressShadows || _shadowDepthCameras.GetCount() == 0)
			return;
		
		if(_lightType == Type::DirectionalLight)
		{
			if(_shadowTarget)
			{
				float near = _shadowTarget->GetClipNear();
				float far;
				
				_shadowCameraMatrices.clear();
				
				for(int i = 0; i < _shadowSplits; i++)
				{
					float linear = _shadowTarget->GetClipNear() + (_shadowTarget->GetClipFar() - _shadowTarget->GetClipNear())*(i+1.0f) / float(_shadowSplits);
					float log = _shadowTarget->GetClipNear() * powf(_shadowTarget->GetClipFar() / _shadowTarget->GetClipNear(), (i+1.0f) / float(_shadowSplits));
					far = linear*_shadowDistanceBlendFactor+log*(1.0f-_shadowDistanceBlendFactor);
					
					Camera *tempcam = _shadowDepthCameras.GetObjectAtIndex<Camera>(i);
					tempcam->SetWorldRotation(GetWorldRotation());
					
					_shadowCameraMatrices.push_back(std::move(tempcam->MakeShadowSplit(_shadowTarget, this, near, far)));
					
					near = far;
				}
			}
		}
		else if(_lightType == Type::SpotLight)
		{
			RN::Camera *shadowcam = static_cast<RN::Camera*>(_shadowDepthCameras.GetFirstObject());
			
			shadowcam->SetWorldPosition(GetWorldPosition());
			shadowcam->SetWorldRotation(GetWorldRotation());
			shadowcam->SetClipFar(_range);
			shadowcam->SetFOV(_angle * 2.0f);
			shadowcam->UpdateProjection();
			shadowcam->PostUpdate();
			
			_shadowCameraMatrices.clear();
			_shadowCameraMatrices.emplace_back(shadowcam->GetProjectionMatrix() * shadowcam->GetViewMatrix());
		}
		else
		{
			RN::Camera *shadowcam = static_cast<RN::Camera*>(_shadowDepthCameras.GetFirstObject());
			shadowcam->SetWorldPosition(GetWorldPosition());
			shadowcam->SetClipFar(_range);
			shadowcam->UpdateProjection();
		}
	}

	void Light::ReCalculateColor()
	{
		_resultColor = Vector3(_color->r, _color->g, _color->b);
		_resultColor *= (float)_intensity;
	}
}
