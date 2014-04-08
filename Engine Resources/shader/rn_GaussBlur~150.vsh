//
//  rn_GaussBlur.vsh
//  Rayne
//
//  Copyright 2014 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#version 150
precision highp float;

in vec2 attPosition;
in vec2 attTexcoord0;

out vec2 vertTexcoord;

void main()
{
	vertTexcoord = attTexcoord0;
	gl_Position = vec4(attPosition, 0.0, 1.0);
}
