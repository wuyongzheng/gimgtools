#include "gimglib.h"

void dump_typ (struct subfile_struct *typ)
{
	struct garmin_typ *header = (struct garmin_typ *)typ->header;

	dump_comm(typ->header);

	printf("=== TYP HEADER ===\n");
	printf("Codepage:   %u\n", header->codepage);
	printf("Point:      reloff=0x%x absoff=0x%x size=0x%x\n",
			header->point_datoff, typ->offset + header->point_datoff,
			header->point_datsize);
	printf("Line:       reloff=0x%x absoff=0x%x size=0x%x\n",
			header->line_datoff, typ->offset + header->line_datoff,
			header->line_datsize);
	printf("Polygon:    reloff=0x%x absoff=0x%x size=0x%x\n",
			header->polygon_datoff, typ->offset + header->polygon_datoff,
			header->polygon_datsize);
	printf("FID:        %u\n", header->fid);
	printf("PID:        %u\n", header->pid);
	printf("PointArr:   reloff=0x%x absoff=0x%x mod=0x%x size=0x%x\n",
			header->point_arroff, typ->offset + header->point_arroff,
			header->point_arrmod, header->point_arrsize);
	printf("LineArr:    reloff=0x%x absoff=0x%x mod=0x%x size=0x%x\n",
			header->line_arroff, typ->offset + header->line_arroff,
			header->line_arrmod, header->line_arrsize);
	printf("PolygonArr: reloff=0x%x absoff=0x%x mod=0x%x size=0x%x\n",
			header->polygon_arroff, typ->offset + header->polygon_arroff,
			header->polygon_arrmod, header->polygon_arrsize);
	printf("DOArr:      reloff=0x%x absoff=0x%x mod=0x%x size=0x%x\n",
			header->draworder_arroff, typ->offset + header->draworder_arroff,
			header->draworder_arrmod, header->draworder_arrsize);
	if (header->comm.hlen < 0x6e)
		goto headerfini;
	printf("NT1:        reloff=0x%x absoff=0x%x size=0x%x flag=0x%x\n",
			header->nt1_datoff, typ->offset + header->nt1_datoff,
			header->nt1_datsize, header->nt1_flag);
	printf("NT1Arr:     reloff=0x%x absoff=0x%x mod=0x%x size=0x%x\n",
			header->nt1_arroff, typ->offset + header->nt1_arroff,
			header->nt1_arrmod, header->nt1_arrsize);
	if (header->comm.hlen < 0x9c)
		goto headerfini;
	printf("Block0:     reloff=0x%x absoff=0x%x size=0x%x x=0x%x y=0x%x\n",
			header->blok0_off, typ->offset + header->blok0_off,
			header->blok0_size, header->blok0_x, header->blok0_y);
	printf("Block1:     reloff=0x%x absoff=0x%x size=0x%x x=0x%x y=0x%x\n",
			header->blok1_off, typ->offset + header->blok1_off,
			header->blok1_size, header->blok1_x, header->blok1_y);
	printf("Block2:     reloff=0x%x absoff=0x%x size=0x%x x=0x%x\n",
			header->blok2_off, typ->offset + header->blok2_off,
			header->blok2_size, header->blok2_x);
	printf("Unknown09a: 0x%x\n", header->unknown_09a);
	if (header->comm.hlen > 0x9c)
		printf("from 0x9c to 0x%x (0x%x bytes): %s\n",
				header->comm.hlen - 1, header->comm.hlen - 0x9c,
				dump_unknown_bytes((uint8_t *)header + 0x9c, header->comm.hlen - 0x9c));
headerfini:

	return;
}
