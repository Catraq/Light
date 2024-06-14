#version 430 core

layout(location=0) in vec2 v_position;


uniform vec2 position;
uniform vec2 size;

void main()
{
	gl_Position = vec4(position + 2.0*(v_position/2.0+0.5)*size, 0.0, 1.0); 
}

