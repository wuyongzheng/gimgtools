#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "gimginfo.h"

static void dump_subdiv_rec (uint8_t *ptr, int index, int level, int *pnum_nleave)
{
	struct garmin_tre_subdiv *div;

	if (level == 0) {
		if (*pnum_nleave == 0)
			*pnum_nleave = index - 1;
		div = (struct garmin_tre_subdiv *)(ptr + (index - 1) * (sizeof(struct garmin_tre_subdiv) - 2) + *pnum_nleave * 2);
		while (1) {
			printf("0 off=%06x, ele=%02x, lng=%8d(%+11.6f), lat=%8d(%+11.6f), width=%5d, height=%5d\n",
					bytes_to_uint24(div->rgn_offset), div->elements,
					bytes_to_sint24(div->center_lng), bytes_to_sint24(div->center_lng) * (360.0 / 0x01000000),
					bytes_to_sint24(div->center_lat), bytes_to_sint24(div->center_lat) * (360.0 / 0x01000000),
					div->width, div->height);
			if (div->terminate)
				break;
			index ++;
			div = (struct garmin_tre_subdiv *)(ptr + (index - 1) * (sizeof(struct garmin_tre_subdiv) - 2) + *pnum_nleave * 2);
		}
	} else {
		div = (struct garmin_tre_subdiv *)(ptr + sizeof(struct garmin_tre_subdiv) * (index - 1));
		while (1) {
			printf("%d off=%06x, ele=%02x, lng=%8d(%+11.6f), lat=%8d(%+11.6f), width=%5d, height=%5d\n",
					level,
					bytes_to_uint24(div->rgn_offset), div->elements,
					bytes_to_sint24(div->center_lng), bytes_to_sint24(div->center_lng) * (360.0 / 0x01000000),
					bytes_to_sint24(div->center_lat), bytes_to_sint24(div->center_lat) * (360.0 / 0x01000000),
					div->width, div->height);
			dump_subdiv_rec(ptr, div->next, level - 1, pnum_nleave);
			if (div->terminate)
				break;
			index ++;
			div = (struct garmin_tre_subdiv *)(ptr + sizeof(struct garmin_tre_subdiv) * (index - 1));
		}
	}
}

static void dump_subdiv (uint8_t *ptr, int levels)
{
	int num_nleave = 0;
	dump_subdiv_rec(ptr, 1, levels, &num_nleave);
}

static void dump_maplevels (struct garmin_tre_map_level *levels, int num)
{
	int i;
	for (i = 0; i < num; i ++) {
		printf("level=%d bit456=0x%x %s bits=%2d nsubdiv=%d\n",
				levels[i].level, levels[i].bit456,
				levels[i].inherited ? "inher" : "noinh",
				levels[i].bits, levels[i].nsubdiv);
	}
}

void dump_tre (struct subfile_struct *tre)
{
	struct garmin_tre *header = (struct garmin_tre *)tre->header;

	assert(tre->typeid == ST_TRE);

	dump_comm(tre->header);

	printf("=== TRE HEADER ===\n");
	printf("Bound:                  %d(%f) %d(%f) %d(%f) %d(%f)\n",
			bytes_to_sint24(header->northbound), bytes_to_sint24(header->northbound) * (360.0 / 0x01000000),
			bytes_to_sint24(header->eastbound),  bytes_to_sint24(header->eastbound) * (360.0 / 0x01000000),
			bytes_to_sint24(header->southbound), bytes_to_sint24(header->southbound) * (360.0 / 0x01000000),
			bytes_to_sint24(header->westbound),  bytes_to_sint24(header->westbound) * (360.0 / 0x01000000));
	printf("TRE1 Map Levels:        reloff=0x%x absoff=0x%x size=0x%x\n",
			header->tre1_offset, tre->offset + header->tre1_offset, header->tre1_size);
	printf("TRE2 Subdivisions:      reloff=0x%x absoff=0x%x size=0x%x\n",
			header->tre2_offset, tre->offset + header->tre2_offset, header->tre2_size);
	printf("TRE3 Copyright:         reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre3_offset, tre->offset + header->tre3_offset, header->tre3_size, header->tre3_rec_size);
	printf("unknown_03b:            %s\n", dump_unknown_bytes(header->unknown_03b, sizeof(header->unknown_03b)));
	printf("POI Flags:              %s %s %s\n",
			header->POI_flags & 0x1 ? "transp" : "not-transp",
			header->POI_flags & 0x2 ? "street-before-num" : "num-before-street",
			header->POI_flags & 0x4 ? "zip-before-city" : "city-before-zip");
	printf("Draw Priority:          %d\n", header->drawprio);
	printf("unknown_041:            %s\n", dump_unknown_bytes(header->unknown_041, sizeof(header->unknown_041)));
	printf("TRE4 Polyline Overview: reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre4_offset, tre->offset + header->tre4_offset, header->tre4_size, header->tre4_rec_size);
	printf("unknown_054:            %s\n", dump_unknown_bytes(header->unknown_054, sizeof(header->unknown_054)));
	printf("TRE5 Polygon Overview:  reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre5_offset, tre->offset + header->tre5_offset, header->tre5_size, header->tre5_rec_size);
	printf("unknown_062:            %s\n", dump_unknown_bytes(header->unknown_062, sizeof(header->unknown_062)));
	printf("TRE6 Point Overview:    reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre6_offset, tre->offset + header->tre6_offset, header->tre6_size, header->tre6_rec_size);
	printf("unknown_070:            %s\n", dump_unknown_bytes(header->unknown_070, sizeof(header->unknown_070)));
	if (header->comm.hlen <= 0x74)
		goto dumpsubdiv;
	printf("Map ID:                 0x%x(%d)\n", header->mapID, header->mapID);
	if (header->comm.hlen <= 0x78)
		goto dumpsubdiv;
	printf("unknown_078:            %s\n", dump_unknown_bytes(header->unknown_078, sizeof(header->unknown_078)));
	printf("TRE7:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre7_offset, tre->offset + header->tre7_offset, header->tre7_size, header->tre7_rec_size);
	printf("unknown_086:            %s\n", dump_unknown_bytes(header->unknown_086, sizeof(header->unknown_086)));
	printf("TRE8:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre8_offset, tre->offset + header->tre8_offset, header->tre8_size, header->tre8_rec_size);
	printf("unknown_092:            %s\n", dump_unknown_bytes(header->unknown_092, sizeof(header->unknown_092)));
	if (header->comm.hlen <= 0x9a)
		goto dumpsubdiv;
	printf("Encrpytion Key:         %s\n", dump_unknown_bytes(header->key, sizeof(header->key)));
	printf("TRE9:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre9_offset, tre->offset + header->tre9_offset, header->tre9_size, header->tre9_rec_size);
	printf("unknown_0b8:            %s\n", dump_unknown_bytes(header->unknown_0b8, sizeof(header->unknown_0b8)));
	if (header->comm.hlen <= 0xbc)
		goto dumpsubdiv;
	printf("from 0xbc to 0x%x (%d bytes): %s\n",
			header->comm.hlen - 1,
			header->comm.hlen - 0xbc,
			dump_unknown_bytes((uint8_t *)header + 0xbc, header->comm.hlen - 0xbc));

dumpsubdiv:
	printf("=== MAP LEVELS ===\n");
	if (header->comm.locked)
		printf("locked\n");
	else
		dump_maplevels((struct garmin_tre_map_level *)(tre->base + header->tre1_offset),
				header->tre1_size / sizeof(struct garmin_tre_map_level));

	printf("=== SUBDIVISIONS ===\n");
	dump_subdiv(tre->base + header->tre2_offset, header->tre1_size / 4 - 1);
}
