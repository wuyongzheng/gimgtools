#include "gimglib.h"
#include "cmdlib.h"

/* CMD_DUPD deviation unit per degree:
 * Since the max deviation observed is 0.0085509,
 * 0.0085509 * 2500000  = 21377.25 fits will under signed short. */
#define CMD_DUPD 2500000
#define CMD_X0 72
#define CMD_X1 135
#define CMD_Y0 18
#define CMD_Y1 54

static int dryrun = 0;

static int fix_subdiv (struct subfile_struct *map, struct garmin_tre_map_level *maplevel,
		struct garmin_tre_subdiv *div, unsigned char *div2, int div2_len)
{
	int bitshift = 24 - maplevel->bits;
	int center_x, center_y;

	center_x = bytes_to_sint24(div->center_lng);
	center_y = bytes_to_sint24(div->center_lat);
	printf("subdiv cent orig: x=%d(%s) y=%d(%s) width=%5d height=%5d\n",
			center_x, sint24_to_lng(center_x),
			center_y, sint24_to_lat(center_y),
			(div->width<<bitshift) * 2 + 1, (div->height<<bitshift) * 2 + 1);
//	cmd_g24p_fix(&center_x, &center_y);
	{
		int x0 = center_x - (div->width<<bitshift);
		int y0 = center_y - (div->height<<bitshift);
		int x1 = center_x + (div->width<<bitshift);
		int y1 = center_y + (div->height<<bitshift);
		cmd_g24r_fix(&x0, &y0, &x1, &y1, 0);
		center_x = (x0 + x1) / 2;
		center_y = (y0 + y1) / 2;
	}
	printf("subdiv cent fixd: x=%d(%s) y=%d(%s)\n",
			center_x, sint24_to_lng(center_x),
			center_y, sint24_to_lat(center_y));
	if (!dryrun) {
		sint24_to_bytes(center_x, div->center_lng);
		sint24_to_bytes(center_y, div->center_lat);
	}

	return 0;
}

static int fix_tre (struct subfile_struct *tre)
{
	struct garmin_tre *header = (struct garmin_tre *)tre->header;
	struct garmin_tre_map_level *maplevels;
	int x0, y0, x1, y1;
	int level, global_index, level_index;
	unsigned char *subdiv_ptr;

	x0 = bytes_to_sint24(header->westbound);
	y0 = bytes_to_sint24(header->southbound);
	x1 = bytes_to_sint24(header->eastbound);
	y1 = bytes_to_sint24(header->northbound);
	printf("submap bond orig: x0=%d(%s) y0=%d(%s) x1=%d(%s) y1=%d(%s)\n",
			x0, sint24_to_lng(x0), y0, sint24_to_lat(y0),
			x1, sint24_to_lng(x1), y1, sint24_to_lat(y1));
	cmd_g24r_fix(&x0, &y0, &x1, &y1, 1);
	printf("submap bond fixd: x0=%d(%s) y0=%d(%s) x1=%d(%s) y1=%d(%s)\n",
			x0, sint24_to_lng(x0), y0, sint24_to_lat(y0),
			x1, sint24_to_lng(x1), y1, sint24_to_lat(y1));
	if (!dryrun) {
		sint24_to_bytes(x0, header->westbound);
		sint24_to_bytes(y0, header->southbound);
		sint24_to_bytes(x1, header->eastbound);
		sint24_to_bytes(y1, header->northbound);
	}

	if (header->comm.locked) {
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

	for (subdiv_ptr = tre->base + header->tre2_offset,
			level = 0, global_index = 1;
			level < header->tre1_size / 4; level ++) {
		for (level_index = 0; level_index < maplevels[level].nsubdiv;
				level_index ++, global_index ++) {
			if (fix_subdiv(tre, &maplevels[level],
						(struct garmin_tre_subdiv *)subdiv_ptr,
						header->comm.hlen >= 0x86 && header->tre7_size ?
							tre->base + header->tre7_offset +
								global_index * header->tre7_rec_size :
							NULL,
						header->comm.hlen >= 0x86 && header->tre7_size ?
							header->tre7_rec_size : 0))
				return 1;
			subdiv_ptr += level == header->tre1_size / 4 - 1 ?
				sizeof(struct garmin_tre_subdiv) - 2 :
				sizeof(struct garmin_tre_subdiv);
		}
	}
	return 0;
}

static int fix_nod (struct subfile_struct *nod)
{
	struct garmin_nod *header = (struct garmin_nod *)nod->header;

	if (header->nod3_length) {
		unsigned char *ptr = nod->base + header->nod3_offset;
		int length = header->nod3_length;
		int recsize = header->nod3_recsize;

		if (recsize < 9) {
			printf("Error: NOD3 boundry nodes's recsize %d is less than 9\n", recsize);
			return 1;
		}

		for (; length >= recsize; ptr += recsize, length -= recsize) {
			int x = bytes_to_sint24(ptr);
			int y = bytes_to_sint24(ptr+3);
			printf("orig bond-node: e=%d(%s), n=%d(%s)\n",
					x, sint24_to_lng(x), y, sint24_to_lat(y));
			cmd_g24p_fix(&x, &y);
			printf("fixd bond-node: e=%d(%s), n=%d(%s)\n",
					x, sint24_to_lng(x), y, sint24_to_lat(y));
			if (!dryrun) {
				sint24_to_bytes(x, ptr);
				sint24_to_bytes(y, ptr+3);
			}
		}
	}
	return 0;
}

static int fix_map (struct submap_struct *map)
{
	printf("%s\n", map->name);
	return fix_tre(map->tre) || fix_nod(map->nod);
}

static int fix_img (struct gimg_struct *img)
{
	struct submap_struct *map;

	for (map = img->submaps; map != NULL; map = map->next)
		if (fix_map(map))
			return 1;

	return 0;
}

static void usage (int code)
{
	printf("Usage gimgfixcmd file.img\n");
	exit(code);
}

int main (int argc, char **argv)
{
	const char *img_path = NULL;
	struct gimg_struct *img;
	int i;

	for (i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "-h") == 0 ||
				strcmp(argv[i], "--help") == 0 ||
				strcmp(argv[i], "-?") == 0) {
			usage(0);
		} else if (strcmp(argv[i], "-dryrun") == 0) {
			dryrun = 1;
		} else if (argv[i][0] == '-') {
			printf("unknown option %s\n", argv[i]);
			usage(1);
		} else {
			if (img_path == NULL)
				img_path = argv[i];
			else {
				printf("only one subfile please.\n");
				usage(1);
			}
		}
	}

	if (cmd_init("cmd.db")) {
		printf("failed to initialize cmd.db\n");
		return 1;
	}

	if (img_path == NULL) {
		printf("no img file specified\n");
		usage(1);
	}

	img = gimg_open(img_path, dryrun ? 0 : 1);
	if (img == NULL) {
		printf("failed to open or parse %s\n", img_path);
		return 1;
	}

	if (fix_img(img))
		return 1;

	gimg_close(img);

	return 0;
}
