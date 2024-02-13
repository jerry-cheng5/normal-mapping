#version 330 core

out vec4 FragColor;

in vec2 oUV;
in vec3 oNorm;
in vec3 FragPos;

uniform sampler2D tex1;

uniform float dirlightBrightness;

void main()
{
   FragColor = texture(tex1, oUV);
}
