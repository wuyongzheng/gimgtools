#ifndef GARMIN_STRUCT_H
#define GARMIN_STRUCT_H

#include <stdint.h>

struct img_header_struct {
	uint8_t xor;
	uint8_t unknown001[9];
	uint8_t umonth;
	uint8_t uyear;
	uint8_t unknown00c[3];
	uint8_t checksum;
	char signature[7];
	uint8_t unknown017;
	uint16_t sectors;
	uint16_t heads;
	uint16_t cylinders;
	uint16_t unknown01e;
	uint8_t unknown020[25];
	uint16_t cyear;
	uint8_t cmonth;
	uint8_t cdate;
	uint8_t chour;
	uint8_t cminute;
	uint8_t csecond;
	uint8_t fatoffset; // in blocks of 512bytes from the start
	char identifier[7];
	uint8_t unknown048;
	char desc1[20];
	uint16_t heads1;
	uint16_t sectors1;
	uint8_t blockexp1;
	uint8_t blockexp2;
	uint16_t unknown063;
	char desc2[31];
	uint8_t unknown084[904];
	uint32_t dataoffset;
	uint8_t unknown410[16];
	uint16_t blocks[240];
} __attribute__((packed));

struct fat_header_struct {
	uint8_t flag;
	char name[8];
	char type[3];
	uint32_t size;
	uint16_t part;
	uint8_t unknown012[14];
	uint16_t blocks[240];
} __attribute__((packed));

struct subfile_header_struct {
	uint16_t hlen;
	char type[10];
	uint8_t unknown00c;
	uint8_t unknown00d;
	uint16_t year;
	uint8_t month;
	uint8_t date;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} __attribute__((packed));

struct gmp_header_struct
{
	struct subfile_header_struct comm;
	uint32_t unknown1;
	uint32_t tre_offset;
	uint32_t rgn_offset;
	uint32_t lbl_offset;
	uint32_t net_offset;
	uint32_t nod_offset;
	uint32_t unknown2;
}  __attribute__((packed));

#endif
