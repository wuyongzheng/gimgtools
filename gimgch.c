#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct header_struct {
	const char *imgpath;
	char subfile[16];
	int header_rel_offset; /* 0 if it's OF */
	int header_size;
	int subfile_offset; /* abs offset */
	int subfile_size;
	unsigned char *header;
	char id[4];
};

#define MAX_HEADERS 8192
static struct header_struct *headers[MAX_HEADERS];
static int header_num = 0;


static int read_byte_at (FILE *fp, unsigned long offset)
{
	if (fseek(fp, offset, SEEK_SET)) {
		perror(NULL);
		exit(1);
	}
	return getc(fp);
}

static int read_2byte_at (FILE *fp, unsigned long offset)
{
	int n = 0;
	if (fseek(fp, offset, SEEK_SET)) {
		perror(NULL);
		exit(1);
	}
	if (fread(&n, 2, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
	return n;
}

static unsigned int read_4byte_at (FILE *fp, unsigned long offset)
{
	int n = 0;
	if (fseek(fp, offset, SEEK_SET)) {
		perror(NULL);
		exit(1);
	}
	if (fread(&n, 4, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
	return n;
}

static void read_bytes_at (FILE *fp, unsigned long offset,
		unsigned char *buffer, int size)
{
	if (fseek(fp, offset, SEEK_SET)) {
		perror(NULL);
		exit(1);
	}
	if (fread(buffer, size, 1, fp) != 1) {
		perror(NULL);
		exit(1);
	}
}

static void display_headers (int line_columns)
{
	int i, ptr, bytes_pre_line;
	char emptyid[4];

	for (i = 0; i < header_num; i ++) {
		if (header_num <= 26) {
			headers[i]->id[0] = 'a' + i;
			headers[i]->id[1] = '\0';
		} else if (header_num <= 26 * 26) {
			headers[i]->id[0] = 'a' + i / 26;
			headers[i]->id[1] = 'a' + i % 26;
			headers[i]->id[2] = '\0';
		} else {
			headers[i]->id[0] = 'a' + i / (26 * 26);
			headers[i]->id[1] = 'a' + i / 26 % 26;
			headers[i]->id[2] = 'a' + i % 26;
			headers[i]->id[3] = '\0';
		}
	}
	bytes_pre_line = (line_columns - strlen(headers[0]->id) - 1) / 2;
	memset(emptyid, ' ', 4);
	emptyid[strlen(headers[0]->id)] = '\0';

	for (i = 0; i < header_num; i ++) {
		printf("%s %8x=foff %8x=flen %4x=hoff %4x=hlen %s %s\n",
				headers[i]->id,
				headers[i]->subfile_offset,
				headers[i]->subfile_size,
				headers[i]->header_rel_offset,
				headers[i]->header_size,
				headers[i]->imgpath,
				headers[i]->subfile);
	}
	printf("\n");

	for (ptr = 0; ; ptr += bytes_pre_line) {
		int more_lines = 0, bc, fc;

		/* print address bar */
		printf("%s ", emptyid);
		for (bc = 0; bc <= bytes_pre_line - 2; bc += 4) {
			printf("%-4x", ptr + bc);
			if (bc < bytes_pre_line - 4)
				printf("    ");
		}
		printf("\n");

		/* print header content */
		for (fc = 0; fc < header_num; fc ++) {
			printf("%s ", headers[fc]->id);
			if (headers[fc]->header_size > ptr + bytes_pre_line)
				more_lines = 1;
//			if (headers[fc]->header_size <= ptr) {
//				printf("\n");
//				continue;
//			}
			for (bc = 0; bc < headers[fc]->header_size - ptr && bc < bytes_pre_line; bc ++)
				printf("%02x", headers[fc]->header[ptr + bc]);
			printf("\n");
		}

		/* print diff */
		if (header_num > 1) {
			printf("%s ", emptyid);
			for (bc = 0; bc < bytes_pre_line; bc ++) {
				int b = -1;
				for (fc = 0; fc < header_num; fc ++) {
					if (ptr + bc >= headers[fc]->header_size)
						continue;
					if (b == -1)
						b = headers[fc]->header[ptr + bc];
					if (b != headers[fc]->header[ptr + bc])
						break;
				}
				printf(fc == header_num ? "  " : "* " );
			}
			printf("\n");
		}

		/* print address bar again */
		printf("%s ", emptyid);
		for (bc = 0; bc <= bytes_pre_line - 2; bc += 4) {
			printf("%-4x", ptr + bc);
			if (bc < bytes_pre_line - 4)
				printf("    ");
		}
		printf("\n\n");

		if (!more_lines)
			break;
	}
}

static int add_header(FILE *fp, const char *imgpath, const char *sf_name,
		int subfile_offset, int subfile_size, int header_rel_offset)
{
	struct header_struct *header =
		(struct header_struct *)malloc(sizeof(struct header_struct));

	header->imgpath = imgpath;
	strcpy(header->subfile, sf_name);
	header->subfile_offset = subfile_offset;
	header->subfile_size = subfile_size;
	header->header_rel_offset = header_rel_offset;

	header->header_size = read_2byte_at(fp, subfile_offset + header_rel_offset);
	if (header->header_size < 0x15) {
		printf("wrong header size %d\n", header->header_size);
		return 1;
	}

	header->header = (unsigned char *)malloc(header->header_size);
	read_bytes_at(fp, subfile_offset + header_rel_offset,
			header->header, header->header_size);

	if (header_num >= MAX_HEADERS) {
		printf("too many files\n");
		return 1;
	}
	headers[header_num ++] = header;
	return 0;
}

#define errexit(...) \
	do { \
		printf(__VA_ARGS__); \
		if (fp) \
			fclose(fp); \
		return 1; \
	} while (0)
static int read_header (const char *imgpath, const char *subfile_name_pattern,
		int match_maximum)
{
	FILE *fp = NULL;
	int block_size, fatstart, fatend, fatcount;
	int added = 0;

	fp = fopen(imgpath, "rb");
	if (fp == NULL)
		errexit("can't open %s", imgpath);

	if ((read_byte_at(fp, 0) ^ read_byte_at(fp, 0x10)) != 'D' ||
			(read_byte_at(fp, 0) ^ read_byte_at(fp, 0x15)) != 'G')
		errexit("Not a garmin img file.\n");
	if (read_byte_at(fp, 0))
		errexit("XOR is not 0. Fix it first.\n");

	block_size = 1 << (read_byte_at(fp, 0x61) + read_byte_at(fp, 0x62));

	fatstart = read_byte_at(fp, 0x40) == 0 ? 3 : read_byte_at(fp, 0x40);
	if (read_4byte_at(fp, 0x40c) == 0) { // use root dir. assume it's the first file
		unsigned long offset = fatstart * 512;
		if (read_byte_at(fp, offset) != 1 ||
				read_byte_at(fp, offset + 0x1) != ' ' ||
				read_byte_at(fp, offset + 0x9) != ' ')
			errexit("imgheader.data_offset = 0 but the first file is not root dir!\n");
		fatstart ++;
		if (read_4byte_at(fp, offset + 0xc) % 512 != 0 ||
				read_4byte_at(fp, offset + 0xc) <= fatstart * 512)
			errexit("rootdir.size = %x which is bad\n", read_4byte_at(fp, offset + 0xc));
		fatend = read_4byte_at(fp, offset + 0xc) / 512;
		//printf("Parsing fat use rootdir, fatstart=%d, fatend=%d\n", fatstart, fatend);
	} else {
		fatend = read_4byte_at(fp, 0x40c) / 512;
		//printf("Parsing fat use data_offset, fatstart=%d, fatend=%d\n", fatstart, fatend);
	}

	for (fatcount = fatstart; fatcount < fatend; fatcount ++) {
		unsigned long offset = fatcount * 512, subfile_offset, subfile_size;
		char sf_name[16];

		if (read_byte_at(fp, offset) != 1)
			continue;
		if (read_byte_at(fp, offset + 0x1) == ' ') /* rootdir */
			continue;
		if (read_2byte_at(fp, offset + 0x10) != 0) /* part != 0 */
			continue;

		/* make up sf_name */
		memset(sf_name, 0, sizeof(sf_name));
		read_bytes_at(fp, offset + 0x1, (unsigned char *)sf_name, 8);
		if (strchr(sf_name, ' ') != NULL)
			strchr(sf_name, ' ')[0] = '\0';
		strcat(sf_name, ".");
		read_bytes_at(fp, offset + 0x9, (unsigned char *)sf_name + strlen(sf_name), 3);
		if (strchr(sf_name, ' ') != NULL)
			strchr(sf_name, ' ')[0] = '\0';
		/* a simple security check */
		if (strchr(sf_name, '/') != NULL || strchr(sf_name, '\\') != NULL)
			errexit("invalid subfile name %s\n", sf_name);

		subfile_offset = read_2byte_at(fp, offset + 0x20) * block_size;
		subfile_size = read_4byte_at(fp, offset + 0xc);

		if (subfile_name_pattern == NULL || strstr(sf_name, subfile_name_pattern)) {
			/* header integrity test */
			if (read_byte_at(fp, subfile_offset + 2) != 'G' ||
					read_byte_at(fp, subfile_offset + 3) != 'A' ||
					read_byte_at(fp, subfile_offset + 4) != 'R') {
				if (!strstr(sf_name, ".MPS")) /* I know MPS doesn't have proper header */
					printf("warning: %s has invalid header\n", sf_name);
				continue;
			}
			if (add_header(fp, imgpath, sf_name, subfile_offset, subfile_size, 0))
				errexit("add_header failed\n");
			added ++;
			if (match_maximum && added >= match_maximum)
				goto out;
		}

		if (strstr(sf_name, ".GMP")) {
			static const char *exts[] = {".TRE", ".RGN", ".LBL", ".NET", ".NOD"};
			static const int offs[] = {0x19, 0x1d, 0x21, 0x25, 0x29};
			int i;
			for (i = 0; i < 5; i ++) {
				char sf_new_name[16];
				int header_rel_offset;
				strcpy(sf_new_name, sf_name);
				strcpy(strstr(sf_new_name, ".GMP"), exts[i]);
				if (subfile_name_pattern != NULL &&
						strstr(sf_new_name, subfile_name_pattern) == NULL)
					continue;
				header_rel_offset = read_4byte_at(fp, subfile_offset + offs[i]);
				if (header_rel_offset == 0)
					continue;
				if (add_header(fp, imgpath, sf_new_name, subfile_offset,
							subfile_size, header_rel_offset))
					errexit("add_header failed\n");
				added ++;
				if (match_maximum && added >= match_maximum)
					goto out;
			}
		}
	}

	if (added == 0)
		printf("no matching subfile found in %s\n", imgpath);
out:
	fclose(fp);
	return 0;
}

static void usage (void)
{
	printf("Usage: gimgch [-w columns] [-m max_sf_per_img] "
			"[-s subfile_name_pattern] file1.img file2.img ...\n");
}

int main (int argc, char *argv[])
{
	int line_columns;
	#define MAX_IMGS 1024
	char *imgs[MAX_IMGS];
	int img_num = 0;
	char *subfile_name_pattern = NULL;
	int match_maximum = 1;
	int i;

	/* default line_columns */
	if (isatty(1)) {
		struct winsize w;
		ioctl(1, TIOCGWINSZ, &w);
		line_columns = w.ws_col;
	} else {
		line_columns = 80;
	}

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
		} else if (strcmp("-m", argv[i]) == 0) {
			if (i + 1 >= argc) {
				usage();
				return 1;
			}
			match_maximum = atoi(argv[++ i]);
		} else if (strcmp("-s", argv[i]) == 0) {
			char *ptr;
			if (i + 1 >= argc) {
				usage();
				return 1;
			}
			subfile_name_pattern = argv[++ i];
			for (ptr = subfile_name_pattern; *ptr; ptr ++)
				if (*ptr >= 'a' && *ptr <= 'z')
					*ptr += 'A' - 'a';
		} else if (argv[i][0] == '-') {
			printf("unknown option %s\n", argv[i]);
			usage();
			return 1;
		} else {
			imgs[img_num ++] = argv[i];
		}
	}

	if (img_num == 0) {
		printf("no files to examine\n");
		usage();
		return 1;
	}

	for (i = 0; i < img_num; i ++)
		if (read_header(imgs[i], subfile_name_pattern, match_maximum))
			return 1;

	if (header_num == 0) {
		printf("no subfile found\n");
		return 1;
	}

	display_headers(line_columns);

	return 0;
}
