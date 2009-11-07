#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "gimginfo.h"

void dump_mps (struct subfile_struct *mps)
{
	int line;

	for (line = 0; line < mps->size;) {
		int type = *(mps->base + line + 0);
		int size = *(uint16_t *)(mps->base + line + 1);
		int ptr = line + 3;
		switch(type) {
		case 0x46:
			printf("0x46: pid=%d fid=%d",
					*(uint16_t *)(mps->base + ptr + 0),
					*(uint16_t *)(mps->base + ptr + 2));
			ptr += 4;
			printf(" str1=\"%s\"\n", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			break;
		case 0x4c:
			printf("0x4c: pid=%d fid=%d ?=0x%x",
					*(uint16_t *)(mps->base + ptr + 0),
					*(uint16_t *)(mps->base + ptr + 2),
					*(uint16_t *)(mps->base + ptr + 6));
			ptr += 8;
			printf(" area=\"%s\"", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			printf(" name=\"%s\"", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			printf(" str3=\"%s\"", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			printf(" id=%x ?=%x\n", *(uint32_t *)(mps->base + ptr), *(uint32_t *)(mps->base + ptr + 4));
			ptr += 8;
			break;
		case 0x55:
			printf("0x55: str=\"%s\"\n", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			break;
		case 0x56:
			printf("0x56: str=\"%s\"", mps->base + ptr);
			ptr += strlen((char *)mps->base + ptr) + 1;
			printf(" ?=0x%x\n", *(mps->base + ptr));
			ptr += 1;
			break;
		default:
			printf("0x%02x: %s\n", type, dump_unknown_bytes(mps->base + ptr, size));
		}
		line += size + 3;
	}
}
