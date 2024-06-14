#version 430 core

uniform sampler2D in_texture;

uniform vec3 color;
uniform vec2 position;
uniform vec2 size;

out vec4 fcolor;

void main()
{	
	fcolor = vec4(color, 0);
}


