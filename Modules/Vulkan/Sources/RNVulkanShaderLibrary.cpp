//
//  RNVulkanShaderLibrary.cpp
//  Rayne
//
//  Copyright 2016 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNVulkanShaderLibrary.h"
#include "RNVulkanShader.h"
#include "RNVulkanRenderer.h"

namespace RN
{
	RNDefineMeta(VulkanSpecificShaderLibrary, Object)
	RNDefineMeta(VulkanShaderLibrary, ShaderLibrary)


	VulkanSpecificShaderLibrary::VulkanSpecificShaderLibrary(const String *fileName, const String *entryPoint, Shader::Type type, Dictionary *signatureDescription) :
		_shaders(new Dictionary()),
		_fileName(fileName->Retain()),
		_entryPoint(entryPoint->Retain()),
		_type(type),
		_signatureDescription(signatureDescription)
	{
		if(_signatureDescription) _signatureDescription->Retain();
	}

	VulkanSpecificShaderLibrary::~VulkanSpecificShaderLibrary()
	{
		_fileName->Release();
		_entryPoint->Release();
		_shaders->Release();
		_signatureDescription->Release();
	}

	const Shader::Options *VulkanSpecificShaderLibrary::GetCleanedShaderOptions(const Shader::Options *options) const
	{
		const Dictionary *oldDefines = options->GetDefines();
		Shader::Options *newOptions = Shader::Options::WithNone();
		if(!_signatureDescription)
			return newOptions;

		Array *signatureOptions = _signatureDescription->GetObjectForKey<Array>(RNCSTR("options"));
		if(!signatureOptions)
			return newOptions;

		signatureOptions->Enumerate([&](Object *option, size_t index, bool &stop) {
			Dictionary *dict = option->Downcast<Dictionary>();
			String *name = nullptr;
			if(!dict)
			{
				name = option->Downcast<String>();
			}
			else
			{
				name = dict->GetObjectForKey<String>(RNCSTR("option"));
			}

			if(name)
			{
				String *obj = oldDefines->GetObjectForKey<String>(name);
				if(obj)
				{
					newOptions->AddDefine(name, obj);
				}
			}
		});

		return newOptions;
	}

	size_t VulkanSpecificShaderLibrary::GetPermutationIndexForOptions(const Shader::Options *options) const
	{
		if(!options || !_signatureDescription) return 0;

		Array *signatureOptions = _signatureDescription->GetObjectForKey<Array>(RNCSTR("options"));
		if(!signatureOptions) return 0;

		const Dictionary *oldDefines = options->GetDefines();
		size_t permutationIndex = 0;
		signatureOptions->Enumerate([&](Object *option, size_t index, bool &stop) {
			Dictionary *dict = option->Downcast<Dictionary>();
			String *name = nullptr;
			if(!dict)
			{
				name = option->Downcast<String>();
			}
			else
			{
				name = dict->GetObjectForKey<String>(RNCSTR("option"));
			}

			if(name)
			{
				String *obj = oldDefines->GetObjectForKey<String>(name);
				if(obj)
				{
					permutationIndex |= (static_cast<size_t>(1) << index);
				}
			}
		});

		return permutationIndex;
	}

	static Array *GetUniformDescriptors(const Array *uniforms, uint32 &offset)
	{
		Array *uniformDescriptors = new Array();
		if(uniforms)
		{
			uniforms->Enumerate([&](Object *uniform, size_t index, bool &stop) {
				String *name = uniform->Downcast<String>();
				if(!name)
				{
					Dictionary *dict = uniform->Downcast<Dictionary>();
					if(dict)
					{
						//TODO: WAHHHHH implement custom uniforms
					}
				}
				else
				{
					Shader::UniformDescriptor *descriptor = new Shader::UniformDescriptor(name, offset);
					offset += descriptor->GetSize();
					uniformDescriptors->AddObject(descriptor->Autorelease());
				}
			});
		}

		return uniformDescriptors->Autorelease();
	}

	static Array *GetSamplers(const Array *samplers)
	{
		Array *samplerArray = new Array();
		if(samplers)
		{
			samplers->Enumerate([&](Object *sampler, size_t index, bool &stop) {
				String *name = sampler->Downcast<String>();
				if(!name)
				{
					Dictionary *dict = sampler->Downcast<Dictionary>();
					if(dict)
					{
						String *wrap = dict->GetObjectForKey<String>(RNCSTR("wrap"));
						String *filter = dict->GetObjectForKey<String>(RNCSTR("filter"));
						Number *anisotropy = dict->GetObjectForKey<Number>(RNCSTR("anisotropy"));

						Shader::Sampler::WrapMode wrapMode = Shader::Sampler::WrapMode::Repeat;
						Shader::Sampler::Filter filterType = Shader::Sampler::Filter::Anisotropic;
						uint8 anisotropyValue = Shader::Sampler::GetDefaultAnisotropy();

						if(wrap)
						{
							if(wrap->IsEqual(RNCSTR("clamp")))
							{
								wrapMode = Shader::Sampler::WrapMode::Clamp;
							}
						}

						if(filter)
						{
							if(filter->IsEqual(RNCSTR("nearest")))
							{
								filterType = Shader::Sampler::Filter::Nearest;
							}
							else if(filter->IsEqual(RNCSTR("linear")))
							{
								filterType = Shader::Sampler::Filter::Linear;
							}
						}

						if(anisotropy)
						{
							anisotropyValue = anisotropy->GetUint32Value();
						}

						//TODO: read comparison function from json
						Shader::Sampler *sampler = new Shader::Sampler(wrapMode, filterType, Shader::Sampler::ComparisonFunction::Never, anisotropyValue);
						samplerArray->AddObject(sampler->Autorelease());
					}
				}
				else
				{
					if(name->IsEqual(RNCSTR("default")))
					{
						Shader::Sampler *sampler = new Shader::Sampler();
						samplerArray->AddObject(sampler->Autorelease());
					}
				}
			});
		}

		return samplerArray->Autorelease();
	}

	const Array *VulkanSpecificShaderLibrary::GetSamplerSignature(const Shader::Options *options) const
	{
		if(!_signatureDescription)
		{
			Array *samplers = new Array();
			return samplers->Autorelease();
		}

		Array *samplerDataArray = _signatureDescription->GetObjectForKey<Array>(RNCSTR("samplers"));
		Array *samplerArray = GetSamplers(samplerDataArray);

		Array *signatureOptions = _signatureDescription->GetObjectForKey<Array>(RNCSTR("options"));
		if(signatureOptions)
		{
			signatureOptions->Enumerate([&](Object *option, size_t index, bool &stop) {
				Dictionary *dict = option->Downcast<Dictionary>();
				if(dict)
				{
					String *name = dict->GetObjectForKey<String>(RNCSTR("option"));
					if(!options->GetDefines()->GetObjectForKey(name))
					{
						return;
					}
					Array *optionSamplerdataArray = dict->GetObjectForKey<Array>(RNCSTR("samplers"));
					Array *optionSamplerArray = GetSamplers(optionSamplerdataArray);
					samplerArray->AddObjectsFromArray(optionSamplerArray);
				}
			});
		}

		return samplerArray;
	}

	Shader *VulkanSpecificShaderLibrary::GetShaderWithOptions(ShaderLibrary *library, const Shader::Options *options)
	{
		const Shader::Options *newOptions = GetCleanedShaderOptions(options);

		VulkanShader *shader = _shaders->GetObjectForKey<VulkanShader>(newOptions);
		if(shader)
			return shader;

		size_t permutationIndex = GetPermutationIndexForOptions(newOptions);
		RN::String *permutationFileName = _fileName->StringByDeletingPathExtension();
		permutationFileName->Append(RNSTR("." << permutationIndex));
		permutationFileName->Append(".spirv");

		//TODO: Fix things to make this work on android!
		RN::String *filePath = permutationFileName;//RN::FileManager::GetSharedInstance()->ResolveFullPath(permutationFileName, 0);

		const Array *samplers = GetSamplerSignature(newOptions);
		shader = new VulkanShader(library, filePath, _entryPoint, _type, newOptions, samplers);
		_shaders->SetObjectForKey(shader, newOptions);

		return shader->Autorelease();
	}


	VulkanShaderLibrary::VulkanShaderLibrary(const String *file) : _specificShaderLibraries(new Dictionary())
	{
		Data *data = Data::WithContentsOfFile(file);
		if(!data)
			throw InvalidArgumentException(RNSTR("Could not open file: " << file));

		Array *mainArray = JSONSerialization::ObjectFromData<Array>(data, 0);
		mainArray->Enumerate<Dictionary>([&](Dictionary *libraryDictionary, size_t index, bool &stop) {
			String *fileString = libraryDictionary->GetObjectForKey<String>(RNCSTR("file~vulkan"));
			if(!fileString)
				fileString = libraryDictionary->GetObjectForKey<String>(RNCSTR("file"));

			if(!fileString)
				return;

			Array *shadersArray = libraryDictionary->GetObjectForKey<Array>(RNCSTR("shaders"));
			shadersArray->Enumerate<Dictionary>([&](Dictionary *shaderDictionary, size_t index, bool &stop) {
				String *entryPointName = shaderDictionary->GetObjectForKey<String>(RNCSTR("name"));
				String *shaderType = shaderDictionary->GetObjectForKey<String>(RNCSTR("type"));
				Dictionary *signature = shaderDictionary->GetObjectForKey<Dictionary>(RNCSTR("signature"));

				Shader::Type type = Shader::Type::Vertex;
				if(shaderType->IsEqual(RNCSTR("vertex")))
				{
					type = Shader::Type::Vertex;
				}
				else if(shaderType->IsEqual(RNCSTR("fragment")))
				{
					type = Shader::Type::Fragment;
				}
				else if(shaderType->IsEqual(RNCSTR("compute")))
				{
					type = Shader::Type::Compute;
				}
				else
				{
					RN_ASSERT(false, "Unknown shader type %s for %s in library %s.", shaderType, entryPointName, file);
				}

				VulkanSpecificShaderLibrary *specificLibrary = new VulkanSpecificShaderLibrary(fileString, entryPointName, type, signature);
				_specificShaderLibraries->SetObjectForKey(specificLibrary, entryPointName);
			});
		});
	}

	VulkanShaderLibrary::~VulkanShaderLibrary()
	{
		_specificShaderLibraries->Release();
	}

	Shader *VulkanShaderLibrary::GetShaderWithName(const String *name, const Shader::Options *options)
	{
		VulkanSpecificShaderLibrary *specificLibrary = _specificShaderLibraries->GetObjectForKey<VulkanSpecificShaderLibrary>(name);
		if(!specificLibrary)
			return nullptr;

		if(options)
			return specificLibrary->GetShaderWithOptions(this, options);

		return specificLibrary->GetShaderWithOptions(this, Shader::Options::WithNone());
	}

	Shader *VulkanShaderLibrary::GetInstancedShaderForShader(Shader *shader)
	{
		return nullptr;
	}
}
