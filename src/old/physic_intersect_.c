

struct light_physic_body
{
	float mass;
	struct vec3 position;
	struct vec3 velocity;
};

struct light_physic_particle
{
	float mass;
	struct vec3 position;
	struct vec3 velocity;
};

struct light_physic_intersect
{
	struct vec3 position;
	struct vec3 normal;
};

struct light_physic_shape_sphere
{
	float radius;
};

struct light_physic_shape_plane
{
	struct vec3 normal;
};


void 
light_physic_intersect_resolve(struct light_physic_body *body_a, struct light_physic_body *body_b, struct light_physic_intersect *intersect)
{
	//Reflect velocity in nor plane. m/s 	
	float proj_vel_a = v3dot(body_a->velocity, intersect->normal); 
	float proj_vel_b = v3dot(body_b->velocity, intersect->normal);

/* m/s */
	float new_vel_a = (body_a->mass - body_b->mass)/(body_a->mass + body_b->mass) * proj_vel_a + 2*body_b->mass/(body_a->mass + body_b->mass)*proj_vel_b;
	float new_vel_b = (body_b->mass - body_a->mass)/(body_a->mass + body_b->mass) * proj_vel_b + 2*body_a->mass/(body_a->mass + body_b->mass)*proj_vel_a;

	body_a->velocity = v3scl(intersect->normal, new_vel_a);
	body_b->velocity = v3scl(intersect->normal, new_vel_b);
}

int light_physic_intersect_sphere_particle(struct light_physic_body *body, struct light_physic_shape_sphere *sphere, struct light_physic_particle *particle)
{
	struct vec3 dir = v3sub(body->position, particle->position);
	float dist = v3len(dir);
	if(dist > sphere->radius)
	{
		return 0;	
	}
	
	struct vec3 nor = v3scl(dir, 1.0f/dist);	
	//Reflect velocity in nor plane.	
	float proj_a = 	v3dot(particle->velocity, nor);
	particle->velocity = v3sub(particle->velocity, v3scl(nor, 2*proj_a));

	return 1;
}

int light_physic_intersect_sphere_plane(struct light_physic_body *body_a, struct light_physic_shape_sphere *sphere_a, struct light_physic_body *body_b, struct light_physic_shape_plane *plane_b, const float deltatime)
{
	//Center plane in (0, 0, 0)
	struct vec3 p_new = v3sub(body_a->position, body_b->position);
	float plane_proj = v3dot(p_new, plane_b->normal);
	if(plane_proj > sphere_a->radius){
		return 0;	
	}
	//Reflect velocity in nor plane.	
	float proj_a = v3dot(body_a->velocity, plane_b->normal);
	body_a->velocity = v3add(body_a->velocity, v3scl(plane_b->normal, -2*proj_a));
	
	return 1;
}

int light_physic_intersect_sphere_sphere(struct light_physic_body *body_a, struct light_physic_shape_sphere *sphere_a, struct light_physic_body *body_b, struct light_physic_shape_sphere *sphere_b, struct light_physic_intersect *intersect)
{
	
	float d1_d2 = sphere_a->radius + sphere_b->radius;
	struct vec3 dir = v3sub(body_a->position, body_b->position);
	float length = v3len(dir);

	if(length >= d1_d2)
	{
		return 0;	
	}


	intersect->normal = v3scl(dir, 1/length);
	
	return 1;
}

