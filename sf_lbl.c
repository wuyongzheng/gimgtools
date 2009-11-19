#include "gimglib.h"

void dump_lbl (struct subfile_struct *sf)
{
	struct garmin_lbl *header = (struct garmin_lbl *)sf->header;

	assert(sf->typeid == ST_LBL);

	dump_comm(sf->header);

	printf("=== LBL HEADER ===\n");
	printf("LBL1 Data:              reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl1_offset, sf->offset + header->lbl1_offset, header->lbl1_length);
	printf("Shift, Coding:          %d, %d\n",
			header->addr_shift, header->coding);
	printf("LBL2 Country:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl2_offset, sf->offset + header->lbl2_offset, header->lbl2_length, header->lbl2_rec_size);
	printf("unknown_029:            0x%x\n", header->unknown_029);
	printf("LBL3 Region:            reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl3_offset, sf->offset + header->lbl3_offset, header->lbl3_length, header->lbl3_rec_size);
	printf("unknown_037:            0x%x\n", header->unknown_037);
	printf("LBL4 City:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl4_offset, sf->offset + header->lbl4_offset, header->lbl4_length, header->lbl4_rec_size);
	printf("unknown_045:            0x%x\n", header->unknown_045);
	printf("LBL5 POI:               reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl5_offset, sf->offset + header->lbl5_offset, header->lbl5_length, header->lbl5_rec_size);
	printf("unknown_053:            0x%x\n", header->unknown_053);
	printf("LBL6 POI Prop:          reloff=0x%x absoff=0x%x size=0x%x shift=%d mask=0x%x\n",
			header->lbl6_offset, sf->offset + header->lbl6_offset, header->lbl6_length,
			header->lbl6_addr_shift, header->lbl6_glob_mask);
	printf("unknown_061:            %02x%02x%02x\n",
			header->unknown_061[0], header->unknown_061[0], header->unknown_061[0]);
	printf("LBL7 POI Type:          reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl7_offset, sf->offset + header->lbl7_offset, header->lbl7_length, header->lbl7_rec_size);
	printf("unknown_06e:            0x%x\n", header->unknown_06e);
	printf("LBL8 Zip:               reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl8_offset, sf->offset + header->lbl8_offset, header->lbl8_length, header->lbl8_rec_size);
	printf("unknown_07c:            0x%x\n", header->unknown_07c);
	printf("LBL9 Highway:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl9_offset, sf->offset + header->lbl9_offset, header->lbl9_length, header->lbl9_rec_size);
	printf("unknown_08a:            0x%x\n", header->unknown_08a);
	printf("LBL10 Exit:             reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl10_offset, sf->offset + header->lbl10_offset, header->lbl10_length, header->lbl10_rec_size);
	printf("unknown_098:            0x%x\n", header->unknown_098);
	printf("LBL11 Highway Data:     reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x\n",
			header->lbl11_offset, sf->offset + header->lbl11_offset, header->lbl11_length, header->lbl11_rec_size);
	printf("unknown_0a6:            0x%x\n", header->unknown_0a6);
	if (header->comm.hlen <= 0xab)
		goto headerfini;
	if (header->codepage >= 1250 && header->codepage <= 1258)
		printf("Codepage:               Windows-%d\n", header->codepage);
	else if (header->codepage == 950)
		printf("Codepage:               Big5\n");
	else if (header->codepage == 936)
		printf("Codepage:               GBK\n");
	else
		printf("Codepage:               %d\n", header->codepage);

	if (header->comm.hlen > sizeof(struct garmin_lbl))
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				sizeof(struct garmin_lbl), header->comm.hlen - 1,
				header->comm.hlen - sizeof(struct garmin_lbl),
				dump_unknown_bytes((uint8_t *)header + sizeof(struct garmin_lbl), header->comm.hlen - sizeof(struct garmin_lbl)));

headerfini:
	return;
}
