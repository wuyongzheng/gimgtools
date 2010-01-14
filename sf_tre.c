#include "gimglib.h"

static void dump_poverview (uint8_t *ptr, int num, int size)
{
	while(num -- > 0) {
		int i;
		printf("type=0x%2x", ptr[0]);
		printf(" maxl=%2d", ptr[1]);
		for (i = 2; i < size; i ++)
			printf(" subtype=0x%2x", ptr[i]);
		printf("\n");
		ptr += size;
	}
}

static void dump_subdiv (uint8_t *ptr, int level_num, struct garmin_tre_map_level *levels, struct subfile_struct *rgn)
{
	int level, global_index, level_index;

	for (level = 0, global_index = 1; level < level_num; level ++) {
		for (level_index = 0; level_index < levels[level].nsubdiv; level_index ++, global_index ++) {
			struct garmin_tre_subdiv *div = (struct garmin_tre_subdiv *)ptr;
			int bitshift = 24 - levels[level].bits;

			printf("%5d %d off=%06x ele=%02x+%d, lng=%8d(%s), lat=%8d(%s), w=%5d, h=%5d %c",
					global_index, level,
					bytes_to_uint24(div->rgn_offset), div->elements, div->unknownbit,
					bytes_to_sint24(div->center_lng), sint24_to_lng(bytes_to_sint24(div->center_lng)),
					bytes_to_sint24(div->center_lat), sint24_to_lat(bytes_to_sint24(div->center_lat)),
					div->width<<bitshift, div->height<<bitshift, div->terminate ? 'T' : ' ');
			if (level == level_num - 1)
				printf("\n");
			else
				printf(" next=%5d\n", div->next);

#if 0
			if (rgn && div->unknownbit) {
				int rgn1_offset = ((struct garmin_rgn *)rgn->header)->rgn1_offset;
				int offset = bytes_to_uint24(div->rgn_offset);
				int next_offset = level == level_num - 1 ? bytes_to_uint24(div[1].rgn_offset - 2) : bytes_to_uint24(div[1].rgn_offset);
				if (div->elements == 0)
					;
				else if (div->elements == 0x10 || div->elements == 0x20)
					offset += *(short *)(rgn->base + rgn1_offset + offset);
				else if (div->elements == 0x30)
					offset += *(short *)(rgn->base + rgn1_offset + offset + 2);
				else
					printf("???\n");
				printf("RGN: %s\n", dump_unknown_bytes(rgn->base + rgn1_offset + offset, next_offset - offset));
			}
#endif

			ptr += level == level_num - 1 ? sizeof(struct garmin_tre_subdiv) - 2 : sizeof(struct garmin_tre_subdiv);
		}
	}
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

static void dump_tre7 (uint8_t *ptr, int size, int rec_size, struct garmin_tre_map_level *levels, struct subfile_struct *rgn)
{
	int sdidx = 1;
	while (size >= rec_size) {
		printf("%4d:", sdidx ++);
		printf(" rgn2_reloff=0x%x", *(uint32_t *)ptr);
		if (rec_size >= 8)
			printf(" rgn3_reloff=0x%x", *(uint32_t *)(ptr+4));
		if (rec_size >= 12)
			printf(" rgn4_reloff=0x%x", *(uint32_t *)(ptr+8));
		if (rec_size >= 16)
			printf(" rgn5_reloff=0x%x", *(uint32_t *)(ptr+12));
		if (rec_size % 4)
			printf(" flag=0x%x\n", *(ptr+rec_size-1));
		else
			printf("\n");

#if 0
		if (*(uint32_t *)(ptr+rec_size) - *(uint32_t *)(ptr))
			printf("RGN2: %s\n", dump_unknown_bytes(rgn->base + ((struct garmin_rgn *)rgn->header)->rgn2_offset + *(uint32_t *)(ptr), *(uint32_t *)(ptr+rec_size) - *(uint32_t *)(ptr)));
		if (rec_size >= 8 && *(uint32_t *)(ptr+rec_size+4) - *(uint32_t *)(ptr+4))
			printf("RGN3: %s\n", dump_unknown_bytes(rgn->base + ((struct garmin_rgn *)rgn->header)->rgn3_offset + *(uint32_t *)(ptr+4), *(uint32_t *)(ptr+rec_size+4) - *(uint32_t *)(ptr+4)));
		if (rec_size >= 12 && *(uint32_t *)(ptr+rec_size+8) - *(uint32_t *)(ptr+8))
			printf("RGN4: %s\n", dump_unknown_bytes(rgn->base + ((struct garmin_rgn *)rgn->header)->rgn4_offset + *(uint32_t *)(ptr+8), *(uint32_t *)(ptr+rec_size+8) - *(uint32_t *)(ptr+8)));
		if (rec_size >= 16 && *(uint32_t *)(ptr+rec_size+12) - *(uint32_t *)(ptr+12))
			printf("RGN5: %s\n", dump_unknown_bytes(rgn->base + ((struct garmin_rgn *)rgn->header)->rgn5_offset + *(uint32_t *)(ptr+12), *(uint32_t *)(ptr+rec_size+12) - *(uint32_t *)(ptr+12)));
#endif

		ptr += rec_size;
		size -= rec_size;
	}
}

void dump_tre (struct subfile_struct *tre)
{
	struct garmin_tre *header = (struct garmin_tre *)tre->header;
	struct garmin_tre_map_level *maplevels;
	int progress = 0;

	assert(tre->typeid == ST_TRE);

	dump_comm(tre->header);

	printf("=== TRE HEADER ===\n");

	if (header->comm.hlen < 0x74)
		goto headerfini;
	progress = 0x74;
	printf("Bound:                  %d(%s) %d(%s) %d(%s) %d(%s)\n",
			bytes_to_sint24(header->northbound), sint24_to_lat(bytes_to_sint24(header->northbound)),
			bytes_to_sint24(header->eastbound),  sint24_to_lng(bytes_to_sint24(header->eastbound)),
			bytes_to_sint24(header->southbound), sint24_to_lat(bytes_to_sint24(header->southbound)),
			bytes_to_sint24(header->westbound),  sint24_to_lng(bytes_to_sint24(header->westbound)));
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

	if (header->comm.hlen < 0x78)
		goto headerfini;
	progress = 0x78;
	printf("Map ID:                 0x%x(%d)\n", header->mapID, header->mapID);

	if (header->comm.hlen < 0x9a)
		goto headerfini;
	progress = 0x9a;
	printf("unknown_078:            %s\n", dump_unknown_bytes(header->unknown_078, sizeof(header->unknown_078)));
	printf("TRE7:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre7_offset, tre->offset + header->tre7_offset, header->tre7_size, header->tre7_rec_size);
	printf("unknown_086:            %s\n", dump_unknown_bytes(header->unknown_086, sizeof(header->unknown_086)));
	printf("TRE8:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre8_offset, tre->offset + header->tre8_offset, header->tre8_size, header->tre8_rec_size);
	printf("unknown_094:            %s\n", dump_unknown_bytes(header->unknown_094, sizeof(header->unknown_094)));

	if (header->comm.hlen < 0xbc)
		goto headerfini;
	progress = 0xbc;
	printf("Encrpytion Key:         %s\n", dump_unknown_bytes(header->key, sizeof(header->key)));
	printf("TRE9:                   reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre9_offset, tre->offset + header->tre9_offset, header->tre9_size, header->tre9_rec_size);
	printf("unknown_0b8:            %s\n", dump_unknown_bytes(header->unknown_0b8, sizeof(header->unknown_0b8)));

	if (header->comm.hlen < 0xca)
		goto headerfini;
	progress = 0xca;
	printf("TRE10:                  reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->tre10_offset, tre->offset + header->tre10_offset, header->tre10_size, header->tre10_rec_size);
	printf("unknown_0c6:            %s\n", dump_unknown_bytes(header->unknown_0c6, sizeof(header->unknown_0c6)));

	if (header->comm.hlen < 0xce)
		goto headerfini;
	progress = 0xce;
	printf("unknown_0ca:            %s\n", dump_unknown_bytes(header->unknown_0ca, sizeof(header->unknown_0ca)));

	if (header->comm.hlen < 0xcf)
		goto headerfini;
	progress = 0xcf;
	printf("unknown_0ce:            0x%x\n", header->unknown_0ce);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	printf("=== MAP LEVELS ===\n");
	if (header->comm.locked) {
		printf("locked: %s\n", dump_unknown_bytes(tre->base + header->tre1_offset, header->tre1_size));
		maplevels = (struct garmin_tre_map_level *)malloc(header->tre1_size);
		unlockml((unsigned char *)maplevels,
				(unsigned char *)tre->base + header->tre1_offset,
				header->tre1_size,
				*(unsigned int *)(header->key+16));
		//TODO some simple verification, maybe?
	} else {
		maplevels = (struct garmin_tre_map_level *)malloc(header->tre1_size);
		memcpy(maplevels, tre->base + header->tre1_offset, header->tre1_size);
	}
	dump_maplevels(maplevels, header->tre1_size / sizeof(struct garmin_tre_map_level));

	printf("=== SUBDIVISIONS ===\n");
	dump_subdiv(tre->base + header->tre2_offset, header->tre1_size / 4, maplevels, tre->map ? tre->map->rgn : NULL);

	//TODO copyright

	if (header->tre4_size) {
		printf("=== POLYLINE OVERVIEWS ===\n");
		dump_poverview(tre->base + header->tre4_offset,
				header->tre4_size / header->tre4_rec_size,
				header->tre4_rec_size);
	}
	if (header->tre5_size) {
		printf("=== POLYGON OVERVIEWS ===\n");
		dump_poverview(tre->base + header->tre5_offset,
				header->tre5_size / header->tre5_rec_size,
				header->tre5_rec_size);
	}
	if (header->tre6_size) {
		printf("=== POINT OVERVIEWS ===\n");
		dump_poverview(tre->base + header->tre6_offset,
				header->tre6_size / header->tre6_rec_size,
				header->tre6_rec_size);
	}

	if (header->comm.hlen >= 0x86 && header->tre7_size) {
		printf("=== TRE7 ===\n");
		dump_tre7(tre->base + header->tre7_offset, header->tre7_size,
				header->tre7_rec_size, maplevels, tre->map->rgn);
	}

	free(maplevels);
}
