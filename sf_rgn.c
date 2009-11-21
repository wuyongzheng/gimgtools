#include "gimglib.h"

void dump_rgn (struct subfile_struct *sf)
{
	struct garmin_rgn *header = (struct garmin_rgn *)sf->header;
	int progress = 0;

	assert(sf->typeid == ST_RGN);

	dump_comm(sf->header);

	printf("=== RGN HEADER ===\n");

	if (header->comm.hlen < 0x1d)
		goto headerfini;
	progress = 0x1d;
	printf("RGN1:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->rgn1_offset, sf->offset + header->rgn1_offset, header->rgn1_length);

	if (header->comm.hlen < 0x7d)
		goto headerfini;
	progress = 0x7d;
	printf("RGN2:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->rgn2_offset, sf->offset + header->rgn2_offset, header->rgn2_length);
	printf("unknown_025: %s\n", dump_unknown_bytes(header->unknown_025, sizeof(header->unknown_025)));
	printf("RGN3:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->rgn3_offset, sf->offset + header->rgn3_offset, header->rgn3_length);
	printf("unknown_041: %s\n", dump_unknown_bytes(header->unknown_041, sizeof(header->unknown_041)));
	printf("RGN4:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->rgn4_offset, sf->offset + header->rgn4_offset, header->rgn4_length);
	printf("unknown_05d: %s\n", dump_unknown_bytes(header->unknown_05d, sizeof(header->unknown_05d)));
	printf("RGN5:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->rgn5_offset, sf->offset + header->rgn5_offset, header->rgn5_length);
	printf("unknown_079: 0x%x\n", header->unknown_079);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	return;
}
