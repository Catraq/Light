#version 430 core

layout(location=0) in vec2 v_position;


uniform vec2 position;
uniform vec2 size;
out vec2 fragcoord;

void main()
{
	fragcoord.x = (v_position + vec2(1)).x/2;
	fragcoord.y = 1 - (v_position + vec2(1)).y/2;
	gl_Position = vec4(position + 2.0*(v_position/2.0+0.5)*size, 0.0, 1.0); 
}

