#version 430 core

uniform sampler2D in_texture;

uniform vec2 position;
uniform vec2 size;
uniform vec3 color;


out vec4 fcolor;

void main()
{	
	fcolor = vec4(color, 0);
}


