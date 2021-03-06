//
//  RND3D12StateCoordinator.cpp
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "d3dx12.h"
#include "RND3D12StateCoordinator.h"
#include "RND3D12Renderer.h"
#include "RND3D12Framebuffer.h"
#include "RND3D12UniformBuffer.h"

namespace RN
{
	D3D12_BLEND_OP _blendOpLookup[] =
	{
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_OP_SUBTRACT,
		D3D12_BLEND_OP_REV_SUBTRACT,
		D3D12_BLEND_OP_MIN,
		D3D12_BLEND_OP_MAX
	};

	D3D12_BLEND _blendFactorLookup[] =
	{
		D3D12_BLEND_ZERO,
		D3D12_BLEND_ONE,
		D3D12_BLEND_SRC_COLOR,
		D3D12_BLEND_INV_SRC_COLOR,
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND_INV_DEST_COLOR,
		D3D12_BLEND_DEST_ALPHA,
		D3D12_BLEND_INV_DEST_ALPHA,
		D3D12_BLEND_SRC_ALPHA_SAT,
		D3D12_BLEND_SRC1_COLOR,
		D3D12_BLEND_INV_SRC1_COLOR,
		D3D12_BLEND_SRC1_ALPHA,
		D3D12_BLEND_INV_SRC1_ALPHA
	};

	DXGI_FORMAT _vertexFormatLookup[] =
	{
		DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R32_UINT,
		
		DXGI_FORMAT_R8_SINT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R32_SINT,
		
		DXGI_FORMAT_R32_FLOAT,
		
		DXGI_FORMAT_R32G32_FLOAT,
		DXGI_FORMAT_R32G32B32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT
	};

	const char* _vertexFeatureLookup[]
	{
		"POSITION",
		"NORMAL",
		"TANGENT",
		"COLOR",
		"COLOR",
		"TEXCOORD",
		"TEXCOORD",

		"INDEX",

		"BONEWEIGHTS",
		"BONEINDICES",

		"CUSTOM"
	};

	uint8 _vertexFeatureIndexLookup[]
	{
		0,
		0,
		0,
		0,
		1,
		0,
		1,

		0,

		0,
		0,

		0
	};

	D3D12RootSignature::~D3D12RootSignature()
	{
		signature->Release();
	}

	D3D12PipelineState::~D3D12PipelineState()
	{
		state->Release();
	}

	D3D12StateCoordinator::D3D12StateCoordinator() :
		_lastDepthStencilState(nullptr)
	{}

	D3D12StateCoordinator::~D3D12StateCoordinator()
	{
		//TODO: Clean up correctly...
	}

	const D3D12RootSignature *D3D12StateCoordinator::GetRootSignature(const D3D12PipelineStateDescriptor &descriptor)
	{
		D3D12Shader *vertexShader = static_cast<D3D12Shader *>(descriptor.vertexShader);
		const Shader::Signature *vertexSignature = vertexShader->GetSignature();
		uint16 textureCount = vertexSignature->GetTextureCount();
		const Array *vertexSamplers = vertexSignature->GetSamplers();
		Array *samplerArray = new Array(vertexSamplers);
		samplerArray->Autorelease();

		bool wantsDirectionalShadowTexture = vertexShader->_wantsDirectionalShadowTexture;

		//TODO: Support multiple constant buffers per function signature
		uint16 constantBufferCount = (vertexSignature->GetTotalUniformSize() > 0) ? 1 : 0;

		D3D12Shader *fragmentShader = static_cast<D3D12Shader *>(descriptor.fragmentShader);
		if(fragmentShader)
		{
			const Shader::Signature *fragmentSignature = fragmentShader->GetSignature();
			textureCount = fmax(textureCount, fragmentSignature->GetTextureCount());
			const Array *fragmentSamplers = fragmentSignature->GetSamplers();
			samplerArray->AddObjectsFromArray(fragmentSamplers);

			wantsDirectionalShadowTexture = (wantsDirectionalShadowTexture || fragmentShader->_wantsDirectionalShadowTexture);

			//TODO: Support multiple constant buffers per function signature
			constantBufferCount += (fragmentSignature->GetTotalUniformSize() > 0) ? 1 : 0;
		}

		for(D3D12RootSignature *signature : _rootSignatures)
		{
			
			if(signature->textureCount != textureCount)
			{
				continue;
			}

			//TODO: Doesn't really require an extra root signature...
			if(signature->wantsDirectionalShadowTexture != wantsDirectionalShadowTexture)
			{
				continue;
			}

			if(signature->constantBufferCount != constantBufferCount)
			{
				continue;
			}

			if(samplerArray->GetCount() != signature->samplers->GetCount())
			{
				continue;
			}
			
			bool notEqual = false;
			signature->samplers->Enumerate<Shader::Sampler>([&](Shader::Sampler *sampler, size_t index, bool &stop) {
				if(!(sampler == samplerArray->GetObjectAtIndex(index)))
				{
					notEqual = true;
					stop = true;
				}
			});
			if(notEqual)
			{
				continue;
			}

			return signature;
		}

		D3D12Renderer *renderer = static_cast<D3D12Renderer *>(Renderer::GetActiveRenderer());

		D3D12RootSignature *signature = new D3D12RootSignature();
		signature->constantBufferCount = constantBufferCount;
		signature->samplers = samplerArray->Retain();
		signature->textureCount = textureCount;
		signature->wantsDirectionalShadowTexture = wantsDirectionalShadowTexture;

		int numberOfTables = (signature->textureCount > 0) + (signature->constantBufferCount > 0);

		CD3DX12_DESCRIPTOR_RANGE *srvCbvRanges = nullptr;
		CD3DX12_ROOT_PARAMETER *rootParameters = nullptr;

		if(numberOfTables > 0)
		{
			srvCbvRanges = new CD3DX12_DESCRIPTOR_RANGE[numberOfTables];
			rootParameters = new CD3DX12_ROOT_PARAMETER[numberOfTables];
		}

		// Perfomance TIP: Order from most frequent to least frequent.
		int tableIndex = 0;
		if(signature->textureCount > 0)
		{
			srvCbvRanges[tableIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, signature->textureCount, 0, 0);// , D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);
			rootParameters[tableIndex].InitAsDescriptorTable(1, &srvCbvRanges[tableIndex], D3D12_SHADER_VISIBILITY_ALL);	//TODO: Restrict visibility to the shader actually using it
			tableIndex += 1;
		}
		if(signature->constantBufferCount > 0)
		{
			srvCbvRanges[tableIndex].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, signature->constantBufferCount, 0, 0);// , D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE);
			rootParameters[tableIndex].InitAsDescriptorTable(1, &srvCbvRanges[tableIndex], D3D12_SHADER_VISIBILITY_ALL);	//TODO: Restrict visibility to the shader actually using it
			tableIndex += 1;
		}
		
		// Create samplers
		D3D12_STATIC_SAMPLER_DESC *samplerDescriptors = nullptr;
		if(signature->samplers->GetCount() > 0)
		{
			samplerDescriptors = new D3D12_STATIC_SAMPLER_DESC[signature->samplers->GetCount()];
			signature->samplers->Enumerate<Shader::Sampler>([&](Shader::Sampler *sampler, size_t index, bool &stop) {
				D3D12_STATIC_SAMPLER_DESC &samplerDesc = samplerDescriptors[index];

				D3D12_FILTER filter = D3D12_FILTER_ANISOTROPIC;
				D3D12_COMPARISON_FUNC comparisonFunction = D3D12_COMPARISON_FUNC_NEVER;
				if(sampler->GetComparisonFunction() == Shader::Sampler::ComparisonFunction::Never)
				{
					switch (sampler->GetFilter())
					{
					case Shader::Sampler::Filter::Anisotropic:
						filter = D3D12_FILTER_ANISOTROPIC;
						break;
					case Shader::Sampler::Filter::Linear:
						filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
						break;
					case Shader::Sampler::Filter::Nearest:
						filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
						break;
					}
				}
				else
				{
					switch (sampler->GetFilter())
					{
					case Shader::Sampler::Filter::Anisotropic:
						filter = D3D12_FILTER_COMPARISON_ANISOTROPIC;
						break;
					case Shader::Sampler::Filter::Linear:
						filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
						break;
					case Shader::Sampler::Filter::Nearest:
						filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
						break;
					}

					switch(sampler->GetComparisonFunction())
					{
					case Shader::Sampler::ComparisonFunction::Always:
						comparisonFunction = D3D12_COMPARISON_FUNC_ALWAYS;
						break;
					case Shader::Sampler::ComparisonFunction::Equal:
						comparisonFunction = D3D12_COMPARISON_FUNC_EQUAL;
						break;
					case Shader::Sampler::ComparisonFunction::GreaterEqual:
						comparisonFunction = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
						break;
					case Shader::Sampler::ComparisonFunction::Greater:
						comparisonFunction = D3D12_COMPARISON_FUNC_GREATER;
						break;
					case Shader::Sampler::ComparisonFunction::Less:
						comparisonFunction = D3D12_COMPARISON_FUNC_LESS;
						break;
					case Shader::Sampler::ComparisonFunction::LessEqual:
						comparisonFunction = D3D12_COMPARISON_FUNC_LESS_EQUAL;
						break;
					case Shader::Sampler::ComparisonFunction::NotEqual:
						comparisonFunction = D3D12_COMPARISON_FUNC_NOT_EQUAL;
						break;
					}
				}

				D3D12_TEXTURE_ADDRESS_MODE addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
				switch(sampler->GetWrapMode())
				{
				case Shader::Sampler::WrapMode::Repeat:
					addressMode = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
					break;
				case Shader::Sampler::WrapMode::Clamp:
					addressMode = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
					break;
				}

				samplerDesc.Filter = filter;
				samplerDesc.AddressU = addressMode;
				samplerDesc.AddressV = addressMode;
				samplerDesc.AddressW = addressMode;
				samplerDesc.MipLODBias = 0.0f;
				samplerDesc.ComparisonFunc = comparisonFunction;
				samplerDesc.MinLOD = 0.0f;
				samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
				samplerDesc.MaxAnisotropy = sampler->GetAnisotropy();
				samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
				samplerDesc.ShaderRegister = index;
				samplerDesc.RegisterSpace = 0;
				samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;	//TODO: Restrict visibility to the shader actually using it
			});
		}

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(numberOfTables, rootParameters, signature->samplers->GetCount(), samplerDescriptors, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

/*		D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
		//If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

		if(FAILED(renderer->GetD3D12Device()->GetDevice()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
		{
			featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
		}*/

		ID3DBlob *signatureBlob = nullptr;
		ID3DBlob *error = nullptr;
		HRESULT success = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &error);

		if(FAILED(success))
		{
			String *errorString = RNCSTR("");
			if(error)
			{
				errorString = RNSTR((char*)error->GetBufferPointer());
				error->Release();
			}

			RNDebug(RNSTR("Failed to create root signature with error: " << errorString));
		}

		renderer->GetD3D12Device()->GetDevice()->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&signature->signature));

		if (srvCbvRanges)
			delete[] srvCbvRanges;
		if (rootParameters)
			delete[] rootParameters;
		if (samplerDescriptors)
			delete[] samplerDescriptors;

		_rootSignatures.push_back(signature);
		return signature;
	}

	const D3D12PipelineState *D3D12StateCoordinator::GetRenderPipelineState(Material *material, Mesh *mesh, D3D12Framebuffer *framebuffer, Shader::UsageHint shaderHint, Material *overrideMaterial)
	{
		const Mesh::VertexDescriptor &descriptor = mesh->GetVertexDescriptor();
		Material::Properties mergedMaterialProperties = material->GetMergedProperties(overrideMaterial);
		D3D12PipelineStateDescriptor pipelineDescriptor;
		pipelineDescriptor.sampleCount = framebuffer->GetSampleCount();
		pipelineDescriptor.sampleQuality = (framebuffer->_colorTargets.size() > 0 && !framebuffer->GetSwapChain()) ? framebuffer->_colorTargets[0]->targetView.texture->GetDescriptor().sampleQuality : 0;

		for(D3D12Framebuffer::D3D12ColorTargetView *targetView : framebuffer->_colorTargets)
		{
			//TODO: Fix this bullshit
			if(targetView->d3dTargetViewDesc.Format == DXGI_FORMAT_UNKNOWN)
			{
				pipelineDescriptor.colorFormats.push_back(targetView->d3dTargetViewDesc.Format);
			}
			else
			{
				pipelineDescriptor.colorFormats.push_back(targetView->d3dTargetViewDesc.Format);
			}
		}
		pipelineDescriptor.depthStencilFormat = (framebuffer->_depthStencilTarget) ? framebuffer->_depthStencilTarget->d3dTargetViewDesc.Format : DXGI_FORMAT_UNKNOWN;
		pipelineDescriptor.shaderHint = shaderHint;
		pipelineDescriptor.vertexShader = (overrideMaterial && !(overrideMaterial->GetOverride() & Material::Override::GroupShaders) && !(material->GetOverride() & Material::Override::GroupShaders))? overrideMaterial->GetVertexShader(pipelineDescriptor.shaderHint) : material->GetVertexShader(pipelineDescriptor.shaderHint);
		pipelineDescriptor.fragmentShader = (overrideMaterial && !(overrideMaterial->GetOverride() & Material::Override::GroupShaders) && !(material->GetOverride() & Material::Override::GroupShaders)) ? overrideMaterial->GetFragmentShader(pipelineDescriptor.shaderHint) : material->GetFragmentShader(pipelineDescriptor.shaderHint);
		pipelineDescriptor.cullMode = mergedMaterialProperties.cullMode;
		pipelineDescriptor.usePolygonOffset = mergedMaterialProperties.usePolygonOffset;
		pipelineDescriptor.polygonOffsetFactor = mergedMaterialProperties.polygonOffsetFactor;
		pipelineDescriptor.polygonOffsetUnits = mergedMaterialProperties.polygonOffsetUnits;
		pipelineDescriptor.blendOperationRGB = mergedMaterialProperties.blendOperationRGB;
		pipelineDescriptor.blendOperationAlpha = mergedMaterialProperties.blendOperationAlpha;
		pipelineDescriptor.blendFactorSourceRGB = mergedMaterialProperties.blendFactorSourceRGB;
		pipelineDescriptor.blendFactorSourceAlpha = mergedMaterialProperties.blendFactorSourceAlpha;
		pipelineDescriptor.blendFactorDestinationRGB = mergedMaterialProperties.blendFactorDestinationRGB;
		pipelineDescriptor.blendFactorDestinationAlpha = mergedMaterialProperties.blendFactorDestinationAlpha;
		pipelineDescriptor.useAlphaToCoverage = mergedMaterialProperties.useAlphaToCoverage;
		pipelineDescriptor.colorWriteMask = mergedMaterialProperties.colorWriteMask;
		pipelineDescriptor.depthMode = mergedMaterialProperties.depthMode;
		pipelineDescriptor.depthWriteEnabled = mergedMaterialProperties.depthWriteEnabled;
		//TODO: Support all override flags and all the relevant material properties

		for(D3D12PipelineStateCollection *collection : _renderingStates)
		{
			if(collection->descriptor.IsEqual(descriptor))
			{
				if(collection->vertexShader == pipelineDescriptor.vertexShader && collection->fragmentShader == pipelineDescriptor.fragmentShader)
				{
					return GetRenderPipelineStateInCollection(collection, mesh, pipelineDescriptor);
				}
			}
		}

		D3D12PipelineStateCollection *collection = new D3D12PipelineStateCollection(descriptor, pipelineDescriptor.vertexShader, pipelineDescriptor.fragmentShader);
		_renderingStates.push_back(collection);

		return GetRenderPipelineStateInCollection(collection, mesh, pipelineDescriptor);
	}

	const D3D12PipelineState *D3D12StateCoordinator::GetRenderPipelineStateInCollection(D3D12PipelineStateCollection *collection, Mesh *mesh, const D3D12PipelineStateDescriptor &descriptor)
	{
		const D3D12RootSignature *rootSignature = GetRootSignature(descriptor);

		//TODO: Make sure all possible cases are covered... Depth bias for example... cullmode...
		for(const D3D12PipelineState *state : collection->states)
		{
			if(state->descriptor.colorFormats == descriptor.colorFormats && state->descriptor.sampleCount == descriptor.sampleCount && state->descriptor.sampleQuality == descriptor.sampleQuality && state->descriptor.depthStencilFormat == descriptor.depthStencilFormat && rootSignature->signature == state->rootSignature->signature)
			{
				if(state->descriptor.cullMode == descriptor.cullMode && state->descriptor.usePolygonOffset == descriptor.usePolygonOffset && state->descriptor.polygonOffsetFactor == descriptor.polygonOffsetFactor && state->descriptor.polygonOffsetUnits == descriptor.polygonOffsetUnits && state->descriptor.depthMode == descriptor.depthMode && state->descriptor.depthWriteEnabled == descriptor.depthWriteEnabled)
				{
					if(state->descriptor.blendOperationRGB == descriptor.blendOperationRGB && state->descriptor.blendOperationAlpha == descriptor.blendOperationAlpha && state->descriptor.blendFactorSourceRGB == descriptor.blendFactorSourceRGB && state->descriptor.blendFactorSourceAlpha == descriptor.blendFactorSourceAlpha && state->descriptor.blendFactorDestinationRGB == descriptor.blendFactorDestinationRGB && state->descriptor.blendFactorDestinationAlpha == descriptor.blendFactorDestinationAlpha && state->descriptor.useAlphaToCoverage == descriptor.useAlphaToCoverage && state->descriptor.colorWriteMask == descriptor.colorWriteMask)
						return state;
				}
			}
		}

		D3D12Renderer *renderer = static_cast<D3D12Renderer *>(Renderer::GetActiveRenderer());

		D3D12Shader *vertexShaderRayne = collection->vertexShader->Downcast<D3D12Shader>();
		D3D12Shader *fragmentShaderRayne = collection->fragmentShader->Downcast<D3D12Shader>();
		ID3DBlob *vertexShader = vertexShaderRayne?static_cast<ID3DBlob*>(vertexShaderRayne->_shader) : nullptr;
		ID3DBlob *fragmentShader = fragmentShaderRayne ? static_cast<ID3DBlob*>(fragmentShaderRayne->_shader) : nullptr;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.pRootSignature = rootSignature->signature;
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs = CreateVertexElementDescriptorsFromMesh(mesh);
		psoDesc.InputLayout = { inputElementDescs.data(), static_cast<UINT>(inputElementDescs.size()) };
		psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
		if(fragmentShader)
			psoDesc.PS = { reinterpret_cast<UINT8*>(fragmentShader->GetBufferPointer()), fragmentShader->GetBufferSize() };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.RasterizerState.FrontCounterClockwise = true;

		if(descriptor.usePolygonOffset)
		{
			psoDesc.RasterizerState.DepthBias = descriptor.polygonOffsetUnits;
			psoDesc.RasterizerState.SlopeScaledDepthBias = descriptor.polygonOffsetFactor;
			//psoDesc.RasterizerState.DepthBiasClamp = D3D12_FLOAT32_MAX;
		}

		switch(descriptor.cullMode)
		{
		case CullMode::BackFace:
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			break;
		case CullMode::FrontFace:
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
			break;
		case CullMode::None:
			psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			break;
		}
		
		D3D12_BLEND_DESC blendDescription;
		blendDescription.AlphaToCoverageEnable = descriptor.useAlphaToCoverage ? TRUE : FALSE;
		blendDescription.IndependentBlendEnable = TRUE;

		D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc;
		renderTargetBlendDesc.BlendEnable = (descriptor.blendOperationRGB != BlendOperation::None && descriptor.blendOperationAlpha != BlendOperation::None);
		renderTargetBlendDesc.LogicOpEnable = FALSE;
		renderTargetBlendDesc.SrcBlend = _blendFactorLookup[static_cast<int>(descriptor.blendFactorSourceRGB)];
		renderTargetBlendDesc.DestBlend = _blendFactorLookup[static_cast<int>(descriptor.blendFactorDestinationRGB)];
		renderTargetBlendDesc.BlendOp = _blendOpLookup[static_cast<int>(descriptor.blendOperationRGB)];
		renderTargetBlendDesc.SrcBlendAlpha = _blendFactorLookup[static_cast<int>(descriptor.blendFactorSourceAlpha)];
		renderTargetBlendDesc.DestBlendAlpha = _blendFactorLookup[static_cast<int>(descriptor.blendFactorDestinationAlpha)];
		renderTargetBlendDesc.BlendOpAlpha = _blendOpLookup[static_cast<int>(descriptor.blendOperationAlpha)];
		renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		renderTargetBlendDesc.RenderTargetWriteMask = descriptor.colorWriteMask;

		for(UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
		{
			blendDescription.RenderTarget[i] = renderTargetBlendDesc;
		}

		psoDesc.BlendState = blendDescription;

		if(descriptor.depthStencilFormat != DXGI_FORMAT_UNKNOWN)
		{
			psoDesc.DepthStencilState.DepthEnable = TRUE;
			switch(descriptor.depthMode)
			{
			case DepthMode::Never:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;
				break;

			case DepthMode::Always:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				break;

			case DepthMode::Less:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				break;

			case DepthMode::LessOrEqual:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
				break;

			case DepthMode::Equal:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
				break;

			case DepthMode::NotEqual:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NOT_EQUAL;
				break;

			case DepthMode::GreaterOrEqual:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
				break;

			case DepthMode::Greater:
				psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER;
				break;
			}
			
			psoDesc.DepthStencilState.DepthWriteMask = descriptor.depthWriteEnabled?D3D12_DEPTH_WRITE_MASK_ALL: D3D12_DEPTH_WRITE_MASK_ZERO;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
			psoDesc.DSVFormat = descriptor.depthStencilFormat;
		}
		else
		{
			psoDesc.DepthStencilState.DepthEnable = FALSE;
			psoDesc.DepthStencilState.StencilEnable = FALSE;
		}
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = descriptor.colorFormats.size();
		int counter = 0;
		for(DXGI_FORMAT format : descriptor.colorFormats)
		{
			psoDesc.RTVFormats[counter++] = format;
			if(counter >= 8)
				break;
		}
		psoDesc.SampleDesc.Count = descriptor.sampleCount;
		psoDesc.SampleDesc.Quality = descriptor.sampleQuality;

		D3D12PipelineState *state = new D3D12PipelineState();
		state->descriptor = std::move(descriptor);
		state->rootSignature = rootSignature;
		HRESULT success = renderer->GetD3D12Device()->GetDevice()->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&state->state));

		if(FAILED(success))
		{
			return nullptr;
		}

		collection->states.push_back(state);
		return state;
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> D3D12StateCoordinator::CreateVertexElementDescriptorsFromMesh(Mesh *mesh)
	{
		const std::vector<Mesh::VertexAttribute> &attributes = mesh->GetVertexAttributes();
		std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescs;

		for(const Mesh::VertexAttribute &attribute : attributes)
		{
			if(attribute.GetFeature() == Mesh::VertexAttribute::Feature::Indices)
				continue;

			//TODO: support multiple texcoords/other stuff
			D3D12_INPUT_ELEMENT_DESC element = {};
			element.SemanticName = _vertexFeatureLookup[static_cast<int>(attribute.GetFeature())];
			element.SemanticIndex = _vertexFeatureIndexLookup[static_cast<int>(attribute.GetFeature())];
			element.Format = _vertexFormatLookup[static_cast<int>(attribute.GetType())];
			element.InputSlot = 0;
			element.AlignedByteOffset = attribute.GetOffset();
			element.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			element.InstanceDataStepRate = 0;
			inputElementDescs.push_back(element);
		}

		return inputElementDescs;
	}

	D3D12UniformState *D3D12StateCoordinator::GetUniformStateForPipelineState(const D3D12PipelineState *pipelineState)
	{
		D3D12Renderer *renderer = static_cast<D3D12Renderer *>(Renderer::GetActiveRenderer());

		Shader *vertexShader = pipelineState->descriptor.vertexShader;
		Shader *fragmentShader = pipelineState->descriptor.fragmentShader;
		D3D12UniformBufferReference *vertexBuffer = nullptr;
		D3D12UniformBufferReference *fragmentBuffer = nullptr;
		if(vertexShader && vertexShader->GetSignature() && vertexShader->GetSignature()->GetTotalUniformSize())
		{
			vertexBuffer = renderer->GetUniformBufferReference(vertexShader->GetSignature()->GetTotalUniformSize(), 0);
		}
		if(fragmentShader && fragmentShader->GetSignature() && fragmentShader->GetSignature()->GetTotalUniformSize())
		{
			fragmentBuffer = renderer->GetUniformBufferReference(fragmentShader->GetSignature()->GetTotalUniformSize(), 0);
		}

		D3D12UniformState *state = new D3D12UniformState();
		state->vertexUniformBuffer = SafeRetain(vertexBuffer);
		state->fragmentUniformBuffer = SafeRetain(fragmentBuffer);

		return state;
	}
}
