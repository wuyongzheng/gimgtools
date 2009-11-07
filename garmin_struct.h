#ifndef GARMIN_STRUCT_H
#define GARMIN_STRUCT_H

#include <stdint.h>

struct garmin_img {
	uint8_t  xor_byte;
	uint8_t  unknown_001[9];
	uint8_t  umonth;
	uint8_t  uyear;
	uint8_t  unknown_00c[3];
	uint8_t  checksum;
	char     signature[7];
	uint8_t  unknown_017;
	uint16_t sectors;
	uint16_t heads;
	uint16_t cylinders;
	uint16_t unknown_01e;
	uint8_t  unknown_020[25];
	uint16_t cyear;
	uint8_t  cmonth;
	uint8_t  cdate;
	uint8_t  chour;
	uint8_t  cminute;
	uint8_t  csecond;
	uint8_t  fat_offset; // in blocks of 512bytes from the start
	char     identifier[7];
	uint8_t  unknown_048;
	char     desc1[20];
	uint16_t heads1;
	uint16_t sectors1;
	uint8_t  blockexp1;
	uint8_t  blockexp2;
	uint16_t unknown_063;
	char     desc2[30];
	uint8_t  unknown_083;
	uint8_t  unknown_084[904];
	uint32_t data_offset;
	uint8_t  unknown_410[16];
	uint16_t blocks[240];
} __attribute__((packed));

struct garmin_fat {
	uint8_t  flag;
	char     name[8];
	char     type[3];
	uint32_t size;
	uint16_t part;
	uint8_t  unknown_012[14];
	uint16_t blocks[240];
} __attribute__((packed));

struct garmin_subfile {
	uint16_t hlen;
	char     type[10];
	uint8_t  unknown_00c;
	uint8_t  locked;
	uint16_t year;
	uint8_t  month;
	uint8_t  date;
	uint8_t  hour;
	uint8_t  minute;
	uint8_t  second;
} __attribute__((packed));

struct garmin_tre {
	struct garmin_subfile comm;
	uint8_t  northbound[3];         ///< 0x00000015 .. 0x00000017
	uint8_t  eastbound[3];          ///< 0x00000018 .. 0x0000001A
	uint8_t  southbound[3];         ///< 0x0000001B .. 0x0000001D
	uint8_t  westbound[3];          ///< 0x0000001E .. 0x00000020
	uint32_t tre1_offset;        ///< 0x00000021 .. 0x00000024
	uint32_t tre1_size;          ///< 0x00000025 .. 0x00000028
	uint32_t tre2_offset;        ///< 0x00000029 .. 0x0000002C
	uint32_t tre2_size;          ///< 0x0000002D .. 0x00000030
	uint32_t tre3_offset;        ///< 0x00000031 .. 0x00000034
	uint32_t tre3_size;          ///< 0x00000035 .. 0x00000038
	uint16_t tre3_rec_size;      ///< 0x00000039 .. 0x0000003A
	uint8_t  unknown_03b[4];
	uint8_t  POI_flags;           ///< 0x0000003F
	uint8_t  drawprio;            // map draw priority
	uint8_t  unknown_041[9];
	uint32_t tre4_offset;        ///< 0x0000004A .. 0x0000004D
	uint32_t tre4_size;          ///< 0x0000004E .. 0x00000051
	uint16_t tre4_rec_size;      ///< 0x00000052 .. 0x00000053
	uint8_t  unknown_054[4];
	uint32_t tre5_offset;        ///< 0x00000058 .. 0x0000005B
	uint32_t tre5_size;          ///< 0x0000005C .. 0x0000005F
	uint16_t tre5_rec_size;      ///< 0x00000060 .. 0x00000061
	uint8_t  unknown_062[4];
	uint32_t tre6_offset;        ///< 0x00000066 .. 0x00000069
	uint32_t tre6_size;          ///< 0x0000006A .. 0x0000006D
	uint16_t tre6_rec_size;      ///< 0x0000006E .. 0x0000006F
	uint8_t  unknown_070[4];
	uint32_t mapID;
	uint8_t  unknown_078[4];
	uint32_t tre7_offset;        ///< 0x0000007C .. 0x0000007F
	uint32_t tre7_size;          ///< 0x00000080 .. 0x00000083
	uint16_t tre7_rec_size;      ///< 0x00000084 .. 0x00000085
	uint8_t  unknown_086[4];
	uint32_t tre8_offset;        ///< 0x0000008A .. 0x0000008D
	uint32_t tre8_size;          ///< 0x0000008E .. 0x00000091
	uint16_t tre8_rec_size;
	uint8_t  unknown_092[6];
	uint8_t  key[20];            ///< 0x0000009A .. 0x000000AD
//	uint8_t  gap1[4];
	uint32_t tre9_offset;        ///< 0x000000AE .. 0x000000B1
	uint32_t tre9_size;          ///< 0x000000B2 .. 0x000000B5
	uint16_t tre9_rec_size;      ///< 0x000000B6 .. 0x000000B7
	uint8_t  unknown_0b8[4];
//	uint8_t  gap2[4];
	uint32_t tre10_offset;       ///< 0x000000AE .. 0x000000B1
	uint32_t tre10_size;         ///< 0x000000B2 .. 0x000000B5
	uint16_t tre10_rec_size;     ///< 0x000000B6 .. 0x000000B7
} __attribute__((packed));

struct garmin_tre_map_level {
	uint8_t  level       :4;
	uint8_t  bit456      :3;
	uint8_t  inherited   :1;
	uint8_t  bits;
	uint16_t nsubdiv;
} __attribute__((packed));

struct garmin_tre_subdiv {
	uint8_t  rgn_offset[3];
	uint8_t  elements;
	uint8_t  center_lng[3];
	uint8_t  center_lat[3];
	uint16_t width       :15;
	uint16_t terminate   :1;
	uint16_t height      :15;
	uint16_t unknownbit  :1;
	uint16_t next;
} __attribute__((packed));

struct garmin_rgn {
	struct garmin_subfile comm;
	uint32_t data_off;
	uint32_t data_len;
} __attribute__((packed));

struct garmin_lbl {
	struct garmin_subfile comm;
	uint32_t lbl1_offset;        ///< 0x00000015 .. 0x00000018
	uint32_t lbl1_length;        ///< 0x00000019 .. 0x0000001C
	uint8_t  addr_shift;         ///< 0x0000001D
	uint8_t  coding;             ///< 0x0000001E
	uint32_t lbl2_offset;        ///< 0x0000001F .. 0x00000022
	uint32_t lbl2_length;        ///< 0x00000023 .. 0x00000026
	uint16_t lbl2_rec_size;      ///< 0x00000027 .. 0x00000028
	uint8_t  byte0x00000029_0x0000002C[4];
	uint32_t lbl3_offset;        ///< 0x0000002D .. 0x00000030
	uint32_t lbl3_length;        ///< 0x00000031 .. 0x00000034
	uint16_t lbl3_rec_size;      ///< 0x00000035 .. 0x00000036
	uint8_t  byte0x00000037_0x0000003A[4];
	uint32_t lbl4_offset;        ///< 0x0000003B .. 0x0000003E
	uint32_t lbl4_length;        ///< 0x0000003F .. 0x00000042
	uint16_t lbl4_rec_size;      ///< 0x00000043 .. 0x00000044
	uint8_t  byte0x00000045_0x00000048[4];
	uint32_t lbl5_offset;        ///< 0x00000049 .. 0x0000004C
	uint32_t lbl5_length;        ///< 0x0000004D .. 0x00000050
	uint16_t lbl5_rec_size;      ///< 0x00000051 .. 0x00000052
	uint8_t  byte0x00000053_0x00000056[4];
	uint32_t lbl6_offset;        ///< 0x00000057 .. 0x0000005A
	uint32_t lbl6_length;        ///< 0x0000005B .. 0x0000005E
	uint8_t  lbl6_addr_shift;    ///< 0x0000005F
	uint8_t  lbl6_glob_mask;     ///< 0x00000060
	uint8_t  byte0x00000061_0x00000063[3];
	uint32_t lbl7_offset;        ///< 0x00000064 .. 0x00000067
	uint32_t lbl7_length;        ///< 0x00000068 .. 0x0000006B
	uint16_t lbl7_rec_size;      ///< 0x0000006C .. 0x0000006D
	uint8_t  byte0x0000006E_0x00000071[4];
	uint32_t lbl8_offset;        ///< 0x00000072 .. 0x00000075
	uint32_t lbl8_length;        ///< 0x00000076 .. 0x00000079
	uint16_t lbl8_rec_size;      ///< 0x0000007A .. 0x0000007B
	uint8_t  byte0x0000007C_0x0000007F[4];
	uint32_t lbl9_offset;        ///< 0x00000080 .. 0x00000083
	uint32_t lbl9_length;        ///< 0x00000084 .. 0x00000087
	uint16_t lbl9_rec_size;      ///< 0x00000088 .. 0x00000089
	uint8_t  byte0x0000008A_0x0000008D[4];
	uint32_t lbl10_offset;       ///< 0x0000008E .. 0x00000091
	uint32_t lbl10_length;       ///< 0x00000092 .. 0x00000095
	uint16_t lbl10_rec_size;     ///< 0x00000096 .. 0x00000097
	uint8_t  byte0x00000098_0x0000009B[4];
	uint32_t lbl11_offset;       ///< 0x0000009C .. 0x0000009F
	uint32_t lbl11_length;       ///< 0x000000A0 .. 0x000000A3
	uint16_t lbl11_rec_size;     ///< 0x000000A4 .. 0x000000A5
	uint8_t  byte0x000000A6_0x000000AB[4];
	uint16_t codepage;           ///< 0x000000AA .. 0x000000AB  optional check length
} __attribute__((packed));

struct garmin_net {
	struct garmin_subfile comm;
	// NET1 Road definitions
	uint32_t net1_offset;        ///< 0x00000015 .. 0x00000018
	uint32_t net1_length;        ///< 0x00000019 .. 0x0000001C
	uint8_t  net1_addr_shift;    ///< 0x0000001D
	// Segmented roads
	uint32_t net2_offset;        ///< 0x0000001E .. 0x00000021
	uint32_t net2_length;        ///< 0x00000022 .. 0x00000025
	uint8_t  net2_addr_shift;     ///< 0x00000026
	// Sorted Roads
	uint32_t net3_offset;        ///< 0x00000027 .. 0x0000002A
	uint32_t net3_length;        ///< 0x0000002B .. 0x0000002E
} __attribute__((packed));

struct garmin_gmp {
	struct garmin_subfile comm;
	uint32_t unknown_015;
	uint32_t tre_offset;
	uint32_t rgn_offset;
	uint32_t lbl_offset;
	uint32_t net_offset;
	uint32_t nod_offset;
	uint32_t unknown_20d;
} __attribute__((packed));

/* http://ati.land.cz/ */
struct garmin_typ
{
	struct garmin_subfile comm;
	uint16_t codepage;    /* offset = 0x15 */
	uint32_t point_datoff;
	uint32_t point_datsize;
	uint32_t line_datoff;
	uint32_t line_datsize;
	uint32_t polygon_datoff;
	uint32_t polygon_datsize;
	uint16_t fid;
	uint16_t pid;
	uint32_t point_arroff;
	uint16_t point_arrmod;
	uint32_t point_arrsize;
	uint32_t line_arroff;
	uint16_t line_arrmod;
	uint32_t line_arrsize;
	uint32_t polygon_arroff;
	uint16_t polygon_arrmod;
	uint32_t polygon_arrsize;
	uint32_t draworder_arroff;
	uint16_t draworder_arrmod;
	uint32_t draworder_arrsize;
	/* possible size limit here*/
	uint32_t nt1_arroff; /* offset = 0x5B */
	uint16_t nt1_arrmod;
	uint32_t nt1_arrsize;
	uint8_t  nt1_flag;
	uint32_t nt1_datoff;
	uint32_t nt1_datsize;
	/* possible size limit here*/
	uint32_t blok0_x;    /* offset = 0x6E */
	uint32_t blok0_off;
	uint32_t blok0_size;
	uint32_t blok0_y;
	uint32_t blok1_x;
	uint32_t blok1_off;
	uint32_t blok1_size;
	uint32_t blok1_y;
	uint32_t blok2_x;
	uint32_t blok2_off;
	uint32_t blok2_size;
	uint16_t unknown_09a;
	/* offset = 0x9C */
} __attribute__((packed));

#endif
