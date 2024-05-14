

struct sphere_instance
{
	vec3 color;
	float radius;
};


struct cylinder_instance
{
	vec3 color;

	/* radius of each cylinder  */
	float radius; 

	/* height of cylinder */
	float height;
	
	/* Required padding */
	float dummy[3];
};


struct box_instance
{	
	vec3 color; 
	
	float padding1;

	/* box dimension */	
	vec3 dimension;

	float padding2;
};


struct light_instance
{
	vec3 position;
	float padding1;
	vec3 color;
	float padding2;
};



#define	UNION 		0
#define INTERSECT 	1
#define DIFFERENCE	2

struct object_instance
{
	uint levels;
	uint index_type;
	uint index_left;
	uint index_right;
	
	mat4 translation;
	mat4 translation_inv;
};


struct object_node
{
	uint padding;
	uint index_type;
	uint object_index;
	uint node_index;
	
	mat4 translation;
	mat4 translation_inv;
};



layout(binding = 2) uniform sampler2D particle_acceleration_in;
layout(binding = 1) uniform sampler2D particle_velocity_in;
layout(binding = 0) uniform sampler2D particle_position_in;

layout(location=2) out vec3 color;
layout(location=1) out vec3 position;
layout(location=0) out vec3 normal;


layout(binding=0) uniform sphere_ubo
{
	sphere_instance spheres[SPHERE_COUNT];
	uint		spheres_count;
	uint		spheres_dummy[3];		
};


layout(binding=1) uniform cylinder_ubo
{
	cylinder_instance 	cylinders[CYLINDER_COUNT];
	uint			cylinders_count;
	uint			cylinders_dummy[3];
};


layout(binding=2) uniform box_ubo
{
	box_instance 	boxes[BOX_COUNT];
	uint		boxes_count;
	uint		boxes_dummy[3];
};

layout(binding=3) uniform light_ubo
{
	light_instance lights[LIGHT_COUNT];
	uint		lights_count;
	uint		lights_dummy[3];
};


layout(binding=4) uniform objects_ubo
{
	object_instance objects[OBJECT_COUNT];
	uint		objects_count;
	uint		objects_ssbo_dummy[3];
};


layout(binding=5) uniform nodes_ubo
{
	object_node 	nodes[OBJECT_NODE_COUNT];
	uint 		nodes_count; 
	uint		nodes_ubo_dummy[3];
};


layout(binding=6) uniform scene_ubo
{
	mat4 view;
	mat4 view_inv;
	uint width;
	uint height;
	uint dummy[2];
	mat4 proj;	
};


flat in uint particle_id;

float sphere(vec3 p, float r)
{
	return length(p) - r;
}


float cylinder(vec3 p, float height, float radius)
{
	vec2 d = abs(vec2(length(p.xz), p.y)) - vec2(radius, height);
	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));			
}

float box(vec3 p, vec3 d){			
	vec3 q = abs(p) - d;				
	return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);	
}	

struct distance_result
{
	vec4 p;
	float min;
	vec3 color;
};


distance_result distance_object(vec4 p, float t_max, uint index)
{
	
	vec4 p_res = vec4(0.0);
	vec3 color = vec3(0.0);
	float min_t = t_max;
	
	if(index < spheres_count)
	{
		float t = sphere(p.xyz, spheres[index].radius);
		color = spheres[index].color;
		p_res = p;
		min_t = t;
			
	}
	else if(index - spheres_count < cylinders_count)
	{
		uint i = index - spheres_count;
		vec4 e = p;
		float t = cylinder(p.xyz, cylinders[i].radius, cylinders[i].height);
	
		color = cylinders[i].color;
		p_res = p;
		min_t = t;
		
	}
	else if(index - spheres_count - cylinders_count < boxes_count)
	{
		uint i = index - spheres_count - cylinders_count;
		vec4 e = p;
		float t = box(p.xyz, boxes[i].dimension);
	
		color = boxes[i].color;
		p_res = p;
		min_t = t;
		
	} 
	
	distance_result result;
	result.p = p_res;
	result.min = min_t;
	result.color = color;
	
	return result;
	
}



distance_result distance(vec4 p, float t_max)
{
	
	distance_result result;
	result.min = t_max;
	
	
	for(uint i = 0; i < EMITTER_COUNT; i++)
	{
		for(uint j = 0; j < EMITTER_PARTICLE_COUNT; j++)
		{
			ivec2 coord = ivec2(j, i);
			
			vec3 pp = texelFetch(particle_position_in, coord, 0).xyz;
			float t = length(pp - p.xyz) - 0.01;
			if(t < result.min){
				result.min = t;
				result.color = vec3(1.0, 0.0, 0.0);
				result.p = vec4(pp, 1);
			}
		}	
		
	}

	
	
	for(int i = 0; i < objects_count; i++)
	{
		uint left_nodes[OBJECT_NODE_STACK];
		uint right_nodes[OBJECT_NODE_STACK];
		
		left_nodes[0] = objects[i].index_left;	
		right_nodes[0] = objects[i].index_right;	
		

		/* Write node index to buffer */
		for(uint j = 1; j < objects[i].levels; j++)
		{
			left_nodes[j] = nodes[left_nodes[j-1]].node_index;
			right_nodes[j] = nodes[right_nodes[j-1]].node_index;
			
		}
		 
		
		distance_result left_result[OBJECT_NODE_STACK];
		distance_result right_result[OBJECT_NODE_STACK];
		
		
		/* Access the nodes backwards */
		uint index = objects[i].levels-1;
		
		uint left = left_nodes[index];	
		uint right = right_nodes[index];	
		
		


		{
			mat4 translation_inv = objects[i].translation_inv * nodes[left].translation_inv;
			distance_result left_res = distance_object(translation_inv * p, t_max, nodes[left].object_index);
			left_res.p = nodes[left].translation * left_res.p;
			
			left_result[index] = left_res;
		}
	
	


	
		{
			mat4 translation_inv = objects[i].translation_inv * nodes[right].translation_inv;
			distance_result right_res = distance_object(translation_inv * p, t_max, nodes[right].object_index);
			right_res.p = nodes[right].translation * right_res.p;
			
			right_result[index] = right_res;
		}
		
		

		

		for(uint j = 2; j < objects[i].levels+1; j++)
		{
			uint index = objects[i].levels - j;
			uint left = left_nodes[index];	
			uint right = right_nodes[index];	
			

			switch(nodes[left].index_type)
			{
				case UNION:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[left].translation_inv;
					distance_result left_res = distance_object(translation_inv * p, t_max, nodes[left].object_index);
					left_res.p = nodes[left].translation * left_res.p;
					
					left_result[index] = left_result[index+1].min < left_res.min ? left_result[index+1] : left_res; 
					
				}
				break;
				
				case INTERSECT:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[left].translation_inv;
					distance_result left_res = distance_object(translation_inv * p, t_max, nodes[left].object_index);
					left_res.p = nodes[left].translation * left_res.p;
					
					left_result[index] = left_result[index+1].min < left_res.min ? left_res : left_result[index+1]; 
					
				}
				break;
				
				case DIFFERENCE:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[left].translation_inv;
					distance_result left_res = distance_object(translation_inv * p, t_max, nodes[left].object_index);
					left_res.p = nodes[left].translation * left_res.p;
					
					left_result[index] = -left_result[index+1].min < left_res.min ? left_res : left_result[index+1]; 
					
				}
				
				break;
			}
			


			switch(nodes[right].index_type)
			{
				case UNION:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[right].translation_inv;
					distance_result right_res = distance_object(translation_inv  * p, t_max, nodes[right].object_index);
					right_res.p = nodes[right].translation * right_res.p;
					
					right_result[index] = right_result[index+1].min < right_res.min ? right_result[index+1] : right_res; 
					
				}
				break;
				
				case INTERSECT:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[right].translation_inv;
					distance_result right_res = distance_object(translation_inv  * p, t_max, nodes[right].object_index);
					right_res.p = nodes[right].translation * right_res.p;
					
					right_result[index] = right_result[index+1].min < right_res.min ? right_res : right_result[index+1]; 
					
				}
				break;
				
				case DIFFERENCE:
				{
					mat4 translation_inv = objects[i].translation_inv * nodes[right].translation_inv;
					distance_result right_res = distance_object(translation_inv  * p, t_max, nodes[right].object_index);
					right_res.p = nodes[right].translation * right_res.p;
					
					right_result[index+1].min *= -1.0;
					right_result[index] = right_result[index+1].min < right_res.min ? right_res : right_result[index+1]; 
					
				}
				break;
			}
			
		}
		
		distance_result res;
		switch(objects[i].index_type)
		{
			case UNION:
				res = left_result[0].min < right_result[0].min ? left_result[0] : right_result[0];
				break;
			case INTERSECT:
				res = left_result[0].min < right_result[0].min ? right_result[0] : left_result[0] ;
				break;
			case DIFFERENCE:
				right_result[0].min *= -1.0;
				res = left_result[0].min < right_result[0].min ? right_result[0] : left_result[0] ;
				break;
		}
		
		res.p = objects[i].translation * res.p;
		result = res.min < result.min ? res : result;
	} 
	

	return result;
	
}


//Soft shadow variation, see shadertoy. 
float shadow(vec4 ro, vec4 rd, float t_max)
{	
	float res = 1.0;
	float t = 0.0;	
	float ph = 1e10;
	float w = 0.1;
	distance_result d_res; 
	
	for(int i = 0; i < 32; i++)
	{
		d_res = distance(ro +rd*t, t_max);
		float h = d_res.min;
		
		float y = h*h/(2.0*ph);
            	float d = sqrt(h*h-y*y);
            	res = min( res, d/(w*max(0.0,t-y)) );
            	ph = h;
            	
            	t = t + d_res.min;
            	
		if(res < 0.0001 || t > t_max )
		{
			break;
		}
		
	}
	
	res = clamp( res, 0.0, 1.0 );
    	return res*res*(3.0-2.0*res);	
}

distance_result particle_distance(vec4 p, float t_max)
{
	distance_result result;
	result.min = t_max;

	ivec2 coord = ivec2(particle_id%EMITTER_PARTICLE_COUNT, particle_id/EMITTER_PARTICLE_COUNT);
	vec3 pp = texelFetch(particle_position_in, coord, 0).xyz;
	float t = length(pp - p.xyz) - 0.01;
	if(t < result.min){
		result.min = t;
		result.color = vec3(1.0, 0.0, 0.0);
		result.p = vec4(pp, 1);
	}
	
	return result;
}

void main()
{
	vec2 coord = mix(vec2(0, 0), vec2(1, 1), gl_FragCoord.xy / vec2(width, height));
	vec2 uv = 2.0*coord.xy-1.0;
	uv.x *= float(width)/float(height);
	vec3 ray = normalize(vec3(uv, 1.0));
	
	
			
	float t_max = 30.0;
	float t = 0.0;
	
	distance_result d_res; 
	for(int i = 0; i < 64; i++)
	{
		d_res = particle_distance(view_inv*vec4(ray*t, 1.0), t_max);
		if(d_res.min < 0.001 || t > t_max )
		{
			break;
		}
		t = t + d_res.min;
	}
	
	
	position = vec3(0,0,0);
	if(t < t_max)
	{
		distance_result infront = distance(view_inv*vec4(ray*t, 1.0), t_max);
		if(infront.min < d_res.min){
			discard;
		}
	
		vec4 p = d_res.p;
		vec3 color_result = vec3(1.0);
		
		
		vec3 e = vec3(0.01, 0.0, 0.0);
		
		vec4 n_ray = view_inv*vec4(ray*t, 1.0);
		vec3 n = vec3(
			particle_distance(vec4(n_ray+e.xyyy), t_max).min - particle_distance(vec4(n_ray-e.xyyy), t_max).min,
			particle_distance(vec4(n_ray+e.yxyy), t_max).min - particle_distance(vec4(n_ray-e.yxyy), t_max).min,
			particle_distance(vec4(n_ray+e.yyxy), t_max).min - particle_distance(vec4(n_ray-e.yyxy), t_max).min
		);

		n = normalize(n);	
		
		
		for(uint i = 0; i < lights_count; i++)
		{
			vec3 light_pos = lights[i].position;
			vec3 light_color = lights[i].color;
			

			vec3 light_dir = normalize(light_pos - p.xyz);
			float diff = clamp(dot(n, light_dir), 0.0, 1.0) * shadow(vec4(p.xyz, 1), vec4(light_dir, 0), t_max); 
			vec3 diffuse = diff * light_color;
			color_result = color_result + diffuse * d_res.color;
		}
		
		normal = n;
		color = color_result;
		position = p.xyz;
		
	}
	else
	{
		discard;
	}
}
