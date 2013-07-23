#include "gimglib.h"

static void usage (void)
{
	printf("Usage:   gimginfo [options] imgfile [subfile]\n");
	printf("Example: gimginfo america-nt-2011.img\n");
	printf("Example: gimginfo america-nt-2011.img MAPSOURC.MPS\n");
	printf("Example: gimginfo america-nt-2011.img I07DE4E4.TRE\n");
	printf("Example: gimginfo america-nt-2011.img I07DE4E4.LBL\n");
}

int main (int argc, char **argv)
{
	char *img_path = NULL;
	char *subfile_name = NULL;
	struct gimg_struct *img;
	int i;

	for (i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "-h") == 0 ||
				strcmp(argv[i], "--help") == 0 ||
				strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		} else if (argv[i][0] == '-') {
			printf("unknown option %s\n", argv[i]);
			usage();
			return 1;
		} else {
			if (img_path == NULL)
				img_path = argv[i];
			else if (subfile_name == NULL)
				subfile_name = argv[i];
			else {
				printf("only one subfile please.\n");
				usage();
				return 1;
			}
		}
	}

	if (img_path == NULL) {
		printf("no img file specified\n");
		usage();
		return 1;
	}

	img = gimg_open(img_path, 0);
	if (img == NULL) {
		printf("failed to open or parse %s\n", img_path);
		return 1;
	}

	if (subfile_name) {
		dump_subfile(img, subfile_name);
	} else {
		dump_img(img);
	}

	gimg_close(img);

	return 0;
}
