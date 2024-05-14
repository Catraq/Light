

layout(location = 0) in vec2 r_position;

layout(binding = 2) uniform sampler2D particle_acceleration_in;
layout(binding = 1) uniform sampler2D particle_velocity_in;
layout(binding = 0) uniform sampler2D particle_position_in;

flat out uint particle_id;

layout(binding=6) uniform scene_ubo
{
	mat4 view;
	mat4 view_inv;
	uint width;
	uint height;
	uint dummy[2];
	mat4 proj;	
};


layout(binding=8) uniform emitter_index
{
	uint particle_emitter_index[EMITTER_COUNT];
};



void main()
{
	float size = 0.01;
	particle_id = gl_InstanceID;
	
	mat4 m = proj * view; 
	ivec2 coord = ivec2(gl_InstanceID%EMITTER_PARTICLE_COUNT, gl_InstanceID/EMITTER_PARTICLE_COUNT);
	vec3 p = texelFetch(particle_position_in, coord, 0).xyz;
	vec3 crw = vec3(m[0][0], m[1][0], m[2][0]);
	vec3 cuw = vec3(m[0][1], m[1][1], m[2][1]);
	p = p + crw * r_position.x * size + cuw * r_position.y * size;
	gl_Position = m * vec4(p, 1.0);
}
