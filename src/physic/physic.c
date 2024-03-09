struct light_physic_plane_static
{
	struct vec3 position;
	struct vec3 normal;
	struct vec3 dir;

	float width;
	float height;
};



struct light_physic_sphere_body
{
	struct vec3 position;
	struct vec3 velocity;
	float mass;
	float radius;	
};

struct light_physic_particle
{
	struct vec3 position;
	struct vec3 velocity;
	float mass;
};

struct light_physic_intersect
{
	uint32_t i;
	float t;
};


uint32_t light_physic_plane_sphere_intersect(struct light_physic_plane_static *plane, struct light_physic_sphere_body *sphere, struct light_physic_intersect *intersect, uint32_t count)
{
	uint32_t intersect_count = 0;
	for(uint32_t i = 0; i < count; i++)
	{
		float d = v3dot(plane->normal, sphere[i].velocity);
		struct vec3 n = v3scl(plane->normal, 1.0f/v3len(plane->normal));
		struct vec3 p1 = v3add(plane->position, v3scl(n, sphere[i].radius));
		struct vec3 p2 = v3sub(plane->position, v3scl(n, sphere[i].radius));

		float t1 = v3dot(v3sub(p1, sphere[i].position), plane->normal)/d;
		float t2 = v3dot(v3sub(p2, sphere[i].position), plane->normal)/d;
		float t = t1 < t2 ? t1 : t2;

		if(t < 0.0f){
			continue;
		}

		struct vec3 l = v3add(sphere[i].position, v3scl(sphere[i].velocity, t));
		
		float dh = v3len(v3cross(plane->dir, v3sub(plane->position, l)))/v3dot(plane->dir, plane->dir);
		if(dh > plane->height/2.0f){
			continue;	
		}


		struct vec3 dir2 = v3cross(plane->normal, plane->dir);
		float dw = v3len(v3cross(dir2, v3sub(plane->position, l)))/v3dot(dir2, dir2);
		if(dw > plane->width/2.0f){
			continue;	
		}



		intersect[intersect_count].i = i;	
		intersect[intersect_count].t = t;	
		intersect_count++;

			
	}
	return intersect_count; 
}

void light_physic_plane_sphere_resolve(struct light_physic_plane_static *plane, struct light_physic_sphere_body *body)
{
	body->velocity = v3sub(body->velocity, v3scl(plane->normal, 2.0f*v3dot(plane->normal, body->velocity)));
}


uint32_t
light_physic_plane_particle_intersect(struct light_physic_plane_static *plane, struct light_physic_particle *particle, struct light_physic_intersect *intersect, uint32_t count)
{
	uint32_t intersect_count = 0;
	for(uint32_t i = 0; i < count; i++)
	{
		float d = v3dot(plane->normal, particle[i].velocity);
		float t = v3dot(v3sub(plane->position, particle[i].position), plane->normal)/d;
		if(t < 0.0f){
			continue;
		}

		struct vec3 l = v3add(particle[i].position, v3scl(particle[i].velocity, t));
		
		float dh = v3len(v3cross(plane->dir, v3sub(plane->position, l)))/v3dot(plane->dir, plane->dir);
		if(dh > plane->height/2.0f){
			continue;	
		}


		struct vec3 dir2 = v3cross(plane->normal, plane->dir);
		float dw = v3len(v3cross(dir2, v3sub(plane->position, l)))/v3dot(dir2, dir2);
		if(dw > plane->width/2.0f){
			continue;	
		}



		intersect[intersect_count].i = i;	
		intersect[intersect_count].t = t;	
		intersect_count++;
	}

	return intersect_count;
}

void light_physic_plane_particle_resolve(struct light_physic_plane_static *plane, struct light_physic_particle *particle)
{
	particle->velocity = v3sub(particle->velocity, v3scl(plane->normal, 2.0f*v3dot(plane->normal, particle->velocity)));
}

uint32_t 
light_physic_sphere_particle_intersect(struct light_physic_sphere_body *body, struct light_physic_particle *particle, struct light_physic_intersect *intersect, uint32_t particle_count)
{
	uint32_t intersection_count = 0;

	for(uint32_t i = 0; i < particle_count; i++)
	{
		float D = body->radius;

		float a = v3dot(body->velocity, body->velocity) - 2.0f*v3dot(body->velocity, particle[i].velocity) + v3dot(particle[i].velocity, particle[i].velocity); 					
		if(a <= 0.0f)
		{
			continue;	
		}

		float b = 2.0f*v3dot(body->velocity, body->position) - 2.0f*v3dot(body->velocity, particle[i].position) - v3dot(body->position, particle[i].velocity) + v3dot(particle[i].position, particle[i].velocity) - v3dot(particle[i].velocity, body->position) + v3dot(particle[i].velocity, particle[i].position);

		float c = v3dot(body->position, body->position) - 2.0f*v3dot(body->position, particle[i].position) + v3dot(particle[i].position, particle[i].position) - D*D;
		float o = b*b - 4.0f*a*c;

		if(o < 0.0f)
		{
			continue;
		}

		float t = (-b-sqrtf(o))/(2.0f*a);

		if(t < 0.0f)
		{
			continue;	
		}
		
		intersect[intersection_count].i = i;
		intersect[intersection_count].t = t;

		intersection_count++;
	}
	return intersection_count;
}

uint32_t light_physic_sphere_sphere_intersect(struct light_physic_sphere_body *body, struct light_physic_sphere_body *sphere, struct light_physic_intersect *intersect,  uint32_t count)
{
	
	uint32_t intersection_count = 0;	
	for(uint32_t i = 0; i < count; i++)
	{
		float D = sphere[i].radius + body->radius;

		float a = v3dot(body->velocity, body->velocity) - 2.0f*v3dot(body->velocity, sphere[i].velocity) + v3dot(sphere[i].velocity, sphere[i].velocity); 					
		if(a <= 0.0f)
		{
			continue;	
		}

		float b = 2.0f*v3dot(body->velocity, body->position) - 2.0f*v3dot(body->velocity, sphere[i].position) - v3dot(body->position, sphere[i].velocity) + v3dot(sphere[i].position, sphere[i].velocity) - v3dot(sphere[i].velocity, body->position) + v3dot(sphere[i].velocity, sphere[i].position);

		float c = v3dot(body->position, body->position) - 2.0f*v3dot(body->position, sphere[i].position) + v3dot(sphere[i].position, sphere[i].position) - D*D;
		float o = b*b - 4.0f*a*c;

		if(o < 0.0f)
		{
			continue;
		}

		float t = (-b-sqrtf(o))/(2.0f*a);

		if(t < 0.0f)
		{
			continue;	
		}
		
		intersect[intersection_count].i = i;
		intersect[intersection_count].t = t; 
		intersection_count++;
	}

	return intersection_count;
}

void 
light_physic_sphere_particle_resolve(struct light_physic_sphere_body *sphere, struct light_physic_particle *particle)
{
	float v1ns = 2.0f * particle->mass / (particle->mass + sphere->mass) * v3dot(v3sub(sphere->velocity, particle->velocity), v3sub(sphere->position, particle->position))/v3dot(v3sub(sphere->position, particle->position), v3sub(sphere->position, particle->position));
	struct vec3 v1n = v3sub(sphere->velocity, v3scl(v3sub(sphere->position, particle->position), v1ns)); 	
	
	float v2ns = 2.0f * sphere->mass / (particle->mass + sphere->mass) * v3dot(v3sub(particle->velocity, sphere->velocity), v3sub(particle->position, sphere->position))/v3dot(v3sub(sphere->position, particle->position), v3sub(sphere->position, particle->position));
	struct vec3 v2n = v3sub(particle->velocity, v3scl(v3sub(particle->position, sphere->position), v2ns)); 	
	
	sphere->velocity = v1n;
	particle->velocity = v2n;
}

void 
light_physic_sphere_sphere_resolve(struct light_physic_sphere_body *sphere, struct light_physic_sphere_body *particle)
{
	float v1ns = 2.0f * particle->mass / (particle->mass + sphere->mass) * v3dot(v3sub(sphere->velocity, particle->velocity), v3sub(sphere->position, particle->position))/v3dot(v3sub(sphere->position, particle->position), v3sub(sphere->position, particle->position));
	struct vec3 v1n = v3sub(sphere->velocity, v3scl(v3sub(sphere->position, particle->position), v1ns)); 	
	
	float v2ns = 2.0f * sphere->mass / (particle->mass + sphere->mass) * v3dot(v3sub(particle->velocity, sphere->velocity), v3sub(particle->position, sphere->position))/v3dot(v3sub(sphere->position, particle->position), v3sub(sphere->position, particle->position));
	struct vec3 v2n = v3sub(particle->velocity, v3scl(v3sub(particle->position, sphere->position), v2ns)); 	
	
	sphere->velocity = v1n;
	particle->velocity = v2n;
}


