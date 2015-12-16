//
//  Shaders.metal
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include <metal_stdlib>
#include <simd/simd.h>

//#define vertex
//#define fragment
//#define constant const

// Exported defines:
// RN_NORMALS
// RN_COLOR
// RN_UV0
// RN_DISCARD

using namespace metal;

#define RN_FRAGMENT_UNIFORM 0

#if RN_DISCARD
#undef RN_FRAGMENT_UNIFORM
#define RN_FRAGMENT_UNIFORM 1
#endif

#ifndef RN_NORMALS
#define RN_NORMALS 0
#endif

#ifndef RN_COLOR
#define RN_COLOR 0
#endif

#ifndef RN_UV0
#define RN_UV0 0
#endif

// Variables in constant address space
constant float3 light_position = float3(1.0, 1.0, 1.0);

struct Uniforms
{
	matrix_float4x4 viewProjectionMatrix;
	matrix_float4x4 modelViewProjectionMatrix;

#if RN_NORMALS
	matrix_float4x4 modelMatrix;
#endif

	float4 ambientColor;
	float4 diffuseColor;

#if RN_UV0
	float textureTileFactor;
#endif
};

#if RN_FRAGMENT_UNIFORM
struct FragmentUniforms
{
#if RN_DISCARD
	float discardThreshold;
#endif
};
#endif

struct InputVertex
{
	float3 position [[attribute(0)]];

#if RN_NORMALS
	float3 normal [[attribute(1)]];
#endif
#if RN_COLOR
	float4 color [[attribute(1 + RN_NORMALS)]];
#endif
#if RN_UV0
	float2 texCoords [[attribute(1 + RN_NORMALS + RN_COLOR)]];
#endif
#if RN_TANGENTS
	float4 tangents [[attribute(1 + RN_NORMALS + RN_COLOR + RN_UV0)]];
#endif
};

struct FragmentVertex
{
	float4 position [[position]];

#if RN_COLOR
	float4 color;
#endif
#if RN_NORMALS
	float3 normal;
#endif
#if RN_UV0
	float2 texCoords;
#endif

	float4 ambient;
	float4 diffuse;
};

// Non instanced

vertex FragmentVertex gouraud_vertex(InputVertex vert [[stage_in]], constant Uniforms &uniforms [[buffer(1)]])
{
	FragmentVertex result;

	result.position = uniforms.modelViewProjectionMatrix * float4(vert.position, 1.0);

#if RN_COLOR
	result.color = vert.color;
#endif
#if RN_NORMALS
	result.normal = (uniforms.modelMatrix * float4(vert.normal, 0.0)).xyz;
#endif
#if RN_UV0
	result.texCoords = vert.texCoords * uniforms.textureTileFactor;
#endif

	result.ambient = uniforms.ambientColor;
	result.diffuse = uniforms.diffuseColor;

	return result;
}


fragment float4 gouraud_fragment(FragmentVertex vert [[stage_in]]
#if RN_UV0
	, texture2d<float> texture [[texture(0)]], sampler samplr [[sampler(0)]]
#endif
#if RN_FRAGMENT_UNIFORM
	, constant FragmentUniforms &uniforms [[buffer(1)]]
#endif
)
{
#if RN_UV0
	float4 color = texture.sample(samplr, vert.texCoords).rgba;

#if RN_DISCARD
		if(color.a < uniforms.discardThreshold)
			discard_fragment();
#endif
#endif

#if RN_COLOR
	color *= vert.color;
#endif

#if RN_NORMALS
	return color * (vert.ambient + vert.diffuse * saturate(dot(normalize(vert.normal), normalize(light_position))));
#else
	return color * (vert.ambient + vert.diffuse);
#endif
}

// Instancing

struct InstanceBufferUniform
{
	int index;
};

struct InstanceBufferMatrixUniform
{
	matrix_float4x4 _modelMatrix;
	matrix_float4x4 _inverseModelMatrix;
};

vertex FragmentVertex gouraud_vertex_instanced(InputVertex vert [[stage_in]], constant Uniforms &uniforms [[buffer(1)]], constant InstanceBufferUniform *instanceUniforms [[buffer(2)]], constant InstanceBufferMatrixUniform *matricesUniform [[buffer(3)]], ushort iid [[instance_id]])
{
	int index = instanceUniforms[iid].index;

	FragmentVertex result;

	result.position = uniforms.viewProjectionMatrix * matricesUniform[index]._modelMatrix * float4(vert.position, 1.0);

#if RN_COLOR
	result.color = vert.color;
#endif
#if RN_NORMALS
	result.normal = (uniforms.modelMatrix * float4(vert.normal, 0.0)).xyz;
#endif
#if RN_UV0
	result.texCoords = vert.texCoords * uniforms.textureTileFactor;
#endif

	result.ambient = uniforms.ambientColor;
	result.diffuse = uniforms.diffuseColor;

	return result;
}

fragment float4 gouraud_fragment_instanced(FragmentVertex vert [[stage_in]]
#if RN_UV0
	, texture2d<float> texture [[texture(0)]], sampler samplr [[sampler(0)]]
#endif
#if RN_FRAGMENT_UNIFORM
	, constant FragmentUniforms &uniforms [[buffer(1)]]
#endif
)
{
#if RN_UV0
	float4 color = texture.sample(samplr, vert.texCoords).rgba;

#if RN_DISCARD
		if(color.a < uniforms.discardThreshold)
			discard_fragment();
#endif
#endif

#if RN_COLOR
	color *= vert.color;
#endif

#if RN_NORMALS
	return color * (vert.ambient + vert.diffuse * saturate(dot(normalize(vert.normal), normalize(light_position))));
#else
	return color * (vert.ambient + vert.diffuse);
#endif
}

