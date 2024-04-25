
struct light_physic_kd_tree_node
{
	struct light_physic_kd_tree_node *left, *right;
	
	struct vec3 	node_body_position;
	uint32_t 	node_body_index;
};


struct light_physic_kd_tree_node_find_result 
{
	struct light_physic_kd_tree_node *node;
	float distance;
};


void 
light_physic_kd_tree_node_swap(struct light_physic_kd_tree_node *lhs, struct light_physic_kd_tree_node *rhs)
{
	struct light_physic_kd_tree_node tree_node;

	/* Store lhs */
	memcpy(&tree_node.node_body_position, &lhs->node_body_position, sizeof(tree_node.node_body_position));
	tree_node.node_body_index = lhs->node_body_index;
	
	/* Write over lhs with rhs */
	memcpy(&lhs->node_body_position, &rhs->node_body_position, sizeof(lhs->node_body_position));
	lhs->node_body_index = rhs->node_body_index;

	/* write over rhs with stored */
	memcpy(&rhs->node_body_position, &tree_node.node_body_position, sizeof(rhs->node_body_position));
	rhs->node_body_index = tree_node.node_body_index;

}
struct light_physic_kd_tree_node *
light_physic_kd_tree_median(struct light_physic_kd_tree_node *start, struct light_physic_kd_tree_node *end, uint32_t node_index)
{
	if(end <= start) return NULL;
	if(end == start + 1) return start;

	float pivot = 0.0f;
	struct light_physic_kd_tree_node *p, *store, *md = start + (end - start)/2;
	while(1)
	{
		float *v = (float *)&md->node_body_position;
		pivot = v[node_index];

		light_physic_kd_tree_node_swap(md, end -1);
		for(store = p = start; p < end; p++){

			float *p_v  = (float *)&p->node_body_position;
			if(p_v[node_index] < pivot)
			{
				if(p != store)
				{
					light_physic_kd_tree_node_swap(p, store);	
				}
				store++;
			}
		}

		light_physic_kd_tree_node_swap(store, end - 1);

		float *store_v  = (float *)&store->node_body_position;
		float *md_v  = (float *)&md->node_body_position;

		if(store_v[node_index] == md_v[node_index])
		{
			return md;	
		}
		
		if(store > md) end = store;
		else start = store;
	}
	return NULL;
}

struct light_physic_kd_tree_node *
light_physic_kd_tree_build_internal(struct light_physic_kd_tree_node *tree_node, uint32_t tree_node_count, uint32_t tree_node_index)
{
	if(tree_node_count == 0)
		return NULL;

	struct light_physic_kd_tree_node * root = NULL;
	if((root = light_physic_kd_tree_median(tree_node, tree_node + tree_node_count, tree_node_index)))
	{
		tree_node_index = (tree_node_index + 1)%3;
		root->left = light_physic_kd_tree_build_internal(tree_node, root - tree_node, tree_node_index);
		root->right = light_physic_kd_tree_build_internal(root+1, tree_node + tree_node_count - (root + 1), tree_node_index);
	}

	return root;
}

struct light_physic_kd_tree_node *
light_physic_kd_tree_build(struct light_physic_body *physic_body, struct light_physic_kd_tree_node *tree_node, uint32_t count)
{
	assert(physic_body != NULL);
	assert(tree_node != NULL);

	for(uint32_t i = 0; i < count; i++)
	{
		tree_node[i].node_body_index = i;
		memcpy(&tree_node[i].node_body_position, &physic_body[i].position, sizeof(tree_node[i].node_body_position));
	}

	return light_physic_kd_tree_build_internal(tree_node, count, 0);
}


float light_physic_kd_tree_distance(struct light_physic_kd_tree_node *node, struct light_physic_kd_tree_node *find)
{
	float x = node->node_body_position.x - find->node_body_position.x;
	float y = node->node_body_position.y - find->node_body_position.y;
	float z = node->node_body_position.z - find->node_body_position.z;
	return x*x + y*y + z*z;
}

void
light_physic_kd_tree_find_closest(struct light_physic_kd_tree_node *node, struct light_physic_kd_tree_node *find, uint32_t node_index, struct light_physic_kd_tree_node_find_result *result)
{
	if(node == NULL)
		return;
	
	float distance = light_physic_kd_tree_distance(node, find);
	if(!result->node || distance < result->distance)
	{
		result->node = node;
		result->distance = distance;
	}
		
	float *node_pos  = (float *)&node->node_body_position;
	float *find_pos  = (float *)&find->node_body_position;

	float di = node_pos[node_index] - find_pos[node_index];

	node_index = (node_index + 1)%3;

	light_physic_kd_tree_find_closest(di > 0 ? node->left : node->right, find, node_index, result);
	if(di*di >= result->distance) return;

	light_physic_kd_tree_find_closest(di > 0 ? node->right: node->left, find, node_index, result);
}




void
light_physic_kd_tree_find_closest_N(struct light_physic_kd_tree_node *node, struct light_physic_kd_tree_node *find, uint32_t node_index, struct light_physic_kd_tree_node_find_result *result, uint32_t result_count)
{
	if(node == NULL)
		return;
	
	float distance = light_physic_kd_tree_distance(node, find);

	/* Assume that the first is max, then compare with others. */
	uint32_t node_replace_index = 0;
	float max = result[node_replace_index].distance;
	for(uint32_t i = 0; i < result_count; i++)
	{
		if(result[i].distance > result[node_replace_index].distance)
		{
			node_replace_index = i;
			max = result[i].distance;
		}
	}

	if(distance < max)
	{
		result[node_replace_index].node = node;
		result[node_replace_index].distance = distance;
	}
	
	float *node_pos  = (float *)&node->node_body_position;
	float *find_pos  = (float *)&find->node_body_position;

	float di = node_pos[node_index] - find_pos[node_index];

	node_index = (node_index + 1)%3;
	light_physic_kd_tree_find_closest_N(di > 0 ? node->left : node->right, find, node_index, result, result_count);

	uint32_t node_worst_index = 0;
	for(uint32_t i = 0; i < result_count; i++)
	{
		if(result[i].distance > result[node_worst_index].distance)
		{
			node_worst_index = i;	
		}	
	}
	if(di*di >= result[node_worst_index].distance) return;
	light_physic_kd_tree_find_closest_N(di > 0 ? node->right: node->left, find, node_index, result, result_count);
}

