#version 430 core


uniform vec3 color;
uniform uvec2 dimension;
uniform vec2 position;
uniform vec2 size;

out vec4 fcolor;

void main()
{
	vec2 dim = vec2(dimension);
	vec2 uv = 2.0*vec2(gl_FragCoord.xy)/dim-1.0;

	float y1 = position.y + 2.0*size.y*3.0/10.0;
	float y2 = position.y + 2.0*size.y*7.0/10.0;
	
	float s = 2.0*size.y*1.0/10.0;
	if((uv.y > y1 - s && uv.y < y1 + s) 
	|| (uv.y > y2 - s && uv.y < y2 + s))
	{
		discard;	
	}
	else
	{
		fcolor = vec4(color, 0);	
	}
}


