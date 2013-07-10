#include "gimglib.h"

void dump_dem (struct subfile_struct *sf)
{
	struct garmin_dem *header = (struct garmin_dem *)sf->header;

	assert(sf->typeid == ST_DEM);

	dump_comm(sf->header);

	printf("=== DEM HEADER ===\n");
	printf("Flags:             0x%x\n", header->flags);
	printf("Zoom Levels:       0x%x\n", header->zoom_levels);
	printf("Reserved:          0x%x\n", header->reserved0);
	printf("Record Size:       0x%x\n", header->record_size);
	printf("Points to Block 3: 0x%x\n", header->points_to_block3);
	printf("Reserved:          0x%x\n", header->reserved2);

	if (header->comm.hlen > sizeof(struct garmin_gmp))
		printf("from 0x%lx to 0x%x (0x%lx bytes): %s\n",
				sizeof(struct garmin_gmp), header->comm.hlen - 1,
				header->comm.hlen - sizeof(struct garmin_gmp),
				dump_unknown_bytes((uint8_t *)header + sizeof(struct garmin_gmp), header->comm.hlen - sizeof(struct garmin_gmp)));

	return;
}
