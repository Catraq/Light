#version 430 core

#define RADIO_BUTTON_TICKNESS radius*radius/4

uniform vec3 color;
uniform uint checked;
uniform uvec2 dimension;

in vec2 center;
in float radius;


out vec4 fcolor;

void main()
{

	vec2 dim = vec2(dimension);
	vec2 uv = 2.0*vec2(gl_FragCoord.xy)/dim-1.0;
	
	vec2 d = uv-center;
	d.x *= dim.x/dim.y;
	float r = radius * dim.x/dim.y;

	
	
	float v = dot(d,d);

	if(v < r*r)
	{
		fcolor = vec4(color, 0);
		
		if(v < r * r - r*r/2)
		{
			
			if(v < r * r - r*r * 1.5/2 && checked > 0)
			{
				fcolor = vec4(0, 0, 0, 0);
			}else
			{
				discard;
			}
		}
	}
	else
		discard;

}
