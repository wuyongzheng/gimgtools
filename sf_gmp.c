#include "gimglib.h"

void dump_gmp (struct subfile_struct *sf)
{
	struct garmin_gmp *header = (struct garmin_gmp *)sf->header;

	assert(sf->typeid == ST_GMP);

	dump_comm(sf->header);

	assert(header->comm.hlen >= sizeof(struct garmin_gmp));
	printf("=== GMP HEADER ===\n");
	if (header->comm.hlen >= 0x19) printf("unknown_015: 0x%x\n", header->unknown_015);
	if (header->comm.hlen >= 0x1d) printf("TRE Offset:  0x%x\n", header->tre_offset);
	if (header->comm.hlen >= 0x21) printf("RGN Offset:  0x%x\n", header->rgn_offset);
	if (header->comm.hlen >= 0x25) printf("LBL Offset:  0x%x\n", header->lbl_offset);
	if (header->comm.hlen >= 0x29) printf("NET Offset:  0x%x\n", header->net_offset);
	if (header->comm.hlen >= 0x2d) printf("NOD Offset:  0x%x\n", header->nod_offset);
	if (header->comm.hlen >= 0x31) printf("DEM Offset:  0x%x\n", header->dem_offset);
	if (header->comm.hlen >= 0x35) printf("MAR Offset:  0x%x\n", header->mar_offset);

	if (header->comm.hlen > sizeof(struct garmin_gmp))
		printf("from 0x%lx to 0x%x (0x%lx bytes): %s\n",
				sizeof(struct garmin_gmp), header->comm.hlen - 1,
				header->comm.hlen - sizeof(struct garmin_gmp),
				dump_unknown_bytes((uint8_t *)header + sizeof(struct garmin_gmp), header->comm.hlen - sizeof(struct garmin_gmp)));
}
