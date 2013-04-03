//
//  RNRenderer.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNRenderer.h"
#include "RNMatrix.h"
#include "RNQuaternion.h"
#include "RNCamera.h"
#include "RNKernel.h"
#include "RNThreadPool.h"

#define kRNRendererInstancingCutOff  100
#define kRNRendererMaxVAOAge         300

#define kRNRendererPointLightListIndicesIndex 0
#define kRNRendererPointLightListOffsetIndex  1
#define kRNRendererPointLightListDataIndex    2

#define kRNRendererSpotLightListIndicesIndex 0
#define kRNRendererSpotLightListOffsetIndex  1
#define kRNRendererSpotLightListDataIndex    2

namespace RN
{
	Renderer::Renderer()
	{
		_defaultFBO = 0;
		_defaultWidth = _defaultHeight = 0;
		
		_currentMaterial = 0;
		_currentProgram	 = 0;
		_currentCamera   = 0;
		_currentVAO      = 0;
		
		_scaleFactor = Kernel::SharedInstance()->ScaleFactor();
		_time = 0.0f;
		
		// Default OpenGL state
		// TODO: Those initial values are gathered from the OpenGL 4.0 man pages, not sure if they are true for all versions!
		_cullingEnabled   = false;
		_depthTestEnabled = false;
		_blendingEnabled  = false;
		_depthWrite       = false;
		
		_cullMode  = GL_CCW;
		_depthFunc = GL_LESS;
		
		_blendSource      = GL_ONE;
		_blendDestination = GL_ZERO;
		
		// Setup framebuffer copy stuff
		_copyShader = 0;
		
		_copyVertices[0] = Vector4(-1.0f, 1.0f, 0.0f, 1.0f);
		_copyVertices[1] = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
		_copyVertices[2] = Vector4(1.0f, -1.0f, 1.0f, 0.0f);
		_copyVertices[3] = Vector4(-1.0f, -1.0f, 0.0f, 0.0f);
		
		_copyIndices[0] = 0;
		_copyIndices[1] = 3;
		_copyIndices[2] = 1;
		_copyIndices[3] = 2;
		_copyIndices[4] = 1;
		_copyIndices[5] = 3;
		
		_lightIndicesBuffer = 0;
		_lightOffsetBuffer = 0;
		_tempLightIndicesBuffer = 0;
		
		_lightIndicesBufferSize = 0;
		_lightOffsetBufferSize = 0;
		
		glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint *)&_maxTextureUnits);
		_textureUnit = 0;
		
		_hasValidFramebuffer = false;
		_frameCamera = 0;
		
		Initialize();
	}
	
	Renderer::~Renderer()
	{
	}
	
	void Renderer::Initialize()
	{
		_copyShader = new Shader("shader/rn_CopyFramebuffer");
		
		gl::GenVertexArrays(1, &_copyVAO);
		gl::BindVertexArray(_copyVAO);
		
		glGenBuffers(1, &_copyVBO);
		glGenBuffers(1, &_copyIBO);
		
		glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
		glBufferData(GL_ARRAY_BUFFER, 16 * sizeof(GLfloat), _copyVertices, GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(GLshort), _copyIndices, GL_STATIC_DRAW);
		
		gl::BindVertexArray(0);
		
#if !(RN_PLATFORM_IOS)
		// Point lights
		_lightPointDataSize = 0;
		
		glGenTextures(3, _lightPointTextures);
		glGenBuffers(3, _lightPointBuffers);
		
		// light index offsets
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListIndicesIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListIndicesIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _lightPointBuffers[kRNRendererPointLightListIndicesIndex]);
		
		// Light indices
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListOffsetIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListOffsetIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32I, _lightPointBuffers[kRNRendererPointLightListOffsetIndex]);
		
		// Light Data
		glBindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListDataIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
		
		// Spot lights
		_lightSpotDataSize = 0;
		
		glGenTextures(3, _lightSpotTextures);
		glGenBuffers(3, _lightSpotBuffers);
		
		// light index offsets
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListIndicesIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex]);
		
		// Light indices
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListOffsetIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32I, _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex]);
		
		// Light Data
		glBindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListDataIndex]);
		glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
		glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
		
#endif
	}
	
	
	void Renderer::SetDefaultFBO(GLuint fbo)
	{
		_defaultFBO = fbo;
		_hasValidFramebuffer = false;
	}
	
	void Renderer::SetDefaultFrame(uint32 width, uint32 height)
	{
		_defaultWidth  = width;
		_defaultHeight = height;
	}
	
	
	// ---------------------
	// MARK: -
	// MARK: Additional helper
	// ---------------------
	
	void Renderer::UpdateShaderData()
	{
		const Matrix& projectionMatrix = _currentCamera->projectionMatrix;
		const Matrix& inverseProjectionMatrix = _currentCamera->inverseProjectionMatrix;
		
		const Matrix& viewMatrix = _currentCamera->viewMatrix;
		const Matrix& inverseViewMatrix = _currentCamera->inverseViewMatrix;
		
		if(_currentProgram->frameSize != -1)
		{
			const Rect& frame = _currentCamera->Frame();
			glUniform4f(_currentProgram->frameSize, 1.0f/frame.width/_scaleFactor, 1.0f/frame.height/_scaleFactor, frame.width * _scaleFactor, frame.height * _scaleFactor);
		}
		
		if(_currentProgram->clipPlanes != -1)
			glUniform2f(_currentProgram->clipPlanes, _currentCamera->clipnear, _currentCamera->clipfar);
		
		if(_currentProgram->matProj != -1)
			glUniformMatrix4fv(_currentProgram->matProj, 1, GL_FALSE, projectionMatrix.m);
		
		if(_currentProgram->matProjInverse != -1)
			glUniformMatrix4fv(_currentProgram->matProjInverse, 1, GL_FALSE, inverseProjectionMatrix.m);
		
		if(_currentProgram->matView != -1)
			glUniformMatrix4fv(_currentProgram->matView, 1, GL_FALSE, viewMatrix.m);
		
		if(_currentProgram->matViewInverse != -1)
			glUniformMatrix4fv(_currentProgram->matViewInverse, 1, GL_FALSE, inverseViewMatrix.m);
		
		if(_currentProgram->matProjView != -1)
		{
			Matrix projectionViewMatrix = projectionMatrix * viewMatrix;
			glUniformMatrix4fv(_currentProgram->matProjView, 1, GL_FALSE, projectionViewMatrix.m);
		}
		
		if(_currentProgram->matProjViewInverse != -1)
		{
			Matrix inverseProjectionViewMatrix = inverseProjectionMatrix * inverseViewMatrix;
			glUniformMatrix4fv(_currentProgram->matProjViewInverse, 1, GL_FALSE, inverseProjectionViewMatrix.m);
		}
		
		if(_currentProgram->viewPosition != -1)
		{
			const Vector3& position = _currentCamera->Position();
			glUniform3fv(_currentProgram->viewPosition, 1, &position.x);
		}
	}
	
	void Renderer::AllocateLightBufferStorage(size_t indicesSize, size_t offsetSize)
	{
		if(indicesSize > _lightIndicesBufferSize)
		{
			delete _lightIndicesBuffer;
			delete _tempLightIndicesBuffer;
			
			_lightIndicesBufferSize = indicesSize;
			
			_lightIndicesBuffer = new int[_lightIndicesBufferSize];
			_tempLightIndicesBuffer = new int[_lightIndicesBufferSize];
		}
		
		if(offsetSize > _lightOffsetBufferSize)
		{
			delete _lightOffsetBuffer;
			
			_lightOffsetBufferSize = offsetSize;
			_lightOffsetBuffer = new int[_lightOffsetBufferSize];
		}
	}
	
#define Distance(plane, op, r) { \
	float dot = (position.x * plane._normal.x + position.y * plane._normal.y + position.z * plane._normal.z);\
	float distance = dot - plane._d; \
	if(distance op r) \
		continue; \
	}
	
	void Renderer::CullLights(Camera *camera, Light **lights, machine_uint lightCount, GLuint indicesBuffer, GLuint offsetBuffer)
	{
		Rect rect = camera->Frame();
		int tilesWidth  = rect.width / camera->LightTiles().x;
		int tilesHeight = rect.height / camera->LightTiles().y;
		
		Vector3 corner1 = camera->ToWorld(Vector3(-1.0f, -1.0f, 1.0f));
		Vector3 corner2 = camera->ToWorld(Vector3(1.0f, -1.0f, 1.0f));
		Vector3 corner3 = camera->ToWorld(Vector3(-1.0f, 1.0f, 1.0f));
		
		Vector3 dirx = (corner2-corner1) / tilesWidth;
		Vector3 diry = (corner3-corner1) / tilesHeight;
		
		const Vector3& camPosition = camera->Position();
		float *depthArray = camera->DepthArray();
		
		Vector3 camdir = camera->Rotation().RotateVector(RN::Vector3(0.0, 0.0, -1.0));
		Vector3 far = camera->ToWorld(Vector3(1.0f, 1.0f, 1.0f));
		far = far-camPosition;
		
		ThreadPool *pool = ThreadCoordinator::SharedInstance()->GlobalPool();
		pool->BeginTaskBatch();
		
		size_t i = 0;
		size_t tileCount = tilesWidth * tilesHeight;
		
		size_t lightindicesSize = tilesWidth * tilesHeight * lightCount;
		size_t lightindexoffsetSize = tilesWidth * tilesHeight * 2;
		
		AllocateLightBufferStorage(lightindicesSize, lightindexoffsetSize);
		
		std::vector<std::future<size_t>> futures(tileCount);
		
		for(int y=0; y<tilesHeight; y++)
		{
			for(int x=0; x<tilesWidth; x++)
			{
				size_t index = i;
				futures[i ++] = pool->AddTask([&, x, y, index]()->size_t {
					Plane plleft;
					Plane plright;
					Plane pltop;
					Plane plbottom;
					
					Plane plfar;
					Plane plnear;
					
					plleft.SetPlane(camPosition, corner1+dirx*x+diry*(y+1.0f), corner1+dirx*x+diry*(y-1.0f));
					plright.SetPlane(camPosition, corner1+dirx*(x+1.0f)+diry*(y+1.0f), corner1+dirx*(x+1.0f)+diry*(y-1.0f));
					pltop.SetPlane(camPosition, corner1+dirx*(x-1.0f)+diry*(y+1.0f), corner1+dirx*(x+1.0f)+diry*(y+1.0f));
					plbottom.SetPlane(camPosition, corner1+dirx*(x-1.0f)+diry*y, corner1+dirx*(x+1.0f)+diry*y);
					
					plnear.SetPlane(camPosition + camdir * depthArray[index * 2 + 0], camdir);
					plfar.SetPlane(camPosition + camdir * depthArray[index * 2 + 1], camdir);
					
					size_t lightIndicesCount = 0;
					int *lightPointIndices = _tempLightIndicesBuffer + (index * lightCount);
					
					for(size_t i=0; i<lightCount; i++)
					{
						Light *light = lights[i];
						
						const Vector3& position = light->_worldPosition;
						const float range = light->_range;
						
						Distance(plleft, >, range);
						Distance(plright, <, -range);
						Distance(pltop, <, -range);
						Distance(plbottom, >, range);
						
						Distance(plnear, <, -range);
						Distance(plfar, >, range);
						
						lightPointIndices[lightIndicesCount ++] = static_cast<int>(i);
					}
					
					return lightIndicesCount;
				});
			}
		}
		
		pool->EndTaskBatch();
		
		size_t lightIndicesCount = 0;
		size_t lightIndexOffsetCount = 0;
		
		for(i=0; i<tileCount; i++)
		{
			size_t tileCount = futures[i].get();
			int *tileIndices = _tempLightIndicesBuffer + (i * lightCount);
			
			size_t previous = lightIndicesCount;
			_lightOffsetBuffer[lightIndexOffsetCount ++] = static_cast<int>(previous);
			
			std::copy(tileIndices, tileIndices + tileCount, _lightIndicesBuffer + lightIndicesCount);
			lightIndicesCount += tileCount;
			
			_lightOffsetBuffer[lightIndexOffsetCount ++] = static_cast<int>(lightIndicesCount - previous);
		}
		
		// Indices
		if(lightIndicesCount == 0)
			lightIndicesCount ++;
		
		glBindBuffer(GL_TEXTURE_BUFFER, indicesBuffer);
		glBufferData(GL_TEXTURE_BUFFER, lightIndicesCount * sizeof(int), 0, GL_DYNAMIC_DRAW);
		glBufferData(GL_TEXTURE_BUFFER, lightIndicesCount * sizeof(int), _lightIndicesBuffer, GL_DYNAMIC_DRAW);
		
		// Offsets
		glBindBuffer(GL_TEXTURE_BUFFER, offsetBuffer);
		glBufferData(GL_TEXTURE_BUFFER, lightIndexOffsetCount * sizeof(int), 0, GL_DYNAMIC_DRAW);
		glBufferData(GL_TEXTURE_BUFFER, lightIndexOffsetCount * sizeof(int), _lightOffsetBuffer, GL_DYNAMIC_DRAW);
	}
	
	int Renderer::CreatePointLightList(Camera *camera)
	{
		Light **lights = _pointLights.Data();
		machine_uint lightCount = _pointLights.Count();
		
		lightCount = MIN(camera->MaxLightsPerTile(), lightCount);
		
		if(camera->DepthTiles())
		{
			GLuint indicesBuffer = _lightPointBuffers[kRNRendererPointLightListIndicesIndex];
			GLuint offsetBuffer = _lightPointBuffers[kRNRendererPointLightListOffsetIndex];
			
			CullLights(camera, lights, lightCount, indicesBuffer, offsetBuffer);
			
			// Write the position, range and colour of the lights
			Vector4 *lightData = 0;
			size_t lightDataSize = lightCount * 2 * sizeof(Vector4);
			
			if(lightDataSize == 0) // Makes sure that we don't end up with an empty buffer
				lightDataSize = 2 * sizeof(Vector4);
			
			glBindBuffer(GL_TEXTURE_BUFFER, _lightPointBuffers[kRNRendererPointLightListDataIndex]);
			if(lightDataSize > _lightPointDataSize)
			{
				glBufferData(GL_TEXTURE_BUFFER, _lightPointDataSize, 0, GL_DYNAMIC_DRAW);
				glBufferData(GL_TEXTURE_BUFFER, lightDataSize, 0, GL_DYNAMIC_DRAW);
				
				_lightPointDataSize = lightDataSize;
			}
			
			lightData = (Vector4 *)glMapBufferRange(GL_TEXTURE_BUFFER, 0, _lightPointDataSize, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);
			
			for(machine_uint i=0; i<lightCount; i++)
			{
				Light *light = lights[i];
				const Vector3& position = light->WorldPosition();
				const Vector3& color = light->Color();
				
				lightData[i * 2 + 0] = Vector4(position, light->Range());
				lightData[i * 2 + 1] = Vector4(color, 0.0f);
			}
			
			glUnmapBuffer(GL_TEXTURE_BUFFER);
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
		}
		
		return static_cast<int>(lightCount);
	}
	
	int Renderer::CreateSpotLightList(Camera *camera)
	{
		Light **lights = _spotLights.Data();
		machine_uint lightCount = _spotLights.Count();
		
		lightCount = MIN(camera->MaxLightsPerTile(), lightCount);
		
		if(camera->DepthTiles())
		{
			GLuint indicesBuffer = _lightSpotBuffers[kRNRendererSpotLightListIndicesIndex];
			GLuint offsetBuffer = _lightSpotBuffers[kRNRendererSpotLightListOffsetIndex];
			
			CullLights(camera, lights, lightCount, indicesBuffer, offsetBuffer);
			
			// Write the position, range, colour and direction of the lights
			Vector4 *lightData = 0;
			size_t lightDataSize = lightCount * 3 * sizeof(Vector4);
			
			if(lightDataSize == 0)
				lightDataSize = 3 * sizeof(Vector4); // Make sure that we don't end up with an empty buffer
				
			glBindBuffer(GL_TEXTURE_BUFFER, _lightSpotBuffers[kRNRendererSpotLightListDataIndex]);
			if(lightDataSize > _lightSpotDataSize)
			{
				glBufferData(GL_TEXTURE_BUFFER, _lightSpotDataSize, 0, GL_DYNAMIC_DRAW);
				glBufferData(GL_TEXTURE_BUFFER, lightDataSize, 0, GL_DYNAMIC_DRAW);
				
				_lightSpotDataSize = lightDataSize;
			}
			
			lightData = (Vector4 *)glMapBufferRange(GL_TEXTURE_BUFFER, 0, _lightSpotDataSize, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);
			
			for(machine_uint i=0; i<lightCount; i++)
			{
				Light *light = lights[i];
				const Vector3& position = light->WorldPosition();
				const Vector3& color = light->Color();
				const Vector3& direction = light->Direction();
				
				lightData[i * 3 + 0] = Vector4(position, light->Range());
				lightData[i * 3 + 1] = Vector4(color, 0.0f);
				lightData[i * 3 + 2] = Vector4(direction, light->Angle());
			}
			
			glUnmapBuffer(GL_TEXTURE_BUFFER);
			glBindBuffer(GL_TEXTURE_BUFFER, 0);
		}
		
		return static_cast<int>(lightCount);
	}
	
#undef Distance
		
	// ---------------------
	// MARK: -
	// MARK: Binding
	// ---------------------
	
	void Renderer::BindVAO(const std::tuple<ShaderProgram *, Mesh *>& tuple)
	{
		auto iterator = _autoVAOs.find(tuple);
		GLuint vao;
		
		if(iterator == _autoVAOs.end())
		{
			ShaderProgram *shader = std::get<0>(tuple);
			Mesh          *mesh  = std::get<1>(tuple);
			
			gl::GenVertexArrays(1, &vao);
			gl::BindVertexArray(vao);
			
			glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO());
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->IBO());
			
			// Vertices
			if(shader->vertPosition != -1 && mesh->SupportsFeature(kMeshFeatureVertices))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureVertices);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureVertices);
				
				glEnableVertexAttribArray(shader->vertPosition);
				glVertexAttribPointer(shader->vertPosition, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Normals
			if(shader->vertNormal != -1 && mesh->SupportsFeature(kMeshFeatureNormals))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureNormals);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureNormals);
				
				glEnableVertexAttribArray(shader->vertNormal);
				glVertexAttribPointer(shader->vertNormal, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Tangents
			if(shader->vertTangent != -1 && mesh->SupportsFeature(kMeshFeatureTangents))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureTangents);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureTangents);
				
				glEnableVertexAttribArray(shader->vertTangent);
				glVertexAttribPointer(shader->vertTangent, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Texcoord0
			if(shader->vertTexcoord0 != -1 && mesh->SupportsFeature(kMeshFeatureUVSet0))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureUVSet0);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureUVSet0);
				
				glEnableVertexAttribArray(shader->vertTexcoord0);
				glVertexAttribPointer(shader->vertTexcoord0, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Texcoord1
			if(shader->vertTexcoord1 != -1 && mesh->SupportsFeature(kMeshFeatureUVSet1))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureUVSet1);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureUVSet1);
				
				glEnableVertexAttribArray(shader->vertTexcoord1);
				glVertexAttribPointer(shader->vertTexcoord1, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Color0
			if(shader->vertColor0 != -1 && mesh->SupportsFeature(kMeshFeatureColor0))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureColor0);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureColor0);
				
				glEnableVertexAttribArray(shader->vertColor0);
				glVertexAttribPointer(shader->vertColor0, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Color1
			if(shader->vertColor1 != -1 && mesh->SupportsFeature(kMeshFeatureColor1))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureColor1);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureColor1);
				
				glEnableVertexAttribArray(shader->vertColor1);
				glVertexAttribPointer(shader->vertColor1, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Bone Weights
			if(shader->vertBoneWeights != -1 && mesh->SupportsFeature(kMeshFeatureBoneWeights))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureBoneWeights);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureBoneWeights);
				
				glEnableVertexAttribArray(shader->vertBoneWeights);
				glVertexAttribPointer(shader->vertBoneWeights, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			// Bone Indices
			if(shader->vertBoneIndices != -1 && mesh->SupportsFeature(kMeshFeatureBoneIndices))
			{
				MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureBoneIndices);
				size_t offset = mesh->OffsetForFeature(kMeshFeatureBoneIndices);
				
				glEnableVertexAttribArray(shader->vertBoneIndices);
				glVertexAttribPointer(shader->vertBoneIndices, descriptor->elementMember, GL_FLOAT, GL_FALSE, (GLsizei)mesh->Stride(), (const void *)offset);
			}
			
			_autoVAOs[tuple] = std::tuple<GLuint, uint32>(vao, 0);
			_currentVAO = vao;
		}
		
		uint32& age = std::get<1>(iterator->second);
		
		vao = std::get<0>(iterator->second);
		age = 0;
		
		BindVAO(vao);
	}
	
	void Renderer::BindMaterial(Material *material, ShaderProgram *program)
	{
		bool changedShader = (program != _currentProgram);
		UseShader(program);
		
		Material *surfaceMaterial = _currentCamera->Material();
		if(!surfaceMaterial)
			surfaceMaterial = material;
		
		if(changedShader || material != _currentMaterial)
		{
			_textureUnit = 0;
			
			Array<Texture> *textures = (_currentCamera->override & Camera::OverrideTextures) ? surfaceMaterial->Textures() : material->Textures();
			Array<GLuint> *textureLocations = &program->texlocations;
			
			if(textureLocations->Count() > 0)
			{
				machine_uint textureCount = MIN(textureLocations->Count(), textures->Count());
				
				for(machine_uint i=0; i<textureCount; i++)
				{
					GLint location = textureLocations->ObjectAtIndex(i);
					Texture *texture = textures->ObjectAtIndex(i);
					
					glUniform1i(location, BindTexture(texture));
				}
			}
		}

#define PickAttribute(override, attribute) (_currentCamera->override & Camera::override) ? surfaceMaterial->attribute : material->attribute
		
		SetCullingEnabled(PickAttribute(OverrideCulling, culling));
		SetCullMode(PickAttribute(OVerrideCullmode, cullmode));

		SetDepthTestEnabled(PickAttribute(OverrideDepthtest, depthtest));
		SetDepthFunction(PickAttribute(OverrideDepthtestMode, depthtestmode));
		
		if(_currentCamera->override & Camera::OverrideBlendmode)
		{
			SetDepthWriteEnabled((surfaceMaterial->depthwrite && _currentCamera->AllowsDepthWrite()));
		}
		else
		{
			SetDepthWriteEnabled((material->depthwrite && _currentCamera->AllowsDepthWrite()));
		}
		
		SetBlendingEnabled(PickAttribute(OverrideBlending, blending));
		
		if(_currentCamera->override & Camera::OverrideBlendmode)
		{
			SetBlendFunction(surfaceMaterial->blendSource, surfaceMaterial->blendDestination);
		}
		else
		{
			SetBlendFunction(material->blendSource, material->blendDestination);
		}
		
#undef PickAttribute
		
		_currentMaterial = material;
	}
	
	// ---------------------
	// MARK: -
	// MARK: Frame handling
	// ---------------------
	
	void Renderer::BeginFrame(float delta)
	{
		_time += delta;
		
		for(auto i=_autoVAOs.begin(); i!=_autoVAOs.end();)
		{
			uint32& age = std::get<1>(i->second);
			
			if((++ age) > kRNRendererMaxVAOAge)
			{
				i = _autoVAOs.erase(i);
				continue;
			}
			
			i ++;
		}
	}
	
	void Renderer::FinishFrame()
	{
		if(!_hasValidFramebuffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
			
			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			{
				_flushCameras.clear();
				return;
			}
			
			_hasValidFramebuffer = true;
		}
		
		glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glViewport(0, 0, _defaultWidth * _scaleFactor, _defaultHeight * _scaleFactor);
		
		for(auto iterator=_flushCameras.begin(); iterator!=_flushCameras.end(); iterator++)
		{
			Camera *camera  = *iterator;
			FlushCamera(camera);
		}
		
		_flushCameras.clear();
	}
	
	void Renderer::FlushCamera(Camera *camera)
	{
		_currentCamera = camera;
		_textureUnit = 0;
		
		SetDepthTestEnabled(false);
		
		if(_currentVAO != _copyVAO)
		{
			gl::BindVertexArray(_copyVAO);
			_currentVAO = _copyVAO;
			
			glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		}
		
		ShaderProgram *program = _copyShader->ProgramOfType(ShaderProgram::TypeNormal);
		
		UseShader(program);
		UpdateShaderData();
		
		glEnableVertexAttribArray(program->vertPosition);
		glVertexAttribPointer(program->vertPosition,  2, GL_FLOAT, GL_FALSE, 16, (const void *)0);
		
		glEnableVertexAttribArray(program->vertTexcoord0);
		glVertexAttribPointer(program->vertTexcoord0, 2, GL_FLOAT, GL_FALSE, 16, (const void *)8);
		
		uint32 targetmaps = MIN((uint32)program->targetmaplocations.Count(), camera->RenderTargets());
		if(targetmaps >= 1)
		{
			Texture *texture = camera->RenderTarget(0);
			GLuint location = program->targetmaplocations.ObjectAtIndex(0);
			
			glUniform1i(location, BindTexture(texture));
		}
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		
		glDisableVertexAttribArray(program->vertPosition);
		glDisableVertexAttribArray(program->vertTexcoord0);
	}
	
	void Renderer::DrawCameraStage(Camera *camera, Camera *stage)
	{
		Material *material = stage->Material();
		ShaderProgram *program = material->Shader()->ProgramOfType(ShaderProgram::TypeNormal);
		
		_currentCamera = stage;
		_textureUnit = 0;
		
		if(_currentVAO != _copyVAO)
		{
			gl::BindVertexArray(_copyVAO);
			_currentVAO = _copyVAO;
			
			glBindBuffer(GL_ARRAY_BUFFER, _copyVBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _copyIBO);
		}
		
		BindMaterial(material, program);
		UpdateShaderData();
		
		glEnableVertexAttribArray(program->vertPosition);
		glVertexAttribPointer(program->vertPosition,  2, GL_FLOAT, GL_FALSE, 16, (const void *)0);
		
		glEnableVertexAttribArray(program->vertTexcoord0);
		glVertexAttribPointer(program->vertTexcoord0, 2, GL_FLOAT, GL_FALSE, 16, (const void *)8);
		
		uint32 targetmaps = MIN((uint32)program->targetmaplocations.Count(), camera->RenderTargets());
		for(uint32 i=0; i<targetmaps; i++)
		{
			Texture *texture = camera->RenderTarget(i);
			GLuint location = program->targetmaplocations.ObjectAtIndex(i);
			
			glUniform1i(location, BindTexture(texture));
		}
		
		if(program->depthmap != -1)
		{
			Texture *depthmap = camera->Storage()->DepthTarget();
			if(depthmap)
			{
				glUniform1i(program->depthmap, BindTexture(depthmap));
			}
		}
		
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
		
		glDisableVertexAttribArray(program->vertPosition);
		glDisableVertexAttribArray(program->vertTexcoord0);
	}
	
	// ---------------------
	// MARK: -
	// MARK: Rendering
	// ---------------------
	
	void Renderer::BeginCamera(Camera *camera)
	{
		RN_ASSERT0(_frameCamera == 0);
		_frameCamera = camera;
		
		_currentProgram  = 0;
		_currentMaterial = 0;
		_currentCamera   = 0;
		_currentVAO      = 0;
		_textureUnit     = 0;
	}
	
	void Renderer::FinishCamera()
	{
		Camera *previous = 0;
		Camera *camera = _frameCamera;
		
		// Skycube
		Model *skyCube = camera->SkyCube();
		uint32 skyCubeMeshes = 0;
		
		Matrix cameraRotation;
		cameraRotation.MakeRotate(camera->Rotation());
		
		if(skyCube)
		{
			Array<RenderingObject> skyCubeObejcts;
			
			skyCubeMeshes = skyCube->Meshes(0);
			
			for(uint32 j=0; j<skyCubeMeshes; j++)
			{
				RenderingObject object;
				
				object.mesh = skyCube->MeshAtIndex(0, j);
				object.material = skyCube->MaterialAtIndex(0, j);
				object.transform = &cameraRotation;
				object.skeleton = 0;
				
				skyCubeObejcts.AddObject(object);
			}
			
			_frame.InsertObjectsAtIndex(skyCubeObejcts, 0);
		}
		
		// Render loop
		bool changedCamera;
		bool changedShader;
		
		while(camera)
		{
			camera->Bind();
			camera->PrepareForRendering();
			
			_currentCamera = camera;
			changedCamera  = true;
			
			if(!(camera->CameraFlags() & Camera::FlagDrawTarget))
			{				
				// Sort the objects
				_frame.SortUsingFunction([&](const RenderingObject& a, const RenderingObject& b) {
					// Sort by material
					const Material *materialA = a.material;
					const Material *materialB = b.material;
					
					if(materialA->blending != materialB->blending)
					{
						if(!materialB->blending)
							return kRNCompareGreaterThan;
						
						if(!materialA->blending)
							return kRNCompareLessThan;
					}
					
					if(materialA->alphatest != materialB->alphatest)
					{
						if(!materialB->alphatest)
							return kRNCompareGreaterThan;
						
						if(!materialA->alphatest)
							return kRNCompareLessThan;
					}
					
					if(materialA->Shader() > materialB->Shader())
						return kRNCompareGreaterThan;
					
					if(materialB->Shader() > materialA->Shader())
						return kRNCompareLessThan;
					
					// Sort by mesh
					if(a.mesh > b.mesh)
						return kRNCompareGreaterThan;
					
					if(b.mesh > a.mesh)
						return kRNCompareLessThan;
					
					return kRNCompareEqualTo;
				}, skyCubeMeshes);
				
				Material *surfaceMaterial = camera->Material();
					
				// Create the light lists for the camera
				int lightPointCount = CreatePointLightList(camera);
				int lightSpotCount  = CreateSpotLightList(camera);
				
				// Update the shader
				const Matrix& projectionMatrix = camera->projectionMatrix;
				const Matrix& inverseProjectionMatrix = camera->inverseProjectionMatrix;
				
				const Matrix& viewMatrix = camera->viewMatrix;
				const Matrix& inverseViewMatrix = camera->inverseViewMatrix;
				
				Matrix projectionViewMatrix = projectionMatrix * viewMatrix;
				Matrix inverseProjectionViewMatrix = inverseProjectionMatrix * inverseViewMatrix;
				
				uint32 offset;
				uint32 noCheck = 0;
				
				machine_uint objectsCount = _frame.Count();
				for(machine_uint i=0; i<objectsCount; i+=offset)
				{
					offset = 1;
					
					RenderingObject& object = _frame.ObjectAtIndex(i);
					
					Mesh *mesh = object.mesh;
					Material *material = object.material;
					Shader *shader = surfaceMaterial ? surfaceMaterial->Shader() : material->Shader();
					
					Matrix& transform = (Matrix &)*object.transform;
					Matrix inverseTransform = transform.Inverse();
					
					// Check if we can use instancing here
					bool canDrawInstanced = false;
					
					if(noCheck == 0 && SupportsOpenGLFeature(kOpenGLFeatureInstancing))
					{
						machine_uint end = i + 1;
						offset = 1;
						
						while(end < objectsCount)
						{
							RenderingObject& temp = _frame.ObjectAtIndex(end);
							
							if(temp.mesh != mesh)
								break;
							
							if(!surfaceMaterial && temp.material != material)
								break;
							
							offset ++;
							end ++;
						}
						
						canDrawInstanced = (offset >= kRNRendererInstancingCutOff && shader->SupportsProgramOfType(ShaderProgram::TypeInstanced));
						if(!canDrawInstanced)
						{
							noCheck = offset;
							offset = 1;
						}
					}
					else
					{
						if(noCheck > 0)
							noCheck --;
					}
					
					// Grab the correct shader program
					uint32 programTypes = 0;
					ShaderProgram *program = 0;
					
					bool wantsDiscard = material->discard;
					if(surfaceMaterial && _currentCamera->override & Camera::OverrideDiscard)
					{
						wantsDiscard = surfaceMaterial->discard;
					}
					
					if(object.skeleton && shader->SupportsProgramOfType(ShaderProgram::TypeAnimated))
						programTypes |= ShaderProgram::TypeAnimated;
					
					if(lightPointCount > 0 && shader->SupportsProgramOfType(ShaderProgram::TypeLightning))
						programTypes |= ShaderProgram::TypeLightning;
					
					if(canDrawInstanced && shader->SupportsProgramOfType(ShaderProgram::TypeInstanced))
						programTypes |= ShaderProgram::TypeInstanced;
					
					if(wantsDiscard)
					{
					if(wantsDiscard && shader->SupportsProgramOfType(ShaderProgram::TypeDiscard))
						programTypes |= ShaderProgram::TypeDiscard;
					}
		
					program = shader->ProgramOfType(programTypes);
					changedShader = (_currentProgram != program);
					
					BindMaterial(material, program);
					
					// Update the shader data
					if(changedShader || changedCamera)
					{
						UpdateShaderData();
						changedCamera = false;
					}
					
					if(changedShader)
					{
						// Light data
						if(program->lightPointCount != -1)
							glUniform1i(program->lightPointCount, lightPointCount);
						
						if(program->lightSpotCount != -1)
							glUniform1i(program->lightSpotCount, lightSpotCount);
						
						if(camera->LightTiles() != 0)
						{
							// Point lights
							if(program->lightPointList != -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListIndicesIndex]);
								glUniform1i(program->lightPointList, textureUnit);
							}
							
							if(program->lightPointListOffset != -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListOffsetIndex]);
								glUniform1i(program->lightPointListOffset, textureUnit);
							}
							
							if(program->lightPointListData != -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightPointTextures[kRNRendererPointLightListDataIndex]);
								glUniform1i(program->lightPointListData, textureUnit);
							}
							
							// Spot lights
							if(program->lightSpotList != -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListIndicesIndex]);
								glUniform1i(program->lightSpotList, textureUnit);
							}
							
							if(program->lightSpotListOffset!= -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListOffsetIndex]);
								glUniform1i(program->lightSpotListOffset, textureUnit);
							}
							
							if(program->lightSpotListData != -1)
							{
								uint32 textureUnit = BindTexture(GL_TEXTURE_BUFFER, _lightSpotTextures[kRNRendererSpotLightListDataIndex]);
								glUniform1i(program->lightSpotListData, textureUnit);
							}
							
							if(program->lightTileSize != -1)
							{
								Rect rect = camera->Frame();
								int tilesWidth  = rect.width / camera->LightTiles().x;
								int tilesHeight = rect.height / camera->LightTiles().y;
								
								Vector2 lightTilesSize = camera->LightTiles() * _scaleFactor;
								Vector2 lightTilesCount = Vector2(tilesWidth, tilesHeight);
								
								glUniform4f(program->lightTileSize, lightTilesSize.x, lightTilesSize.y, lightTilesCount.x, lightTilesCount.y);
							}
						}
						
						if(program->discardThreshold != -1)
						{
							float threshold = _currentMaterial->discardThreshold;
							
							if(surfaceMaterial && _currentCamera->override & Camera::OverrideDiscardThreshold)
								threshold = surfaceMaterial->discardThreshold;
							
							glUniform1f(program->discardThreshold, threshold);
						}
					}

					if(canDrawInstanced)
					{
						DrawMeshInstanced(i, offset);
						continue;
					}
					
					// More updates
					if(object.skeleton && program->matBones != -1)
					{
						float *data = reinterpret_cast<float *>(object.skeleton->Matrices().Data());
						glUniformMatrix4fv(program->matBones, object.skeleton->NumBones(), GL_FALSE, data);
					}
					
					if(program->matModel != -1)
						glUniformMatrix4fv(program->matModel, 1, GL_FALSE, transform.m);
					
					if(program->matModelInverse != -1)
						glUniformMatrix4fv(program->matModelInverse, 1, GL_FALSE, inverseTransform.m);
					
					if(program->matViewModel != -1)
					{
						Matrix viewModel = viewMatrix * transform;
						glUniformMatrix4fv(program->matViewModel, 1, GL_FALSE, viewModel.m);
					}
					
					if(program->matViewModelInverse != -1)
					{
						Matrix viewModel = inverseViewMatrix * inverseTransform;
						glUniformMatrix4fv(program->matViewModelInverse, 1, GL_FALSE, viewModel.m);
					}
					
					if(program->matProjViewModel != -1)
					{
						Matrix projViewModel = projectionViewMatrix * transform;
						glUniformMatrix4fv(program->matProjViewModel, 1, GL_FALSE, projViewModel.m);
					}
					
					if(program->matProjViewModelInverse != -1)
					{
						Matrix projViewModelInverse = inverseProjectionViewMatrix * inverseTransform;
						glUniformMatrix4fv(program->matProjViewModelInverse, 1, GL_FALSE, projViewModelInverse.m);
					}
					
					DrawMesh(mesh);
				}
			}
			else
			{
				if(previous)
					DrawCameraStage(previous, camera);
			}
			
			previous = camera;
			
			camera->Unbind();
			camera = camera->Stage();

			if(!camera)
				_flushCameras.push_back(previous);
		}
		
		// Cleanup of the frame
		_frameCamera = 0;
		
		_frame.RemoveAllObjects();
		_pointLights.RemoveAllObjects();
		_spotLights.RemoveAllObjects();
	}
	
	void Renderer::DrawMesh(Mesh *mesh)
	{
		MeshDescriptor *descriptor = mesh->Descriptor(kMeshFeatureIndices);
		BindVAO(std::tuple<ShaderProgram *, Mesh *>(_currentProgram, mesh));
		
		GLenum type = (descriptor->elementSize == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		glDrawElements(GL_TRIANGLES, (GLsizei)descriptor->elementCount, type, 0);
	}
	
	void Renderer::DrawMeshInstanced(machine_uint start, machine_uint count)
	{
		/*Mesh *mesh = _frame[(int)start].mesh;
		MeshLODStage *stage = mesh->LODStage(0);
		MeshDescriptor *descriptor = stage->Descriptor(kMeshFeatureIndices);
		
		BindVAO(std::tuple<ShaderProgram *, MeshLODStage *>(_currentProgram, stage));
		
		Matrix *instancingMatrices = 0;
		GLuint instancingVBO = 0;
		
		size_t size = (count * 2) * sizeof(Matrix);
		bool resized = stage->InstancingData(size, &instancingVBO, (void **)&instancingMatrices);
		
		glBindBuffer(GL_ARRAY_BUFFER, instancingVBO);
		
		if(resized)
			glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
		
		uint32 offset = 0;
		if(_currentProgram->imatModel != -1)
		{
			for(int i=0; i<4; i++)
			{
				glEnableVertexAttribArray(_currentProgram->imatModel + i);
				glVertexAttribPointer(_currentProgram->imatModel + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void *)(sizeof(float) * ((i * 4) + offset)));
				gl::VertexAttribDivisor(_currentProgram->imatModel + i, 1);
			}
			
			for(machine_uint i=0; i<count; i++)
			{
				RenderingObject& object = _frame[(int)(start + i)];
				instancingMatrices[i + offset] = *object.transform;
			}
			
			offset += count;
		}
		
		if(_currentProgram->imatModelInverse != -1)
		{
			for(int i=0; i<4; i++)
			{
				glEnableVertexAttribArray(_currentProgram->imatModelInverse + i);
				glVertexAttribPointer(_currentProgram->imatModelInverse + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 16, (void *)(sizeof(float) * ((i * 4) + offset)));
				gl::VertexAttribDivisor(_currentProgram->imatModelInverse + i, 1);
			}
			
			for(machine_uint i=0; i<count; i++)
			{
				RenderingObject& object = _frame[(int)(start + i)];
				instancingMatrices[i + offset] = object.transform->Inverse();
			}
			
			offset += count;
		}
		
		if(offset == 0)
			return;
		
		GLenum type = (descriptor->elementSize == 2) ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
		
		glBufferSubData(GL_ARRAY_BUFFER, 0, offset * sizeof(Matrix), instancingMatrices);
		gl::DrawElementsInstanced(GL_TRIANGLES, (GLsizei)descriptor->elementCount, type, 0, (GLsizei)count);
		
		// Disabling vertex attributes
		if(_currentProgram->imatModel != -1)
		{
			for(int i=0; i<4; i++)
				glDisableVertexAttribArray(_currentProgram->imatModel + i);
		}
		
		if(_currentProgram->imatModelInverse != -1)
		{
			for(int i=0; i<4; i++)
				glDisableVertexAttribArray(_currentProgram->imatModelInverse + i);
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	}
	
	void Renderer::RenderObject(const RenderingObject& object)
	{
		_frame.AddObject(object);
	}
	
	void Renderer::RenderLight(Light *light)
	{
		switch(light->LightType())
		{
			case Light::TypePointLight:
				_pointLights.AddObject(light);
				break;
				
			case Light::TypeSpotLight:
				_spotLights.AddObject(light);
				break;
				
			default:
				break;
		}
	}
}