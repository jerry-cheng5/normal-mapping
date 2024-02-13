#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNorm;

out vec2 oUV;
out vec3 oNorm;
out vec3 FragPos;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;


void main()
{
    FragPos = vec3(modelMatrix * vec4(aPos, 1.0));
    oUV = aUV;
    oNorm = aNorm;

    gl_Position = projectionMatrix * viewMatrix * vec4(FragPos, 1.0); 
}
