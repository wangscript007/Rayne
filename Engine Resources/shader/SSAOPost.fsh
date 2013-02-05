
#version 150
precision highp float;

uniform sampler2D mTexture0; // SSAO
uniform sampler2D targetmap0; // Scene

in vec2 texcoord;
out vec4 fragColor0;

void main()
{
	vec4 scene = texture(targetmap0, texcoord);
	float ssao = texture(mTexture0, texcoord).r;

	fragColor0 = scene * ssao;
}
