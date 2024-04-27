#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define OBJECT_NONE 0xFFFFFFFF

enum 
{
	OBJECT_OBJECT, 
	NODE_NODE, 
	OBJECT_NODE, 	
};

struct tree_node
{
	uint32_t index_type;
	uint32_t left_index;
	uint32_t right_index;
};

struct object
{
	int dummy;
};


void test(struct tree_node *root_node, struct tree_node *nodes, uint32_t node_total_count, struct object *o, uint32_t levels)
{
	/* Access tree backwards */	
	uint32_t node_count_sum = 0;
	uint32_t node_value[node_total_count];

	for(uint32_t i = 0; i < levels; i++)
	{
		uint32_t node_count = 1;
		for(uint32_t j = 0; j < levels - i; j++)
		{
			node_count = node_count*2;	
		}
		node_count_sum += node_count;

		for(uint32_t j = 0; j < node_count; j++)
		{
			uint32_t index = node_total_count - node_count_sum + j;

			/* Part of computation example */
			uint32_t type = nodes[index].index_type;
			switch(type)
			{
				case OBJECT_OBJECT:
					{
						uint32_t left = nodes[index].left_index == OBJECT_NONE ? 0 : o[nodes[index].left_index].dummy;
						uint32_t right = nodes[index].right_index == OBJECT_NONE ? 0 : o[nodes[index].right_index].dummy;
						node_value[index] = left + right;
					}
				break;	
				case OBJECT_NODE:
					{
						uint32_t left = nodes[index].left_index == OBJECT_NONE ? 0 : o[nodes[index].left_index].dummy;
						node_value[index] = left + node_value[nodes[index].right_index];
					}
				break;	
				case NODE_NODE:
					{
						node_value[index] = node_value[nodes[index].left_index] + node_value[nodes[index].right_index];
					}
				break;	
			}

			printf("level=%u, index=%u, value=%u \n", levels-i, index, node_value[index]);	
		}
	}

	/* Root node */
	uint32_t result; 
	uint32_t type = root_node->index_type;
	switch(type)
	{
		case OBJECT_OBJECT:
			{
				uint32_t left = root_node->left_index == OBJECT_NONE ? 0 : o[root_node->left_index].dummy;
				uint32_t right = root_node->right_index == OBJECT_NONE ? 0 : o[root_node->right_index].dummy;
				result = left + right;
			}
		break;	
		case OBJECT_NODE:
			{
				uint32_t left = root_node->left_index == OBJECT_NONE ? 0 : o[root_node->left_index].dummy;
				result = left + node_value[root_node->right_index];
			}
		break;	
		case NODE_NODE:
			{
				result = node_value[root_node->left_index] + node_value[root_node->right_index];
			}
		break;	
	}

	printf("result=%u \n", result);
}


int main(int args, char *argv[])
{
	{
		struct object o[6];
		for(uint32_t i = 0; i < 6; i++)
		{
			o[i].dummy = i;	
		}
		
		uint32_t levels = 2;
		struct tree_node root_node;
		root_node.index_type = NODE_NODE;
		root_node.left_index = 0;
		root_node.right_index = 1;

		struct tree_node nodes[2+4];
		nodes[0].index_type = NODE_NODE;
		nodes[0].left_index = 1;
		nodes[0].right_index = 2;

		nodes[1].index_type = OBJECT_NODE;
		nodes[1].left_index = 3;
		nodes[1].right_index = 4;

		nodes[2].index_type = OBJECT_OBJECT;
		nodes[2].left_index = 0;
		nodes[2].right_index = 1;

		nodes[3].index_type = OBJECT_OBJECT;
		nodes[3].left_index = 2;
		nodes[3].right_index = 3;

		nodes[4].index_type = OBJECT_OBJECT;
		nodes[4].left_index = OBJECT_NONE;
		nodes[4].right_index = OBJECT_NONE;

		nodes[5].index_type = OBJECT_OBJECT;
		nodes[5].left_index = 0;
		nodes[5].right_index = OBJECT_NONE;


		uint32_t node_total_count = 2+4;
		test(&root_node, nodes, node_total_count, o, levels);
	}

	{
		struct object o[4];
		for(uint32_t i = 0; i < 4; i++)
		{
			o[i].dummy = i;	
		}
		
		uint32_t levels = 1;
		struct tree_node root_node;
		root_node.index_type = NODE_NODE;
		root_node.left_index = 0;
		root_node.right_index = 1;

		struct tree_node nodes[2+4];
		nodes[0].index_type = OBJECT_OBJECT;
		nodes[0].left_index = 0;
		nodes[0].right_index = 1;

		nodes[1].index_type = OBJECT_OBJECT;
		nodes[1].left_index = 2;
		nodes[1].right_index = 3;

		uint32_t node_total_count = 2;
		test(&root_node, nodes, node_total_count, o, levels);
	
	}


	return 0;
}
