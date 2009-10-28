#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct common_header
{
	uint16_t length;              ///< 0x00000000 .. 0x00000001
	char     type[10];            ///< 0x00000002 .. 0x0000000B
	uint8_t  byte0x0000000C;
	uint8_t  flag;                ///< 0x0000000D
	uint8_t  byte0x0000000E_0x00000014[7];
} __attribute__((packed));

struct tre_header
{
	uint8_t  northbound[3];       ///< 0x00000015 .. 0x00000017
	uint8_t  eastbound[3];        ///< 0x00000018 .. 0x0000001A
	uint8_t  southbound[3];       ///< 0x0000001B .. 0x0000001D
	uint8_t  westbound[3];        ///< 0x0000001E .. 0x00000020
	uint32_t tre1_offset;         ///< 0x00000021 .. 0x00000024
	uint32_t tre1_size;           ///< 0x00000025 .. 0x00000028
	uint32_t tre2_offset;         ///< 0x00000029 .. 0x0000002C
	uint32_t tre2_size;           ///< 0x0000002D .. 0x00000030
	uint32_t tre3_offset;         ///< 0x00000031 .. 0x00000034
	uint32_t tre3_size;           ///< 0x00000035 .. 0x00000038
	uint16_t tre3_rec_size;       ///< 0x00000039 .. 0x0000003A
	uint8_t  byte0x0000003B_0x0000003E[4];
	uint8_t  POI_flags;           ///< 0x0000003F
	uint8_t  byte0x00000040_0x00000049[10];
	uint32_t tre4_offset;         ///< 0x0000004A .. 0x0000004D
	uint32_t tre4_size;           ///< 0x0000004E .. 0x00000051
	uint16_t tre4_rec_size;       ///< 0x00000052 .. 0x00000053
	uint8_t  byte0x00000054_0x00000057[4];
	uint32_t tre5_offset;         ///< 0x00000058 .. 0x0000005B
	uint32_t tre5_size;           ///< 0x0000005C .. 0x0000005F
	uint16_t tre5_rec_size;       ///< 0x00000060 .. 0x00000061
	uint8_t  byte0x00000062_0x00000065[4];
	uint32_t tre6_offset;         ///< 0x00000066 .. 0x00000069
	uint32_t tre6_size;           ///< 0x0000006A .. 0x0000006D
	uint16_t tre6_rec_size;       ///< 0x0000006E .. 0x0000006F
	uint8_t  byte0x00000070_0x00000073[4];
	/*-----------------------------------------------------*/
	uint8_t  byte0x00000074_0x0000007B[8];
	uint32_t tre7_offset;         ///< 0x0000007C .. 0x0000007F
	uint32_t tre7_size;           ///< 0x00000080 .. 0x00000083
	uint16_t tre7_rec_size;       ///< 0x00000084 .. 0x00000085
	uint8_t  byte0x00000086_0x00000089[4];
	uint32_t tre8_offset;         ///< 0x0000008A .. 0x0000008D
	uint32_t tre8_size;           ///< 0x0000008E .. 0x00000091
	uint8_t  byte0x00000092_0x00000099[8];
	/*-----------------------------------------------------*/
	uint8_t  key[20];             ///< 0x0000009A .. 0x000000AD
	uint32_t tre9_offset;         ///< 0x000000AE .. 0x000000B1
	uint32_t tre9_size;           ///< 0x000000B2 .. 0x000000B5
	uint16_t tre9_rec_size;       ///< 0x000000B6 .. 0x000000B7
} __attribute__((packed));

struct subdiv {
	uint8_t  rgn_offset[3];
	uint8_t  elements;
	uint8_t  center_lng[3];
	uint8_t  center_lat[3];
	uint16_t width_trm;
	uint16_t height;
	uint16_t next;
} __attribute__((packed));

static int to_uint24 (unsigned char *arr)
{
	return (arr[2] << 16) + (arr[1] << 8) + arr[0];
}

static int to_sint24 (unsigned char *arr)
{
	int n = (arr[2] << 16) + (arr[1] << 8) + arr[0];
	return (n < 0x00800000) ? n : (n | 0xff000000);
}

void dump_comm (struct common_header *comh)
{
	printf("h len:  %d(0x%x)\n", comh->length, comh->length);
	printf("flag0d: 0x%x\n", comh->flag);
}

void dump_subdiv_rec (char *ptr, int index, int level, int *pnum_nleave)
{
	struct subdiv *div;
	if (level == 0) {
		if (*pnum_nleave == 0)
			*pnum_nleave = index - 1;
		div = ptr + (index - 1) * (sizeof(struct subdiv) - 2) + *pnum_nleave * 2;
		while (1) {
			printf("0 off=%06x, ele=%02x, lng=%8d(%3.6f), lat=%8d(%3.6f), width=%5d, height=%5d\n",
					to_uint24(div->rgn_offset), div->elements,
					to_sint24(div->center_lng), to_sint24(div->center_lng) * (360.0 / 0x01000000),
					to_sint24(div->center_lat), to_sint24(div->center_lat) * (360.0 / 0x01000000),
					div->width_trm & 0x7fff, div->height);
			if (div->width_trm & 0x8000)
				break;
			index ++;
			div = ptr + (index - 1) * (sizeof(struct subdiv) - 2) + *pnum_nleave * 2;
		}
	} else {
		div = ptr + sizeof(struct subdiv) * (index - 1);
		while (1) {
			printf("%d off=%06x, ele=%02x, lng=%8d(%3.6f), lat=%8d(%3.6f), width=%5d, height=%5d\n",
					level,
					to_uint24(div->rgn_offset), div->elements,
					to_sint24(div->center_lng), to_sint24(div->center_lng) * (360.0 / 0x01000000),
					to_sint24(div->center_lat), to_sint24(div->center_lat) * (360.0 / 0x01000000),
					div->width_trm & 0x7fff, div->height);
			dump_subdiv_rec(ptr, div->next, level - 1, pnum_nleave);
			if (div->width_trm & 0x8000)
				break;
			index ++;
			div = ptr + sizeof(struct subdiv) * (index - 1);
		}
	}
}

void dump_subdiv (char *ptr, int levels)
{
	int num_nleave = 0;
	dump_subdiv_rec(ptr, 1, levels, &num_nleave);
}

void dump_tre (char *tre, int size)
{
	struct tre_header *treh = (struct tre_header *)(tre + sizeof(struct common_header));

	dump_comm((struct common_header *)tre);

	printf("Bound:  %d(%f) %d(%f) %d(%f) %d(%f)\n",
			to_sint24(treh->northbound), to_sint24(treh->northbound) * (360.0 / 0x01000000),
			to_sint24(treh->eastbound),  to_sint24(treh->eastbound) * (360.0 / 0x01000000),
			to_sint24(treh->southbound), to_sint24(treh->southbound) * (360.0 / 0x01000000),
			to_sint24(treh->westbound),  to_sint24(treh->westbound) * (360.0 / 0x01000000));
	printf("tre1-4: %x+%x %x+%x %x+%x %x+%x\n",
			treh->tre1_offset, treh->tre1_size,
			treh->tre2_offset, treh->tre2_size,
			treh->tre3_offset, treh->tre3_size,
			treh->tre4_offset, treh->tre4_size);
	printf("tre5-9: %x+%x %x+%x %x+%x %x+%x %x+%x\n",
			treh->tre5_offset, treh->tre5_size,
			treh->tre6_offset, treh->tre6_size,
			treh->tre7_offset, treh->tre7_size,
			treh->tre8_offset, treh->tre8_size,
			treh->tre9_offset, treh->tre9_size);

	dump_subdiv(tre + treh->tre2_offset, treh->tre1_size / 4 - 1);
}

int main (int argc, char **argv)
{
	FILE *fp;
	void *data;
	int size;

	if (argc != 2) {
		printf("usage: %s file.tre\n", argv[0]);
		return 1;
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("can't open file\n");
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = (void *)malloc(size);
	if (data == NULL) {
		printf("out of memory\n");
		return 1;
	}

	fread(data, size, 1, fp);
	dump_tre(data, size);

	return 0;
}
