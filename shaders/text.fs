#version 430 core

uniform sampler2D in_texture;

uniform vec3 color;
uniform vec2 position;
uniform vec2 size;

in vec2 fragcoord;
out vec4 fcolor;

void main()
{

	vec4 s = vec4(1,1,1, texture(in_texture, fragcoord).r);
	fcolor = vec4(color, 1.0) * s;
}


