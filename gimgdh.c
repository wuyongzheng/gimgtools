#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/ioctl.h>

struct header_struct {
	const char *imgpath;
	char subfile[16];
	int header_size;
	int header_rel_offset; /* 0 if it's OF */
	int subfile_offset; /* abs offset */
	unsigned char *header;
	char id[4];
};

static int line_columns;
#define MAX_HEADERS 4096
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

static void read_bytes_at (FILE *fp, unsigned long offset, unsigned char *buffer, int size)
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

static void display_headers (void)
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
		printf("%s foff=%8x hoff=%4x hlen=%4x %s %s\n",
				headers[i]->id,
				headers[i]->subfile_offset,
				headers[i]->header_rel_offset,
				headers[i]->header_size,
				headers[i]->imgpath,
				headers[i]->subfile);
	}

	for (ptr = 0; ;ptr += bytes_pre_line) {
		int more_lines = 0, bc, fc;

		/* print address bar */
		printf("%s ", emptyid);
		for (bc = 0; bc < bytes_pre_line - 2; bc += 4) {
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

		if (!more_lines)
			break;
	}
}

#define errexit(...) do {printf(__VA_ARGS__); if (fp) fclose(fp); return 1;} while (0)
static int read_header (const char *imgpath, const char *subfile)
{
	FILE *fp = NULL;
	int block_size, fatstart, fatend, fatcount;
	char subfile_name[16], *subfile_type;
	int added = 0;

	strncpy(subfile_name, subfile, sizeof(subfile_name));
	subfile_name[sizeof(subfile_name) - 1] = '\0';
	if (strchr(subfile_name, '.') == NULL)
		errexit("wrong subfile name %s\n", subfile);
	subfile_type = strchr(subfile_name, '.') + 1;
	strchr(subfile_name, '.')[0] = '\0';

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
		unsigned long offset = fatcount * 512, header_rel_offset, subfile_offset;
		char curr_subfile_name[9], curr_subfile_type[4];
		struct header_struct *header;

		if (read_byte_at(fp, offset) != 1)
			continue;
		if (read_byte_at(fp, offset + 0x1) == ' ') /* rootdir */
			continue;
		if (read_2byte_at(fp, offset + 0x10) != 0)
			continue;

		read_bytes_at(fp, offset + 0x1, (unsigned char *)curr_subfile_name, 8);
		curr_subfile_name[8] = '\0';
		if (strchr(curr_subfile_name, ' '))
			strchr(curr_subfile_name, ' ')[0] = '\0';
		read_bytes_at(fp, offset + 0x9, (unsigned char *)curr_subfile_type, 3);
		curr_subfile_type[3] = '\0';
		if (strchr(curr_subfile_type, ' '))
			strchr(curr_subfile_type, ' ')[0] = '\0';

		if (subfile_name[0] != '\0' && strcasecmp(curr_subfile_name, subfile_name) != 0)
			continue;
		if (strcasecmp(curr_subfile_type, subfile_type) == 0) {
			subfile_offset = read_2byte_at(fp, offset + 0x20) * block_size;
			header_rel_offset = 0;
		} else if (strcmp(curr_subfile_type, "GMP") == 0) {
			subfile_offset = read_2byte_at(fp, offset + 0x20) * block_size;
			if (read_byte_at(fp, subfile_offset + 0x2) != 'G' ||
					read_byte_at(fp, subfile_offset + 0x9) != 'G' ||
					read_byte_at(fp, subfile_offset + 0xa) != 'M' ||
					read_byte_at(fp, subfile_offset + 0xb) != 'P')
				continue;
			if (strcasecmp(subfile_type, "TRE") == 0)
				header_rel_offset = read_4byte_at(fp, subfile_offset + 0x19);
			else if (strcasecmp(subfile_type, "RGN") == 0)
				header_rel_offset = read_4byte_at(fp, subfile_offset + 0x1d);
			else if (strcasecmp(subfile_type, "LBL") == 0)
				header_rel_offset = read_4byte_at(fp, subfile_offset + 0x21);
			else if (strcasecmp(subfile_type, "NET") == 0)
				header_rel_offset = read_4byte_at(fp, subfile_offset + 0x25);
			else if (strcasecmp(subfile_type, "NOD") == 0)
				header_rel_offset = read_4byte_at(fp, subfile_offset + 0x29);
			if (header_rel_offset == 0)
				continue;
		} else {
			continue;
		}

		header = (struct header_struct *)malloc(sizeof(struct header_struct));
		header->imgpath = imgpath;
		strcpy(header->subfile, curr_subfile_name);
		strcat(header->subfile, ".");
		strcat(header->subfile, curr_subfile_type);
		header->header_rel_offset = header_rel_offset;
		header->subfile_offset = subfile_offset;

		header->header_size = read_2byte_at(fp, header->subfile_offset + header->header_rel_offset);
		if (header->header_size < 0x15)
			errexit("wrong header size %d\n", header->header_size);

		header->header = (unsigned char *)malloc(header->header_size);
		read_bytes_at(fp, header->subfile_offset + header->header_rel_offset, header->header, header->header_size);

		headers[header_num ++] = header;
		added ++;
	}

	if (added == 0)
		errexit("cannot find %s %s\n", imgpath, subfile);

	fclose(fp);
	return 0;
}

static void usage (void)
{
	printf("Usage: gimgdh [-w columns] file1.img subfile1 [file2.img subfile2] ...\n");
}

int main (int argc, char *argv[])
{
	int i;

	/* default line_columns */
	if (isatty(1)) {
		struct winsize w;
		ioctl(1, TIOCGWINSZ, &w);
		printf("%x\n", w.ws_col);
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
		} else if (argv[i][0] == '-') {
			printf("unknown option %s\n", argv[i]);
			usage();
			return 1;
		} else {
			if (i + 1 >= argc) {
				printf("file.img and subfile are always in pairs\n");
				usage();
				return 1;
			}
			if (read_header(argv[i], argv[i+1]))
				return 1;
			i ++;
		}
	}

	if (header_num == 0) {
		printf("no files to examine\n");
		usage();
		return 1;
	}

	display_headers();

	return 0;
}
