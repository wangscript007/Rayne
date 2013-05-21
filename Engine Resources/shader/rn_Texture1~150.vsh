//
//  rn_Texture1.vsh
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

#include "rn_Matrices.vsh"
#include "rn_Animation.vsh"
#include "rn_Shadow.vsh"

in vec3 attPosition;
in vec3 attNormal;
in vec2 attTexcoord0;
in vec4 attTangent;

out vec2 vertTexcoord;

#ifdef RN_LIGHTING
out vec3 vertPosition;
#ifdef RN_NORMALMAP
out mat3 vertMatInvTangent;
#else
out vec3 vertNormal;
#endif
#endif

void main()
{
	vertTexcoord = attTexcoord0;

	vec4 position = rn_Animate(vec4(attPosition, 1.0));
	vec4 normal   = rn_Animate(vec4(attNormal, 0.0));

	normal.w = 0.0;
	position.w = 1.0;
	
#ifdef RN_LIGHTING
	#ifdef RN_NORMALMAP
	vec4 tangent = rn_Animate(vec4(attTangent.xyz, 0.0));
	vertMatInvTangent[0] = normalize((matModel*tangent).xyz);
	vertMatInvTangent[2] = normalize((matModel*normal).xyz);
	vertMatInvTangent[1] = normalize(cross(vertMatInvTangent[0], vertMatInvTangent[2])*attTangent.w);
	#else
	vertNormal = (matModel * normal).xyz;
	#endif
	vertPosition = (matModel * position).xyz;
#endif
	
#if defined(RN_DIRECTIONAL_SHADOWS) && defined(RN_LIGHTING)
	rn_ShadowDir1(position);
#endif
	
	gl_Position = matProjViewModel * vec4(position.xyz, 1.0);
}
