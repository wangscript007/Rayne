//
//  rn_Texture1.vsh
//  Rayne
//
//  Copyright 2013 by Felix Pohl, Nils Daumann and Sidney Just. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

uniform mat4 matProjViewModel;

in vec3 position;
in vec2 texcoord0;

out vec2 outTexcoord;

void main()
{
	outTexcoord = texcoord0;
	
	gl_Position = matProjViewModel * vec4(position, 1.0);
}