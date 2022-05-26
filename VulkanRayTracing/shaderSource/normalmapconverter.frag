#version 460
#pragma shader_stage(fragment)
// NEVER FORGET THIS VERSION LINE, I SPENT AN HOUR BEFORE FIGURING OUT I MISSED IT LOL
// change this if the OpenGL version is earlier than what supports this


// input texture
layout (binding = 0) uniform sampler2D inputTexture;

// input uv coordinates
layout (location = 0) in vec2 uv;

layout (location = 0) out vec4 fragColor;

void main()
{
	vec4 color = texture(inputTexture, uv);
	
	float normalZ = length(2 * color.ag - vec2(1, 1)); // derive z from orange normal map
	
	fragColor.rg = color.ag; // swizzle to extract the r and g channels
	fragColor.b = 0.5 * (normalZ + 1);
	fragColor.a = 1;
}