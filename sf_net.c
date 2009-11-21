#include "gimglib.h"

void dump_net (struct subfile_struct *sf)
{
	struct garmin_net *header = (struct garmin_net *)sf->header;
	int progress = 0;

	assert(sf->typeid == ST_NET);

	dump_comm(sf->header);

	printf("=== NET HEADER ===\n");

	if (header->comm.hlen < 0x37)
		goto headerfini;
	progress = 0x37;
	printf("NET1 Road Def:       reloff=0x%x absoff=0x%x size=0x%x shift=%d\n",
			header->net1_offset, sf->offset + header->net1_offset, header->net1_length, header->net1_shift);
	printf("NET2 Segmented Road: reloff=0x%x absoff=0x%x size=0x%x shift=%d\n",
			header->net2_offset, sf->offset + header->net2_offset, header->net2_length, header->net2_shift);
	printf("NET3 Sorted Road:    reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->net3_offset, sf->offset + header->net3_offset, header->net3_length, header->net3_recsize);
	printf("unknown_031:         0x%x\n", header->unknown_031);
	printf("unknown_035:         0x%x\n", header->unknown_035);

	if (header->comm.hlen < 0x3b)
		goto headerfini;
	progress = 0x3b;
	printf("unknown_037:         0x%x\n", header->unknown_037);

	if (header->comm.hlen < 0x64)
		goto headerfini;
	progress = 0x64;
	printf("unknown_03b:         0x%x\n", header->unknown_03b);
	printf("unknown_03f:         0x%x\n", header->unknown_03f);
	printf("NET4:                reloff=0x%x absoff=0x%x size=0x%x ?04b=0x%x\n",
			header->net4_offset, sf->offset + header->net4_offset, header->net4_length, header->net4_u);
	printf("NET5:                reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->net5_offset, sf->offset + header->net5_offset, header->net5_length, header->net5_recsize);
	printf("NET6:                reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->net6_offset, sf->offset + header->net6_offset, header->net6_length, header->net6_recsize);
	printf("unknown_060:         0x%x\n", header->unknown_060);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	return;
}
