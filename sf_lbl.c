#include "gimglib.h"

void dump_lbl (struct subfile_struct *sf)
{
	struct garmin_lbl *header = (struct garmin_lbl *)sf->header;
	int progress = 0;

	assert(sf->typeid == ST_LBL);

	dump_comm(sf->header);

	printf("=== LBL HEADER ===\n");
	if (header->comm.hlen < 0xaa)
		goto headerfini;
	progress = 0xaa;
	printf("LBL1 Data:          reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl1_offset, sf->offset + header->lbl1_offset, header->lbl1_length);
	printf("Shift, Coding:      %d, %d\n",
			header->addr_shift, header->coding);
	printf("LBL2 Country:       reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?029=0x%x\n",
			header->lbl2_offset, sf->offset + header->lbl2_offset,
			header->lbl2_length, header->lbl2_recsize, header->lbl2_u);
	printf("LBL3 Region:        reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?037=0x%x\n",
			header->lbl3_offset, sf->offset + header->lbl3_offset,
			header->lbl3_length, header->lbl3_recsize, header->lbl3_u);
	printf("LBL4 City:          reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?045=0x%x\n",
			header->lbl4_offset, sf->offset + header->lbl4_offset,
			header->lbl4_length, header->lbl4_recsize, header->lbl4_u);
	printf("LBL5 POI:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?053=0x%x\n",
			header->lbl5_offset, sf->offset + header->lbl5_offset,
			header->lbl5_length, header->lbl5_recsize, header->lbl5_u);
	printf("LBL6 POI Prop:      reloff=0x%x absoff=0x%x size=0x%x shift=%d mask=0x%x ?061=0x%02x%02x%02x\n",
			header->lbl6_offset, sf->offset + header->lbl6_offset, header->lbl6_length,
			header->lbl6_addr_shift, header->lbl6_glob_mask,
			header->lbl6_u[0], header->lbl6_u[0], header->lbl6_u[0]);
	printf("LBL7 POI Type:      reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?06e=0x%x\n",
			header->lbl7_offset, sf->offset + header->lbl7_offset,
			header->lbl7_length, header->lbl7_recsize, header->lbl7_u);
	printf("LBL8 Zip:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?07c=0x%x\n",
			header->lbl8_offset, sf->offset + header->lbl8_offset,
			header->lbl8_length, header->lbl8_recsize, header->lbl8_u);
	printf("LBL9 Highway:       reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?08a=0x%x\n",
			header->lbl9_offset, sf->offset + header->lbl9_offset,
			header->lbl9_length, header->lbl9_recsize, header->lbl9_u);
	printf("LBL10 Exit:         reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?098=0x%x\n",
			header->lbl10_offset, sf->offset + header->lbl10_offset,
			header->lbl10_length, header->lbl10_recsize, header->lbl10_u);
	printf("LBL11 Highway Data: reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?0a6=0x%x\n",
			header->lbl11_offset, sf->offset + header->lbl11_offset,
			header->lbl11_length, header->lbl11_recsize, header->lbl11_u);

	if (header->comm.hlen < 0xb0)
		goto headerfini;
	progress = 0xb0;
	if (header->codepage >= 1250 && header->codepage <= 1258)
		printf("Codepage:           Windows-%d\n", header->codepage);
	else if (header->codepage == 950)
		printf("Codepage:           Big5\n");
	else if (header->codepage == 936)
		printf("Codepage:           GBK\n");
	else
		printf("Codepage:           %d\n", header->codepage);
	printf("Codepage2:          0x%d\n", header->codepage2);
	printf("Codepage3:          0x%x\n", header->codepage3);

	if (header->comm.hlen < 0xb8)
		goto headerfini;
	progress = 0xb8;
	printf("LBL12:              reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl12_offset, sf->offset + header->lbl12_offset, header->lbl12_length);
	if (header->comm.hlen < 0xc4)
		goto headerfini;
	progress = 0xc4;
	printf("LBL13:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl13_offset, sf->offset + header->lbl13_offset, header->lbl13_length,
			header->lbl13_recsize, header->lbl13_u);
	if (header->comm.hlen < 0xd0)
		goto headerfini;
	progress = 0xd0;
	printf("LBL14:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl14_offset, sf->offset + header->lbl14_offset, header->lbl14_length,
			header->lbl14_recsize, header->lbl14_u);
	if (header->comm.hlen < 0xde)
		goto headerfini;
	progress = 0xde;
	printf("LBL15:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl15_offset, sf->offset + header->lbl15_offset, header->lbl15_length,
			header->lbl15_recsize, header->lbl15_u);
	if (header->comm.hlen < 0xec)
		goto headerfini;
	progress = 0xec;
	printf("LBL16:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl16_offset, sf->offset + header->lbl16_offset, header->lbl16_length,
			header->lbl16_recsize, header->lbl16_u);
	if (header->comm.hlen < 0xfa)
		goto headerfini;
	progress = 0xfa;
	printf("LBL17:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl17_offset, sf->offset + header->lbl17_offset, header->lbl17_length,
			header->lbl17_recsize, header->lbl17_u);
	if (header->comm.hlen < 0x108)
		goto headerfini;
	progress = 0x108;
	printf("LBL18:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl18_offset, sf->offset + header->lbl18_offset, header->lbl18_length,
			header->lbl18_recsize, header->lbl18_u);
	if (header->comm.hlen < 0x116)
		goto headerfini;
	progress = 0x116;
	printf("LBL19:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl19_offset, sf->offset + header->lbl19_offset, header->lbl19_length,
			header->lbl19_recsize, header->lbl19_u);
	if (header->comm.hlen < 0x124)
		goto headerfini;
	progress = 0x124;
	printf("LBL20:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl20_offset, sf->offset + header->lbl20_offset, header->lbl20_length,
			header->lbl20_recsize, header->lbl20_u);
	if (header->comm.hlen < 0x132)
		goto headerfini;
	progress = 0x132;
	printf("LBL21:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl21_offset, sf->offset + header->lbl21_offset, header->lbl21_length,
			header->lbl21_recsize, header->lbl21_u);
	if (header->comm.hlen < 0x140)
		goto headerfini;
	progress = 0x140;
	printf("LBL22:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl22_offset, sf->offset + header->lbl22_offset, header->lbl22_length,
			header->lbl22_recsize, header->lbl22_u);
	if (header->comm.hlen < 0x14e)
		goto headerfini;
	progress = 0x14e;
	printf("LBL23:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl23_offset, sf->offset + header->lbl23_offset, header->lbl23_length,
			header->lbl23_recsize, header->lbl23_u);
	if (header->comm.hlen < 0x15a)
		goto headerfini;
	progress = 0x15a;
	printf("LBL24:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl24_offset, sf->offset + header->lbl24_offset, header->lbl24_length,
			header->lbl24_recsize, header->lbl24_u);
	if (header->comm.hlen < 0x168)
		goto headerfini;
	progress = 0x168;
	printf("LBL25:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl25_offset, sf->offset + header->lbl25_offset, header->lbl25_length,
			header->lbl25_recsize, header->lbl25_u);
	if (header->comm.hlen < 0x176)
		goto headerfini;
	progress = 0x176;
	printf("LBL26:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl26_offset, sf->offset + header->lbl26_offset, header->lbl26_length,
			header->lbl26_recsize, header->lbl26_u);
	if (header->comm.hlen < 0x184)
		goto headerfini;
	progress = 0x184;
	printf("LBL27:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl27_offset, sf->offset + header->lbl27_offset, header->lbl27_length,
			header->lbl27_recsize, header->lbl27_u);
	if (header->comm.hlen < 0x192)
		goto headerfini;
	progress = 0x192;
	printf("LBL28:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl28_offset, sf->offset + header->lbl28_offset, header->lbl28_length,
			header->lbl28_recsize, header->lbl28_u);
	if (header->comm.hlen < 0x19a)
		goto headerfini;
	progress = 0x19a;
	printf("LBL29:              reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl29_offset, sf->offset + header->lbl29_offset, header->lbl29_length);
	if (header->comm.hlen < 0x1a6)
		goto headerfini;
	progress = 0x1a6;
	printf("LBL30:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl30_offset, sf->offset + header->lbl30_offset, header->lbl30_length,
			header->lbl30_recsize, header->lbl30_u);
	if (header->comm.hlen < 0x1b2)
		goto headerfini;
	progress = 0x1b2;
	printf("LBL31:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl31_offset, sf->offset + header->lbl31_offset, header->lbl31_length,
			header->lbl31_recsize, header->lbl31_u);
	if (header->comm.hlen < 0x1be)
		goto headerfini;
	progress = 0x1be;
	printf("LBL32:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl32_offset, sf->offset + header->lbl32_offset, header->lbl32_length,
			header->lbl32_recsize, header->lbl32_u);
	if (header->comm.hlen < 0x1ca)
		goto headerfini;
	progress = 0x1ca;
	printf("LBL33:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl33_offset, sf->offset + header->lbl33_offset, header->lbl33_length,
			header->lbl33_recsize, header->lbl33_u);
	if (header->comm.hlen < 0x1d8)
		goto headerfini;
	progress = 0x1d8;
	printf("LBL34:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl34_offset, sf->offset + header->lbl34_offset, header->lbl34_length,
			header->lbl34_recsize, header->lbl34_u);
	if (header->comm.hlen < 0x1e6)
		goto headerfini;
	progress = 0x1e6;
	printf("LBL35:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl35_offset, sf->offset + header->lbl35_offset, header->lbl35_length,
			header->lbl35_recsize, header->lbl35_u);
	if (header->comm.hlen < 0x1f2)
		goto headerfini;
	progress = 0x1f2;
	printf("LBL36:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl36_offset, sf->offset + header->lbl36_offset, header->lbl36_length,
			header->lbl36_recsize, header->lbl36_u);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	return;
}
