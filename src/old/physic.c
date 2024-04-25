


/* 
	BDOY 
		position 
		rotation 

		center_of_gravity 
		angular velocity  
		velocity 


	SHAPE(SPHERE)
		radius


	
	Movement
	Intersection
	Angular momentum 
	momentum 
		



*/


/* 
 *	Physic body is the description of the physical object 
 *	if all point masses are threated as one. 
 */

struct light_physic_body
{
	float mass;
	struct vec3 position;
	struct vec3 velocity;
};


struct light_physic_intersection
{
	struct vec3 normal;
};

struct light_physic_shape_sphere
{
	float radius;
};

int light_physic_intersect_sphere_sphere(struct physic_body *body_a, struct physic_shape_sphere *sphere_a, struct physic_body *body_b, struct physic_shape_sphere *sphere_b)
{
	
	float d1_d2 = sphere_a->radius + sphere_b->radius;
	struct vec3 dir = v3sub(body_a->position, body_b->position);
	float length = v3len(dir);
	if(length > d1_d2)
	{
		return 0;	
	}
	struct vec3 nor = v3scl(dir, 1/length);
	
	//Reflect velocity in nor plane.	
	float proj_a = 	v3dot(body_a->velocity, nor);
	body_a->velocity = body_a->velocity  - v3scl(nor, 2*proj_a);
	
	//Reflect velocity in nor plane.	
	float proj_b = 	v3dot(body_b->velocity, nor);
	body_b->velocity = body_b->velocity  - v3scl(nor, 2*proj_b);
	


	return 0;
}



