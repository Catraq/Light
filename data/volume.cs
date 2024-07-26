layout(local_size_x=1,local_size_y=1,local_size_z=1) in;


struct object_node
{
	uint object_index;
	uint padding[3];

	
	mat4 translation;
	mat4 translation_inv;
};

layout(binding=1) uniform nodes_ubo
{
	object_node nodes[OBJECT_NODE_COUNT];
};

uniform uint node_index;

uniform float samples;

layout(binding=0) buffer volume_ssbo
{
	uint volume_samples[];
};


float nrand(vec2 n)
{
	return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}



void main()
{
	float x = 5*(2*float(gl_GlobalInvocationID.x)/samples-1);
	float y = 5*(2*float(gl_GlobalInvocationID.y)/samples-1);
	float z = 5*(2*float(gl_GlobalInvocationID.z)/samples-1);
	
	vec3 p = vec3(x,y,z);
	//1 if point is inside. vec4(p, 0) as 0 at .w index do not translate the point 
	uint inside = function_inside(nodes[node_index].object_index, (nodes[node_index].translation_inv * vec4(p, 0)).xyz);
	atomicAdd(volume_samples[node_index], inside);
}
