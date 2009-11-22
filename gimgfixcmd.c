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

static int fix_map (struct submap_struct *map)
{
	struct garmin_tre *tre_header = (struct garmin_tre *)map->tre->header;
	int x0, y0, x1, y1;

	printf("%s\n", map->name);
	x0 = bytes_to_sint24(tre_header->westbound);
	y0 = bytes_to_sint24(tre_header->southbound);
	x1 = bytes_to_sint24(tre_header->eastbound);
	y1 = bytes_to_sint24(tre_header->northbound);
	printf("x0=%d(%s) y0=%d(%s) x1=%d(%s) y1=%d(%s)\n",
			x0, sint24_to_lng(x0), y0, sint24_to_lat(y0),
			x1, sint24_to_lng(x1), y1, sint24_to_lat(y1));
	cmd_g24p_fix(&x0, &y0);
	cmd_g24p_fix(&x1, &y1);
	printf("x0=%d(%s) y0=%d(%s) x1=%d(%s) y1=%d(%s)\n",
			x0, sint24_to_lng(x0), y0, sint24_to_lat(y0),
			x1, sint24_to_lng(x1), y1, sint24_to_lat(y1));

	return 0;
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

	img = gimg_open(img_path, 1);
	if (img == NULL) {
		printf("failed to open or parse %s\n", img_path);
		return 1;
	}

	if (fix_img(img))
		return 1;

	gimg_close(img);

	return 0;
}
