//
//  rn_Lighting.fsh
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#define RN_LIGHTING_FSH

#ifdef RN_LIGHTING

uniform isamplerBuffer lightPointList;
uniform isamplerBuffer lightPointListOffset;
uniform samplerBuffer lightPointListData;

uniform isamplerBuffer lightSpotList;
uniform isamplerBuffer lightSpotListOffset;
uniform samplerBuffer lightSpotListData;

uniform int lightDirectionalCount;
uniform vec3 lightDirectionalDirection[10];
uniform vec3 lightDirectionalColor[10];
uniform sampler2DShadow lightDirectionalDepth;

uniform vec2 clipPlanes;

uniform vec4 lightTileSize;
in vec3 outLightNormal;
in vec3 outLightPosition;
in vec4 outDirLightProj;

float offset_lookup(sampler2DShadow map, vec4 loc, vec2 offset)
{
	return textureProj(map, vec4(loc.xy + offset/1024.0/2.0 * loc.w,
	loc.z, loc.w));
}

vec4 rn_Lighting()
{
	vec3 normal = normalize(outLightNormal);
	vec3 posdiff = vec3(0.0);
	float attenuation = 0.0;
	vec3 light = vec3(0.2);
	vec4 lightpos;
	vec3 lightcolor;
	vec4 lightdir;
	float dirfac;
	int lightindex;
	ivec2 listoffset;
	
/*	int tileindex = int(int(gl_FragCoord.y/lightTileSize.y)*lightTileSize.z+int(gl_FragCoord.x/lightTileSize.x));
	
	listoffset = texelFetch(lightPointListOffset, tileindex).xy;
	for(int i=0; i<listoffset.y; i++)
	{
		lightindex = (texelFetch(lightPointList, listoffset.x + i).r) * 2;

		lightpos   = texelFetch(lightPointListData, lightindex);
		lightcolor = texelFetch(lightPointListData, lightindex + 1).xyz;
		
		posdiff     = lightpos.xyz-outLightPosition;
		attenuation = max((lightpos.w-length(posdiff))/lightpos.w, 0.0);
		
		light += lightcolor*max(dot(normal, normalize(posdiff)), 0.0)*attenuation*attenuation;
	}

	listoffset = texelFetch(lightSpotListOffset, tileindex).xy;
	for(int i=0; i<listoffset.y; i++)
	{
		lightindex = (texelFetch(lightSpotList, listoffset.x + i).r) * 3;

		lightpos   = texelFetch(lightSpotListData, lightindex);
		lightcolor = texelFetch(lightSpotListData, lightindex + 1).xyz;
		lightdir   = texelFetch(lightSpotListData, lightindex + 2);
		
		posdiff = lightpos.xyz-outLightPosition;
		attenuation = max((lightpos.w-length(posdiff))/lightpos.w, 0.0);
		dirfac = dot(normalize(posdiff), lightdir.xyz);
		
		if(dirfac > lightdir.w)
			light += lightcolor*max(dot(normal, normalize(posdiff)), 0.0)*attenuation*attenuation;
	}*/
	
	for(int i=0; i<lightDirectionalCount; i++)
	{
		float shadow = offset_lookup(lightDirectionalDepth, outDirLightProj, vec2(-0.5, -0.5));
		shadow += offset_lookup(lightDirectionalDepth, outDirLightProj, vec2(-0.5, 0.5));
		shadow += offset_lookup(lightDirectionalDepth, outDirLightProj, vec2(0.5, -0.5));
		shadow += offset_lookup(lightDirectionalDepth, outDirLightProj, vec2(0.5, 0.5));
		
		shadow *= 0.25;
		
		light += lightDirectionalColor[i]*max(dot(normal, lightDirectionalDirection[0]), 0.0)*shadow;
	}
	
	return vec4(light, 1.0);
}

#else

#define rn_Lighting() (vec4(1.0))

#endif