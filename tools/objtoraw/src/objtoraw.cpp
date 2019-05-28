#include <stdio.h>

#include <model/raw_model.h>
#include <model/obj_model.h>


int main( int args, char *argv[])
{
	int result = -1;
	if( args != 3)
	{
		printf("Usage: %s in out", argv[0] );
		return (result);
	}
	
	const char * source_model = argv[1];
	const char * dest_model = argv[2];
	
	obj_model src_model;
	bool loaded = obj_load(source_model, &src_model);
	if(loaded)
	{
		FILE * fp = fopen(argv[2] , "wb");
		if(fp)
		{
			
			raw_model * model = raw_model_create(src_model.vertices, src_model.vertex_count, src_model.indices,  src_model.indice_count);
			if(model)
			{									  
				raw_model_save(fp, model);
				raw_model_release(model);
				printf("Saved model %s", dest_model);
			}
			else
			{
				printf("Could not create model %s \n", dest_model);
			}
			
			fclose(fp);
		}
		else
		{
			printf("Could not open %s for writing. \n", dest_model);
		}
	
		
		obj_release(&src_model);
		result = 1;
	}
	else
	{
		printf("Could not open %s for reading. \n", source_model);
	}
	
	return (result);
}
