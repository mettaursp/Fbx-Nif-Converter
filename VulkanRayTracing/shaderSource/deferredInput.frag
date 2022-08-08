#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D albedoTexture;

layout(location = 0) in vec3 uvs;
layout(location = 1) in vec4 pos;
layout(location = 2) in vec3 norm;

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 position;
layout(location = 2) out vec3 normal;
layout(location = 3) out vec2 uv;

void main()
{
	albedo = texture(albedoTexture, uvs.xy).xyz;
	position = pos.xyz;
	normal = normalize(norm);
	uv = uvs.xy;
}