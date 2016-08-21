//
//  Shaders.hlsl
//  Rayne
//
//  Copyright 2015 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

// Exported defines:
// RN_NORMALS
// RN_COLOR
// RN_UV0
// RN_DISCARD

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
static const float3 light_position = float3(1.0, 1.0, 1.0);

#if RN_UV0
Texture2D texture : register(t0);
SamplerState samplr : register(s0);
#endif

/*cbuffer Uniforms : register(b0)
{
	matrix modelViewProjectionMatrix;

#if RN_NORMALS
	matrix modelMatrix;
#endif

	float4 ambientColor;
	float4 diffuseColor;
};

#if RN_FRAGMENT_UNIFORM
cbuffer FragmentUniforms : register(b1)
{
#if RN_DISCARD
	float discardThreshold;
#endif
};
#endif*/

struct InputVertex
{
	float3 position : POSITION;

#if RN_NORMALS
	float3 normal : NORMAL;
#endif
#if RN_COLOR
	float4 color : COLOR0;
#endif
#if RN_UV0
	float2 texCoords : TEXCOORD0;
#endif
#if RN_TANGENTS
	float4 tangents : TANGENT;
#endif
};

struct FragmentVertex
{
	float4 position : SV_POSITION;

#if RN_NORMALS
	float3 normal : NORMAL;
#endif
#if RN_COLOR
	float4 color : COLOR2;
#endif
#if RN_UV0
	float2 texCoords : TEXCOORD0;
#endif

	float4 ambient : COLOR0;
	float4 diffuse : COLOR1;
};

FragmentVertex gouraud_vertex(InputVertex vert)
{
	FragmentVertex result;

	result.position = float4(vert.position, 1.0);// mul(modelViewProjectionMatrix, float4(vert.position, 1.0));

#if RN_COLOR
	result.color = vert.color;
#endif
#if RN_NORMALS
	result.normal = mul(modelMatrix, float4(vert.normal, 0.0)).xyz;
#endif
#if RN_UV0
	result.texCoords = vert.texCoords;
#endif

	result.ambient = 1.0f;//ambientColor;
	result.diffuse = 1.0f;//diffuseColor;

	return result;
}


float4 gouraud_fragment(FragmentVertex vert) : SV_TARGET
{
	float4 color = 1.0f;
#if RN_UV0
	color = texture.Sample(samplr, vert.texCoords).rgba;

#if RN_DISCARD
	clip(color.a - discardThreshold);
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