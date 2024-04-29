#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define OBJECT_NONE 0xFFFFFFFF

enum 
{
	MAX, 
	MIN, 
};

struct tree_node
{
	uint32_t op_type;
	uint32_t node_index;
	uint32_t object_index;
};

struct tree_node_root
{
	uint32_t levels;
	uint32_t op_type;
	uint32_t node_left_index;
	uint32_t node_right_index;
};

struct object
{
	int dummy;
};


void test(struct tree_node_root *root_node, struct tree_node *nodes, struct object *o)
{
	/* Node index */
	uint32_t left_nodes[root_node->levels];
	uint32_t right_nodes[root_node->levels];
	
	left_nodes[0] = root_node->node_left_index;	
	right_nodes[0] = root_node->node_right_index;	
	
	/* Write node index to buffer */
	for(uint32_t i = 1; i < root_node->levels; i++)
	{
		left_nodes[i] = nodes[left_nodes[i-1]].node_index;
		right_nodes[i] = nodes[right_nodes[i-1]].node_index;
	}
	
	uint32_t left_result[root_node->levels];
	uint32_t right_result[root_node->levels];

	/* Access the nodes backwards */
	uint32_t left = left_nodes[root_node->levels-1];	
	uint32_t left_res = o[nodes[left].object_index].dummy;
	left_result[root_node->levels-1] = left_res;

	uint32_t right = right_nodes[root_node->levels-1];	
	uint32_t right_res = o[nodes[right].object_index].dummy;
	right_result[root_node->levels-1] = right_res;

	for(uint32_t i = 2; i < root_node->levels+1; i++)
	{
		uint32_t index = root_node->levels - i;
		uint32_t left = left_nodes[index];	
		uint32_t right = right_nodes[index];	

		uint32_t left_res = o[nodes[left].object_index].dummy + left_result[index+1];
		left_result[index] = left_res;

		uint32_t right_res = o[nodes[right].object_index].dummy + right_result[index+1];
		right_result[index] = right_res;
	}

	uint32_t result = left_result[0] + right_result[0];
	printf("result=%u\n", result);

}


int main(int args, char *argv[])
{
	{
		struct object o[6];
		for(uint32_t i = 0; i < 6; i++)
		{
			o[i].dummy = i;	
		}
		
		struct tree_node_root root_node;
		root_node.levels = 1;
		root_node.op_type = MIN;
		root_node.node_left_index = 0;
		root_node.node_right_index = 1;

		struct tree_node nodes[2];
		nodes[0].op_type = MAX;
		nodes[0].node_index = 5;
		nodes[0].object_index = 2;

		nodes[1].op_type = MAX;
		nodes[1].node_index = 3;
		nodes[1].object_index = 4;

		test(&root_node, nodes, o);
	}

	{
		struct object o[6];
		for(uint32_t i = 0; i < 6; i++)
		{
			o[i].dummy = i;	
		}
		
		struct tree_node_root root_node;
		root_node.levels = 2;
		root_node.op_type = MIN;
		root_node.node_left_index = 0;
		root_node.node_right_index = 1;

		struct tree_node nodes[4];
		nodes[0].op_type = MAX;
		nodes[0].node_index = 2;
		nodes[0].object_index = 2;

		nodes[1].op_type = MAX;
		nodes[1].node_index = 3;
		nodes[1].object_index = 4;
		
		nodes[2].op_type = MAX;
		nodes[2].node_index = 5;
		nodes[2].object_index = 1;

		nodes[3].op_type = MAX;
		nodes[3].node_index = 3;
		nodes[3].object_index = 5;


		test(&root_node, nodes, o);
	}


	{
		struct object o[6];
		for(uint32_t i = 0; i < 6; i++)
		{
			o[i].dummy = i;	
		}
		
		struct tree_node_root root_node;
		root_node.levels = 3;
		root_node.op_type = MIN;
		root_node.node_left_index = 0;
		root_node.node_right_index = 1;

		struct tree_node nodes[6];
		nodes[0].op_type = MAX;
		nodes[0].node_index = 2;
		nodes[0].object_index = 0;

		nodes[1].op_type = MAX;
		nodes[1].node_index = 3;
		nodes[1].object_index = 1;
		
		nodes[2].op_type = MAX;
		nodes[2].node_index = 4;
		nodes[2].object_index = 2;

		nodes[3].op_type = MAX;
		nodes[3].node_index = 5;
		nodes[3].object_index = 3;
	
		nodes[4].op_type = MAX;
		nodes[4].node_index = 0;
		nodes[4].object_index = 4;

		nodes[5].op_type = MAX;
		nodes[5].node_index = 0;
		nodes[5].object_index = 5;



		test(&root_node, nodes, o);
	}


	return 0;
}
