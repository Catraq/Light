#ifndef PARTICLE_H_
#define PARTICLE_H_

struct particle
{
	vec3f position;
	vec3f velocity;
};


struct particle void particle_update(struct particle prev, const float deltatime)
{
	vec3f delta = prev.velocity * deltatime;
	prev.position = p.position  + delta;
	return prev;
}

int32_t particle_render(struct particle *particles, uint32_t count)
{
	int result = 1;

	return result;
}




#endif PARTICLE_H_
