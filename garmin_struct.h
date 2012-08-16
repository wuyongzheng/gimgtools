#ifndef GARMIN_STRUCT_H
#define GARMIN_STRUCT_H

#ifdef GT_POSIX
# define PACK_STRUCT __attribute__((packed))
#else
# define PACK_STRUCT
# pragma pack(push, 1)
#endif

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
} PACK_STRUCT ;

struct garmin_fat {
	uint8_t  flag;            // 0x00
	char     name[8];         // 0x01
	char     type[3];         // 0x09
	uint32_t size;            // 0x0c
	uint16_t part;            // 0x10
	uint8_t  unknown_012[14]; // 0x12
	uint16_t blocks[240];     // 0x20
} PACK_STRUCT ;

struct garmin_subfile {
	uint16_t hlen;        // 0x00
	char     type[10];    // 0x02
	uint8_t  unknown_00c; // 0x0c
	uint8_t  locked;      // 0x0d
	uint16_t year;        // 0x0e
	uint8_t  month;       // 0x10
	uint8_t  date;        // 0x11
	uint8_t  hour;        // 0x12
	uint8_t  minute;      // 0x13
	uint8_t  second;      // 0x14
} PACK_STRUCT ;

struct garmin_tre {
	struct garmin_subfile comm;
	uint8_t  northbound[3];      ///< 0x00000015 .. 0x00000017
	uint8_t  eastbound[3];       ///< 0x00000018 .. 0x0000001A
	uint8_t  southbound[3];      ///< 0x0000001B .. 0x0000001D
	uint8_t  westbound[3];       ///< 0x0000001E .. 0x00000020
	uint32_t tre1_offset;        ///< 0x00000021 .. 0x00000024
	uint32_t tre1_size;          ///< 0x00000025 .. 0x00000028
	uint32_t tre2_offset;        ///< 0x00000029 .. 0x0000002C
	uint32_t tre2_size;          ///< 0x0000002D .. 0x00000030
	uint32_t tre3_offset;        ///< 0x00000031 .. 0x00000034
	uint32_t tre3_size;          ///< 0x00000035 .. 0x00000038
	uint16_t tre3_rec_size;      ///< 0x00000039 .. 0x0000003A
	uint8_t  unknown_03b[4];
	uint8_t  POI_flags;          ///< 0x0000003F
	uint8_t  drawprio;           // map draw priority
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
	uint8_t  unknown_070[4];     ///< 0x00000070 .. 0x00000073 break
	uint32_t mapID;              ///< 0x00000074 .. 0x00000077 break
	uint8_t  unknown_078[4];
	uint32_t tre7_offset;        ///< 0x0000007C .. 0x0000007F
	uint32_t tre7_size;          ///< 0x00000080 .. 0x00000083
	uint16_t tre7_rec_size;      ///< 0x00000084 .. 0x00000085
	uint8_t  unknown_086[4];
	uint32_t tre8_offset;        ///< 0x0000008A .. 0x0000008D
	uint32_t tre8_size;          ///< 0x0000008E .. 0x00000091
	uint16_t tre8_rec_size;
	uint8_t  unknown_094[6];
	uint8_t  key[20];            ///< 0x0000009A .. 0x000000AD
	uint32_t tre9_offset;        ///< 0x000000AE .. 0x000000B1
	uint32_t tre9_size;          ///< 0x000000B2 .. 0x000000B5
	uint16_t tre9_rec_size;      ///< 0x000000B6 .. 0x000000B7
	uint8_t  unknown_0b8[4];     ///< 0x000000B8 .. 0x000000BB break
	uint32_t tre10_offset;       ///< 0x000000BC .. 0x000000BF
	uint32_t tre10_size;         ///< 0x000000C0 .. 0x000000C3
	uint16_t tre10_rec_size;     ///< 0x000000C4 .. 0x000000C5 break
	uint8_t  unknown_0c6[4];     ///< 0x000000C6 .. 0x000000C9 break
	uint8_t  unknown_0ca[4];     ///< 0x000000CA .. 0x000000CD break
	uint8_t  unknown_0ce;        ///< 0x000000CE .. 0x000000CE break
} PACK_STRUCT ;

struct garmin_tre_map_level {
	uint8_t  level       :4;
	uint8_t  bit456      :3;
	uint8_t  inherited   :1;
	uint8_t  bits;
	uint16_t nsubdiv;
} PACK_STRUCT ;

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
} PACK_STRUCT ;

struct garmin_rgn {
	struct garmin_subfile comm; // 0x000
	uint32_t rgn1_offset;       // 0x015
	uint32_t rgn1_length;       // 0x019
	/* break */
	uint32_t rgn2_offset;       // 0x01d
	uint32_t rgn2_length;       // 0x021
	uint8_t  unknown_025[20];   // 0x025
	uint32_t rgn3_offset;       // 0x039
	uint32_t rgn3_length;       // 0x03d
	uint8_t  unknown_041[20];   // 0x041
	uint32_t rgn4_offset;       // 0x055
	uint32_t rgn4_length;       // 0x059
	uint8_t  unknown_05d[20];   // 0x05d
	uint32_t rgn5_offset;       // 0x071
	uint32_t rgn5_length;       // 0x075
	uint32_t unknown_079;       // 0x079
} PACK_STRUCT ;

struct garmin_lbl {
	struct garmin_subfile comm;
	uint32_t lbl1_offset;        ///< 0x00000015 .. 0x00000018
	uint32_t lbl1_length;        ///< 0x00000019 .. 0x0000001C
	uint8_t  addr_shift;         ///< 0x0000001D
	uint8_t  coding;             ///< 0x0000001E
	uint32_t lbl2_offset;        ///< 0x0000001F .. 0x00000022
	uint32_t lbl2_length;        ///< 0x00000023 .. 0x00000026
	uint16_t lbl2_recsize;       ///< 0x00000027 .. 0x00000028
	uint32_t lbl2_u;
	uint32_t lbl3_offset;        ///< 0x0000002D .. 0x00000030
	uint32_t lbl3_length;        ///< 0x00000031 .. 0x00000034
	uint16_t lbl3_recsize;       ///< 0x00000035 .. 0x00000036
	uint32_t lbl3_u;
	uint32_t lbl4_offset;        ///< 0x0000003B .. 0x0000003E
	uint32_t lbl4_length;        ///< 0x0000003F .. 0x00000042
	uint16_t lbl4_recsize;       ///< 0x00000043 .. 0x00000044
	uint32_t lbl4_u;
	uint32_t lbl5_offset;        ///< 0x00000049 .. 0x0000004C
	uint32_t lbl5_length;        ///< 0x0000004D .. 0x00000050
	uint16_t lbl5_recsize;       ///< 0x00000051 .. 0x00000052
	uint32_t lbl5_u;
	uint32_t lbl6_offset;        ///< 0x00000057 .. 0x0000005A
	uint32_t lbl6_length;        ///< 0x0000005B .. 0x0000005E
	uint8_t  lbl6_addr_shift;    ///< 0x0000005F
	uint8_t  lbl6_glob_mask;     ///< 0x00000060
	uint8_t  lbl6_u[3];
	uint32_t lbl7_offset;        ///< 0x00000064 .. 0x00000067
	uint32_t lbl7_length;        ///< 0x00000068 .. 0x0000006B
	uint16_t lbl7_recsize;       ///< 0x0000006C .. 0x0000006D
	uint32_t lbl7_u;
	uint32_t lbl8_offset;        ///< 0x00000072 .. 0x00000075
	uint32_t lbl8_length;        ///< 0x00000076 .. 0x00000079
	uint16_t lbl8_recsize;       ///< 0x0000007A .. 0x0000007B
	uint32_t lbl8_u;
	uint32_t lbl9_offset;        ///< 0x00000080 .. 0x00000083
	uint32_t lbl9_length;        ///< 0x00000084 .. 0x00000087
	uint16_t lbl9_recsize;       ///< 0x00000088 .. 0x00000089
	uint32_t lbl9_u;
	uint32_t lbl10_offset;       ///< 0x0000008E .. 0x00000091
	uint32_t lbl10_length;       ///< 0x00000092 .. 0x00000095
	uint16_t lbl10_recsize;      ///< 0x00000096 .. 0x00000097
	uint32_t lbl10_u;
	uint32_t lbl11_offset;       ///< 0x0000009C .. 0x0000009F
	uint32_t lbl11_length;       ///< 0x000000A0 .. 0x000000A3
	uint16_t lbl11_recsize;      ///< 0x000000A4 .. 0x000000A5
	uint32_t lbl11_u;            ///< 0x000000A6 .. 0x000000A9 break
	uint16_t codepage;           // 0x0aa
	uint16_t codepage2;          // 0x0ac
	uint16_t codepage3;          // 0x0ae
	uint32_t lbl12_offset;       // 0x0b0
	uint32_t lbl12_length;       // 0x0b4
	uint32_t lbl13_offset;       // 0x0b8
	uint32_t lbl13_length;       // 0x0bc
	uint16_t lbl13_recsize;      // 0x0c0
	uint16_t lbl13_u;            // 0x0c2
	uint32_t lbl14_offset;       // 0x0c4
	uint32_t lbl14_length;       // 0x0c8
	uint16_t lbl14_recsize;      // 0x0cc
	uint16_t lbl14_u;            // 0x0ce
	uint32_t lbl15_offset;       // 0x0d0
	uint32_t lbl15_length;       // 0x0d4
	uint16_t lbl15_recsize;      // 0x0d8
	uint32_t lbl15_u;            // 0x0da
	uint32_t lbl16_offset;       // 0x0de
	uint32_t lbl16_length;       // 0x0e2
	uint16_t lbl16_recsize;      // 0x0e6
	uint32_t lbl16_u;            // 0x0e8
	uint32_t lbl17_offset;       // 0x0ec
	uint32_t lbl17_length;       // 0x0f0
	uint16_t lbl17_recsize;      // 0x0f4
	uint32_t lbl17_u;            // 0x0f6
	uint32_t lbl18_offset;       // 0x0fa
	uint32_t lbl18_length;       // 0x0fe
	uint16_t lbl18_recsize;      // 0x102
	uint32_t lbl18_u;            // 0x104
	uint32_t lbl19_offset;       // 0x108
	uint32_t lbl19_length;       // 0x10c
	uint16_t lbl19_recsize;      // 0x110
	uint32_t lbl19_u;            // 0x112
	uint32_t lbl20_offset;       // 0x116
	uint32_t lbl20_length;       // 0x11a
	uint16_t lbl20_recsize;      // 0x11e
	uint32_t lbl20_u;            // 0x120
	uint32_t lbl21_offset;       // 0x124
	uint32_t lbl21_length;       // 0x128
	uint16_t lbl21_recsize;      // 0x12c
	uint32_t lbl21_u;            // 0x12e
	uint32_t lbl22_offset;       // 0x132
	uint32_t lbl22_length;       // 0x136
	uint16_t lbl22_recsize;      // 0x13a
	uint32_t lbl22_u;            // 0x13c
	uint32_t lbl23_offset;       // 0x140
	uint32_t lbl23_length;       // 0x144
	uint16_t lbl23_recsize;      // 0x148
	uint32_t lbl23_u;            // 0x14a
	uint32_t lbl24_offset;       // 0x14e
	uint32_t lbl24_length;       // 0x152
	uint16_t lbl24_recsize;      // 0x156
	uint16_t lbl24_u;            // 0x158
	uint32_t lbl25_offset;       // 0x15a
	uint32_t lbl25_length;       // 0x15e
	uint16_t lbl25_recsize;      // 0x162
	uint32_t lbl25_u;            // 0x164
	uint32_t lbl26_offset;       // 0x168
	uint32_t lbl26_length;       // 0x16c
	uint16_t lbl26_recsize;      // 0x170
	uint32_t lbl26_u;            // 0x172
	uint32_t lbl27_offset;       // 0x176
	uint32_t lbl27_length;       // 0x17a
	uint16_t lbl27_recsize;      // 0x17e
	uint32_t lbl27_u;            // 0x180
	uint32_t lbl28_offset;       // 0x184
	uint32_t lbl28_length;       // 0x188
	uint16_t lbl28_recsize;      // 0x18c
	uint32_t lbl28_u;            // 0x18e
	uint32_t lbl29_offset;       // 0x192
	uint32_t lbl29_length;       // 0x196
	uint32_t lbl30_offset;       // 0x19a
	uint32_t lbl30_length;       // 0x19e
	uint16_t lbl30_recsize;      // 0x1a2
	uint16_t lbl30_u;            // 0x1a4
	uint32_t lbl31_offset;       // 0x1a6
	uint32_t lbl31_length;       // 0x1aa
	uint16_t lbl31_recsize;      // 0x1ae
	uint16_t lbl31_u;            // 0x1b0
	uint32_t lbl32_offset;       // 0x1b2
	uint32_t lbl32_length;       // 0x1b6
	uint16_t lbl32_recsize;      // 0x1ba
	uint16_t lbl32_u;            // 0x1bc
	uint32_t lbl33_offset;       // 0x1be
	uint32_t lbl33_length;       // 0x1c2
	uint16_t lbl33_recsize;      // 0x1c6
	uint16_t lbl33_u;            // 0x1c8
	uint32_t lbl34_offset;       // 0x1ca
	uint32_t lbl34_length;       // 0x1ce
	uint16_t lbl34_recsize;      // 0x1d2
	uint32_t lbl34_u;            // 0x1d4
	uint32_t lbl35_offset;       // 0x1d8
	uint32_t lbl35_length;       // 0x1dc
	uint16_t lbl35_recsize;      // 0x1e0
	uint32_t lbl35_u;            // 0x1e2
	uint32_t lbl36_offset;       // 0x1e6
	uint32_t lbl36_length;       // 0x1ea
	uint16_t lbl36_recsize;      // 0x1ee
	uint16_t lbl36_u;            // 0x1f0
} PACK_STRUCT ;

struct garmin_net {
	struct garmin_subfile comm;
	// NET1 Road definitions
	uint32_t net1_offset;        ///< 0x00000015 .. 0x00000018
	uint32_t net1_length;        ///< 0x00000019 .. 0x0000001C
	uint8_t  net1_shift;         ///< 0x0000001D
	// Segmented roads
	uint32_t net2_offset;        ///< 0x0000001E .. 0x00000021
	uint32_t net2_length;        ///< 0x00000022 .. 0x00000025
	uint8_t  net2_shift;         ///< 0x00000026
	// Sorted Roads
	uint32_t net3_offset;        ///< 0x00000027 .. 0x0000002A
	uint32_t net3_length;        ///< 0x0000002B .. 0x0000002E
	uint16_t net3_recsize;       // 0x2f
	uint32_t unknown_031;        // 0x31
	uint16_t unknown_035;        // 0x35
	/* break */
	uint32_t unknown_037;        // 0x37
	/* break */
	uint32_t unknown_03b;        // 0x3b
	uint32_t unknown_03f;        // 0x3f
	uint32_t net4_offset;        // 0x43
	uint32_t net4_length;        // 0x47
	uint8_t  net4_u;             // 0x4b
	uint32_t net5_offset;        // 0x4c
	uint32_t net5_length;        // 0x50
	uint16_t net5_recsize;       // 0x54
	uint32_t net6_offset;        // 0x56
	uint32_t net6_length;        // 0x5a
	uint16_t net6_recsize;       // 0x5e
	uint32_t unknown_060;        // 0x60
} PACK_STRUCT ;

struct garmin_nod
{
	struct garmin_subfile comm;
	uint32_t nod1_offset;        // 0x15
	uint32_t nod1_length;        // 0x19
	uint8_t  nod_bits[4];        // 0x1d
	uint8_t  align;              // 0x21
	uint8_t  unknown_022;        // 0x22
	uint16_t roadptrsize;        // 0x23
	uint32_t nod2_offset;        // 0x25 Road data
	uint32_t nod2_length;        // 0x29
	uint32_t unknown_02d;        // 0x2d
	uint32_t nod3_offset;        // 0x31 Boundary nodes
	uint32_t nod3_length;        // 0x35
	uint16_t nod3_recsize;       // 0x39 {9, a}
	uint32_t unknown_03c;        // 0x3c {0, 2}
	/* break */
	uint32_t nod4_offset;        // 0x3f
	uint32_t nod4_length;        // 0x43
	uint32_t unknown_047;        // 0x47
	uint32_t unknown_04b;        // 0x4b
	uint32_t unknown_04f;        // 0x4f
	uint32_t unknown_053;        // 0x53
	uint32_t unknown_057;        // 0x57
	uint8_t  unknown_05b[12];    // 0x5b-0x66
	uint32_t nod5_offset;        // 0x67
	uint32_t nod5_length;        // 0x6b
	uint16_t nod5_recsize;       // 0x6f
	uint32_t nod6_offset;        // 0x71
	uint32_t nod6_length;        // 0x75
	uint16_t nod6_recsize;       // 0x79
	uint32_t unknown_07b;        // 0x7b
} PACK_STRUCT ;

struct garmin_gmp {
	struct garmin_subfile comm;
	uint32_t unknown_015;
	uint32_t tre_offset;
	uint32_t rgn_offset;
	uint32_t lbl_offset;
	uint32_t net_offset;
	uint32_t nod_offset;
	uint32_t dem_offset;
	uint32_t mar_offset;
} PACK_STRUCT ;

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
} PACK_STRUCT ;

#ifdef GT_POSIX
#else
# pragma pack(pop)
#endif

#endif
