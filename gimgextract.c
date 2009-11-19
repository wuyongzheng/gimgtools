#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int read_byte_at (FILE *fp, unsigned long offset)
{
	if (fseek(fp, offset, SEEK_SET)) {
		perror(NULL);
		exit(1);
	}
	return getc(fp);
}

int read_2byte_at (FILE *fp, unsigned long offset)
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

unsigned int read_4byte_at (FILE *fp, unsigned long offset)
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

void read_bytes_at (FILE *fp, unsigned long offset, unsigned char *buffer, int size)
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

#define errexit(...) do {printf(__VA_ARGS__); return 1;} while (0)

int main (int argc, char *argv[])
{
	FILE *infp;
	int block_size, fatstart, fatend, fatcount;

	if (argc != 2) {
		printf("usage: %s file.img\n", argv[0]);
		return 1;
	}

	infp = fopen(argv[1], "rb");
	if (infp == NULL)
		errexit("can't open %s\n", argv[1]);

	if ((read_byte_at(infp, 0) ^ read_byte_at(infp, 0x10)) != 'D' ||
			(read_byte_at(infp, 0) ^ read_byte_at(infp, 0x15)) != 'G')
		errexit("Not a garmin img file.\n");
	if (read_byte_at(infp, 0))
		errexit("XOR is not 0. Fix it first.\n");

	block_size = 1 << (read_byte_at(infp, 0x61) + read_byte_at(infp, 0x62));

	fatstart = read_byte_at(infp, 0x40) == 0 ? 3 : read_byte_at(infp, 0x40);
	if (read_4byte_at(infp, 0x40c) == 0) { // use root dir. assume it's the first file
		unsigned long offset = fatstart * 512;
		if (read_byte_at(infp, offset) != 1 ||
				read_byte_at(infp, offset + 0x1) != ' ' ||
				read_byte_at(infp, offset + 0x9) != ' ')
			errexit("imgheader.data_offset = 0 but the first file is not root dir!\n");
		fatstart ++;
		if (read_4byte_at(infp, offset + 0xc) % 512 != 0 ||
				read_4byte_at(infp, offset + 0xc) <= fatstart * 512)
			errexit("rootdir.size = %x which is bad\n", read_4byte_at(infp, offset + 0xc));
		fatend = read_4byte_at(infp, offset + 0xc) / 512;
	} else {
		fatend = read_4byte_at(infp, 0x40c) / 512;
	}

	for (fatcount = fatstart; fatcount < fatend; fatcount ++) {
		unsigned long offset = fatcount * 512, sf_offset, sf_size;
		char sf_name[16];
		FILE *outfp;

		if (read_byte_at(infp, offset) != 1)
			continue;
		if (read_byte_at(infp, offset + 0x1) == ' ') /* rootdir */
			continue;
		if (read_2byte_at(infp, offset + 0x10) != 0) /* part != 0 */
			continue;

		memset(sf_name, 0, sizeof(sf_name));
		read_bytes_at(infp, offset + 0x1, (unsigned char *)sf_name, 8);
		while (strrchr(sf_name, ' ') != NULL)
			strrchr(sf_name, ' ')[0] = '\0';
		sf_name[strlen(sf_name)] = '.';
		read_bytes_at(infp, offset + 0x9, (unsigned char *)sf_name + strlen(sf_name), 3);
		while (strrchr(sf_name, ' ') != NULL)
			strrchr(sf_name, ' ')[0] = '\0';
		if (strchr(sf_name, '/') != NULL || strchr(sf_name, '\\') != NULL) /* a simple security check */
			errexit("invalid subfile name %s\n", sf_name);

		sf_offset = read_2byte_at(infp, offset + 0x20) * block_size;
		sf_size = read_4byte_at(infp, offset + 0xc);
		if (sf_offset == 0 || sf_size == 0)
			errexit("subfile %s has 0 offset or size: 0x%lx 0x%lx\n", sf_name, sf_offset, sf_size);

		outfp = fopen(sf_name, "wb");
		if (outfp == NULL)
			errexit("can't open %s for writing\n", sf_name);
		while (sf_size != 0) {
			unsigned char buff[4096];
			int len = sf_size < sizeof(buff) ? sf_size :  sizeof(buff);
			read_bytes_at(infp, sf_offset, buff, len);
			if (fwrite(buff, len, 1, outfp) != 1)
				errexit("can't write to %s\n", sf_name);
			sf_offset += len;
			sf_size -= len;
		}
		fclose(outfp);
	}

	fclose(infp);
	return 0;
}
