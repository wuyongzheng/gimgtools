#include "gimglib.h"

void dump_mar (struct subfile_struct *sf)
{
	assert(sf->typeid == ST_MAR);

	dump_comm(sf->header);

	printf("=== DEM HEADER: TODO ===\n");
}
