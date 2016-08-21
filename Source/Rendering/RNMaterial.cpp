//
//  RNMaterial.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNMaterial.h"

namespace RN
{
	RNDefineMeta(Material, Object)

	MaterialDescriptor::MaterialDescriptor() :
		fragmentShader(nullptr),
		vertexShader(nullptr),
		discardThreshold(0.3f),
		depthMode(DepthMode::Less),
		depthWriteEnabled(true),
		ambientColor(1.0f, 1.0f, 1.0f, 1.0f),
		diffuseColor(1.0f, 1.0f, 1.0f, 1.0f),
		specularColor(1.0f, 1.0f, 1.0f, 4.0f),
		emissiveColor(0.0f, 0.0f, 0.0f, 0.0f),
		textureTileFactor(1.0),
		cullMode(CullMode::BackFace),
		_textures(new Array())
	{}

	MaterialDescriptor::MaterialDescriptor(const MaterialDescriptor &other) :
		fragmentShader(other.fragmentShader),
		vertexShader(other.vertexShader),
		discardThreshold(other.discardThreshold),
		depthMode(other.depthMode),
		depthWriteEnabled(other.depthWriteEnabled),
		ambientColor(other.ambientColor),
		diffuseColor(other.diffuseColor),
		specularColor(other.specularColor),
		emissiveColor(other.emissiveColor),
		textureTileFactor(other.textureTileFactor),
		cullMode(other.cullMode),
		_textures(SafeCopy(other._textures))
	{}

	MaterialDescriptor::~MaterialDescriptor()
	{
		SafeRelease(_textures);
	}


	void MaterialDescriptor::SetTextures(const Array *textures)
	{
		_textures->Release();
		_textures = textures->Copy();
	}

	void MaterialDescriptor::AddTexture(Texture *texture)
	{
		_textures->AddObject(texture);
	}
	void MaterialDescriptor::RemoveAllTextures()
	{
		_textures->RemoveAllObjects();
	}
	const Array *MaterialDescriptor::GetTextures() const
	{
		return _textures;
	}

	void MaterialDescriptor::SetShaderProgram(const ShaderProgram *program)
	{
		fragmentShader = program->GetFragmentShader();
		vertexShader = program->GetVertexShader();
	}





	Material::Material(const MaterialDescriptor &descriptor) :
		_fragmentShader(SafeRetain(descriptor.fragmentShader)),
		_vertexShader(SafeRetain(descriptor.vertexShader)),
		_textures(SafeCopy(descriptor.GetTextures())),
		_depthMode(descriptor.depthMode),
		_depthWriteEnabled(descriptor.depthWriteEnabled),
		_ambientColor(descriptor.ambientColor),
		_diffuseColor(descriptor.diffuseColor),
		_specularColor(descriptor.specularColor),
		_emissiveColor(descriptor.emissiveColor),
		_discardThreshold(descriptor.discardThreshold),
		_textureTileFactor(descriptor.textureTileFactor),
		_cullMode(descriptor.cullMode),
		_vertexBuffers(nullptr),
		_fragmentBuffers(nullptr)
	{
		RN_ASSERT(!_fragmentShader || _fragmentShader->GetType() == Shader::Type::Fragment, "Fragment shader must be a fragment shader");
		RN_ASSERT(_vertexShader->GetType() == Shader::Type::Vertex, "Vertex shader must be a vertex shader");
	}

	Material::Material(const Material *other) :
		_fragmentShader(SafeRetain(other->_fragmentShader)),
		_vertexShader(SafeRetain(other->_vertexShader)),
		_textures(SafeRetain(other->_textures)),
		_depthMode(other->_depthMode),
		_depthWriteEnabled(other->_depthWriteEnabled),
		_ambientColor(other->_ambientColor),
		_diffuseColor(other->_diffuseColor),
		_specularColor(other->_specularColor),
		_emissiveColor(other->_emissiveColor),
		_discardThreshold(other->_discardThreshold),
		_textureTileFactor(other->_textureTileFactor),
		_cullMode(other->_cullMode),
		_vertexBuffers(SafeCopy(other->_vertexBuffers)),
		_fragmentBuffers(SafeCopy(other->_fragmentBuffers))
	{}

	Material *Material::WithDescriptor(const MaterialDescriptor &descriptor)
	{
		Material *material = new Material(descriptor);
		return material->Autorelease();
	}

	Material::~Material()
	{
		SafeRelease(_vertexBuffers);
		SafeRelease(_fragmentBuffers);
		SafeRelease(_fragmentShader);
		SafeRelease(_vertexShader);
		SafeRelease(_textures);
	}

	void Material::SetDepthWriteEnabled(bool depthWrite)
	{
		_depthWriteEnabled = depthWrite;
	}
	void Material::SetDepthMode(DepthMode mode)
	{
		_depthMode = mode;
	}

	void Material::SetFragmentBuffers(const Array *buffers)
	{
		SafeRelease(_fragmentBuffers);
		_fragmentBuffers = SafeCopy(buffers);
	}
	void Material::SetVertexBuffers(const Array *buffers)
	{
		SafeRelease(_vertexBuffers);
		_vertexBuffers = SafeCopy(buffers);
	}

	void Material::SetDiscardThreshold(float threshold)
	{
		_discardThreshold = threshold;
	}

	void Material::SetTextureTileFactor(float factor)
	{
		_textureTileFactor = factor;
	}

	void Material::SetAmbientColor(const Color &color)
	{
		_ambientColor = color;
	}
	void Material::SetDiffuseColor(const Color &color)
	{
		_diffuseColor = color;
	}
	void Material::SetSpecularColor(const Color &color)
	{
		_specularColor = color;
	}
	void Material::SetEmissiveColor(const Color &color)
	{
		_emissiveColor = color;
	}

	void Material::SetCullMode(CullMode mode)
	{
		_cullMode = mode;
	}

	MaterialDescriptor Material::GetDescriptor() const
	{
		MaterialDescriptor descriptor;
		descriptor.SetTextures(GetTextures());
		descriptor.fragmentShader = _fragmentShader;
		descriptor.vertexShader = _vertexShader;
		descriptor.depthMode = _depthMode;
		descriptor.depthWriteEnabled = _depthWriteEnabled;
		descriptor.ambientColor = _ambientColor;
		descriptor.diffuseColor = _diffuseColor;
		descriptor.specularColor = _specularColor;
		descriptor.emissiveColor = _emissiveColor;
		descriptor.discardThreshold = _discardThreshold;
		descriptor.textureTileFactor = _textureTileFactor;
		descriptor.cullMode = _cullMode;

		return descriptor;
	}
}