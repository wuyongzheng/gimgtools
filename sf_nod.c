#include "gimglib.h"

void dump_nod (struct subfile_struct *sf)
{
	struct garmin_nod *header = (struct garmin_nod *)sf->header;
	int progress = 0;

	assert(sf->typeid == ST_NOD);

	dump_comm(sf->header);

	printf("=== NOD HEADER ===\n");

	if (header->comm.hlen < 0x3f)
		goto headerfini;
	progress = 0x3f;
	printf("NOD1 Node Data:         reloff=0x%x absoff=0x%x size=0x%x\n",
			header->nod1_offset, sf->offset + header->nod1_offset, header->nod1_length);
	printf("nod_bits, align:        {0x%x 0x%x 0x%x 0x%x}, %d\n",
			header->nod_bits[0], header->nod_bits[1], header->nod_bits[2], header->nod_bits[3], header->align);
	printf("unknown_025:            %d\n", header->unknown_025);
	printf("roadptrsize:            %d\n", header->roadptrsize);
	printf("NOD2 Road Data:         reloff=0x%x absoff=0x%x size=0x%x\n",
			header->nod2_offset, sf->offset + header->nod2_offset, header->nod2_length);
	printf("unknown_02d:            0x%x\n", header->unknown_02d);
	printf("NOD3 Boundary Node:     reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->nod3_offset, sf->offset + header->nod3_offset, header->nod3_length, header->nod3_recsize);
	printf("unknown_03c:            0x%x\n", header->unknown_03c);

	if (header->comm.hlen < 0x7f)
		goto headerfini;
	progress = 0x7f;
	printf("NOD4:                   reloff=0x%x absoff=0x%x size=0x%x\n",
			header->nod4_offset, sf->offset + header->nod4_offset, header->nod4_length);
	printf("unknown_047:            0x%x\n", header->unknown_047);
	printf("unknown_04b:            0x%x\n", header->unknown_04b);
	printf("unknown_04f:            0x%x\n", header->unknown_04f);
	printf("unknown_053:            0x%x\n", header->unknown_053);
	printf("unknown_057:            0x%x\n", header->unknown_057);
	printf("unknown_05b:            %s\n",
			dump_unknown_bytes(header->unknown_05b, sizeof(header->unknown_05b)));
	printf("NOD5:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->nod5_offset, sf->offset + header->nod5_offset, header->nod5_length, header->nod5_recsize);
	printf("NOD6:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->nod6_offset, sf->offset + header->nod6_offset, header->nod6_length, header->nod6_recsize);
	printf("unknown_07b:            0x%x\n", header->unknown_07b);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	return;
}
