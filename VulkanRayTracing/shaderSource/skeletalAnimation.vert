#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PushConstants
{
	mat4 projection;
	mat4 model;
	//vec3 modelPos;
} pushConstants;

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uvs;
layout(location = 3) in vec3 morph1;

layout(location = 0) out vec3 outuvs;
layout(location = 1) out vec4 outpos;
layout(location = 2) out vec3 outnorm;

layout(binding = 2) uniform SceneInfo
{
	float time;
} sceneInfo;

void main()
{
	float t = (1 + 0.5 * cos(sceneInfo.time));

	vec4 worldPos = pushConstants.model * vec4(pos + t * morph1, 1);
	
	gl_Position = pushConstants.projection * worldPos;
	
	gl_Position.y = -gl_Position.y;
	gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0f;
	
	vec4 scaledNormal = transpose(inverse(pushConstants.model)) * vec4(normal, 0);
	
	outpos = worldPos;
	outuvs = vec3(uvs, 0);
	outnorm = scaledNormal.xyz;
}