
void
light_physic_rope_inverse(float *restrict u, float *restrict inv, uint32_t N)
{
	float b_a[N];

	for(uint32_t i = 0; i < N; i++)
	{
		inv[N*i + i] = 1.0;	
	}



	inv[0] = 1/u[0];
	float b_p = u[1]/u[0];

	b_a[0] = b_p;

	for(uint32_t i = 1; i < N-1; i++)
	{
		float a_p = u[N*i + i] - u[N*i + i - 1]*b_p;  
		for(uint32_t j = 0; j < N; j++)
		{
			inv[N*i+j] = (inv[N*i+j] - u[N*i + i - 1]*inv[N*(i-1)+j])/a_p;;
		}
		b_p = u[N*i + i + 1]/a_p;

		b_a[i] = b_p;
	}


	float a_p = u[N*N- 1] - u[N*N- 2]*b_p;  
	for(uint32_t j = 0; j < N; j++)
	{
		inv[N*(N-1)+j] = (inv[N*(N-1)+j] - u[N*N- 2]*inv[N*(N-2)+j])/a_p;;
	}


	for(uint32_t i = 0; i < N-1; i++)
	{
		for(uint32_t j = 0; j < N; j++)
		{
			inv[N*(N-i-2) +j] -= b_a[N - i - 2]*inv[N*(N-i-1) +j];
		}
	}



}

void
light_physic_rope_solve(float *restrict A, float *restrict b, uint32_t N)
{
	float b_a[N-1];

	float b_p = A[1]/A[0];

	b[0] = b[0]/A[0];

	b_a[0] = b_p;

	for(uint32_t i = 1; i < N-1; i++)
	{
		float a_p = A[N*i + i] - A[N*i + i - 1]*b_p;  
		b_p = A[N*i + i + 1]/a_p;
		b_a[i] = b_p;

		b[i] = (b[i] - b[i-1]*A[N*i + i - 1])/a_p;

	}
	
	float a_p = A[N*N-1] - A[N*N-2]*b_p;  
	b[N-1] = (b[N-1] - b[N-1-1]*A[N*N - 2])/a_p;

	for(uint32_t i = 0; i < N-1; i++)
	{
		b[N-i-2] = b[N-i-2] - b[N-i-1]*b_a[N-i-2];
	}

}

void
light_physic_rope(struct light_physic_particle *restrict particle, uint32_t particle_count)
{
	
	const uint32_t N = particle_count-1;
	const uint32_t M = particle_count;
	
	
	

	float JJT[N*N];
	memset(JJT, 0, sizeof(JJT));


	struct vec3 J[N];
	for(uint32_t i = 0; i < N; i++)
	{
		struct vec3 v = v3sub(particle[i].position, particle[i+1].position);
		float len = v3len(v);

		J[i] = v3scl(v, 1/len);
	}
	
	

	for(uint32_t i = 0; i < N-1; i++)
	{
		struct vec3 a = J[i];
		float J_a = v3dot(a,a)*(particle[i].mass + particle[i+1].mass)/(particle[i].mass*particle[i+1].mass);

		struct vec3 b = J[i+1];
		float J_b = -v3dot(a,b)/particle[i+1].mass;

		JJT[N*i + i] = J_a;
		JJT[N*i + i + 1] = J_b;
		JJT[N*i + i + N] = J_b;

	}

	struct vec3 a = J[N-1];
	float J_a = v3dot(a,a)*(particle[M-2].mass + particle[M-1].mass)/(particle[M-2].mass*particle[M-1].mass);

	JJT[N*(N-1) + N - 1] = J_a;

	float Jdqdt[N];
	for(uint32_t i = 0; i < N; i++)
	{
		Jdqdt[i] = -1.0*(v3dot(J[i], particle[i].velocity) - v3dot(J[i], particle[i+1].velocity)); 
	}


	light_physic_rope_solve(JJT, Jdqdt, N);
	float *lambda = Jdqdt;


	struct vec3 p[M];

	p[0] = v3scl(J[0], lambda[0]);
	for(uint32_t i = 1; i < M-1;i++)
	{
		p[i] = v3add(v3scl(J[i-1], -lambda[i-1]), v3scl(J[i], lambda[i]));
	}
	p[M-1] = v3scl(J[N-1], -lambda[N-1]);
	
	for(uint32_t i = 0; i < M; i++)
	{
		particle[i].velocity = v3add(particle[i].velocity, v3scl(p[i], 1.0/particle[i].mass));
	}
}
