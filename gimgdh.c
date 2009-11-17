#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct header_struct {
	const char *filepath;
	const char *subfilename;
	int header_size;
	unsigned char *header;
};

static int line_columns = 80;
#define MAX_HEADERS 4096
static struct header_struct *headers[MAX_HEADERS];
static int header_num = 0;


static void display_headers (void)
{
}

static int read_header (struct header_struct *header)
{
	return 0;
}

static void usage (void)
{
	printf("Usage: gimgdh [-w columns] file1.img subfile1 [file2.img subfile2] ...\n");
}

int main (int argc, char *argv[])
{
	int i;

	for (i = 1; i < argc; i ++) {
		if (strcmp("-h", argv[i]) == 0 ||
				strcmp(argv[i], "--help") == 0 ||
				strcmp(argv[i], "-?") == 0) {
			usage();
			return 0;
		} else if (strcmp("-w", argv[i]) == 0) {
			if (i + 1 >= argc) {
				usage();
				return 1;
			}
			line_columns = atoi(argv[++ i]);
		} else if (argv[i][0] == '-') {
			printf("unknown option %s\n", argv[i]);
			usage();
			return 1;
		} else {
			if (i + 2 >= argc) {
				usage();
				return 1;
			}
			headers[header_num] = (struct header_struct *)malloc(sizeof(struct header_struct));
			headers[header_num]->filepath = argv[i+1];
			headers[header_num]->subfilename = argv[i+2];
			header_num ++;
			i += 2;
		}
	}

	if (header_num == 0) {
		printf("no files to examine\n");
		usage();
		return 1;
	}

	for (i = 1; i < header_num; i ++)
		if (read_header(headers[i]))
			return 1;

	display_headers();

	return 0;
}
