#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 1) uniform sampler2D albedoTexture;

layout(location = 0) in vec3 uvs;
layout(location = 1) in vec4 pos;
layout(location = 2) in vec3 norm;
layout(location = 0) out vec4 color;

void main()
{
	vec3 fragPos = pos.xyz;
	
	vec3 normal = normalize(norm);
	
	vec3 lightPos = vec3(-10, 15, 10);
	vec3 lightColor = vec3(212.0/255.0, 235.0/255.0, 1);
	
	vec3 ambientLight = vec3(0.1, 0.1, 0.1);
	
	vec3 look = -normalize(fragPos);
	vec3 light = normalize(lightPos - fragPos);
	vec3 albedo = texture(albedoTexture, uvs.xy).xyz;
	
	vec3 diffuse = 0.5 * max(dot(light, normal), 0) * albedo * lightColor;
	vec3 specular = 0.5* max(dot(normalize(light + look), normal), 0) * lightColor;
	vec3 ambient = albedo * ambientLight;
	
	color = vec4(diffuse + specular + ambient, 1);
	//color = vec4(max(dot(light, normal), 0), 0, 0, 1);
}