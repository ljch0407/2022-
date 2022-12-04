#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNormal;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 FragPos; 
out vec3 Normal;

void main()
{
	gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vPos, 1.0f);
	
	FragPos = vec3(modelMatrix * vec4(vPos, 1.0));
	Normal = vNormal;
}