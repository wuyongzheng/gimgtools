#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "util_indep.h"

struct patch_struct {
	unsigned char *data;
	struct patch_struct *next;
	off_t offset;
	unsigned long size;
};


void unlockml (unsigned char *dst, const unsigned char *src,
		int size, unsigned int key)
{
	static const unsigned char shuf[] = {
		0xb, 0xc, 0xa, 0x0,
		0x8, 0xf, 0x2, 0x1,
		0x6, 0x4, 0x9, 0x3,
		0xd, 0x5, 0x7, 0xe};
	int i, ringctr;
	int key_sum = shuf[((key >> 24) + (key >> 16) + (key >> 8) + key) & 0xf];

	for (i = 0, ringctr = 16; i < size; i ++) {
		unsigned int upper = src[i] >> 4;
		unsigned int lower = src[i];

		upper -= key_sum;
		upper -= key >> ringctr;
		upper -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		lower -= key_sum;
		lower -= key >> ringctr;
		lower -= shuf[(key >> ringctr) & 0xf];
		ringctr = ringctr ? ringctr - 4 : 16;

		dst[i] = ((upper << 4) & 0xf0) | (lower & 0xf);
	}
}

struct patch_struct *prepend_patch (struct patch_struct *patch_list, unsigned long size)
{
	struct patch_struct *new_patch =
		(struct patch_struct *)malloc(sizeof(struct patch_struct) + size);
	assert(new_patch != NULL);
	memset(new_patch, 0, sizeof(struct patch_struct) + size);
	new_patch->size = size;
	new_patch->data = (unsigned char *)new_patch + sizeof(struct patch_struct);
	new_patch->next = patch_list;
	return new_patch;
}

struct patch_struct *unlock_tre (FILE *fp, struct patch_struct *patch,
		off_t base, off_t header)
{
	unsigned int key, mloff, mlsize;
	unsigned char encml[64]; /* at most 16 levels */

	key = read_4byte_at(fp, header + 0xaa);
	printf("key:          0x%x\n", key);
	mloff = read_4byte_at(fp, header + 0x21);
	mlsize = read_4byte_at(fp, header + 0x25);
	if (mlsize > 64)
		errexit("Map level's size %d > 64\n", mlsize);
	read_bytes_at(fp, base + mloff, encml, mlsize);
	printf("encrypted ml: ");
	hexdump(encml, mlsize);
	patch = prepend_patch(patch, mlsize);
	patch->offset = base + mloff;
	unlockml(patch->data, encml, mlsize, key);
	printf("decrypted ml: ");
	hexdump(patch->data, mlsize);

	patch = prepend_patch(patch, 20);
	patch->offset = header + 0x9a;
	read_bytes_at(fp, header + 0x9a, patch->data, 20);
	printf("orig key: ");
	hexdump(patch->data, 20);
	patch->data[1] ^= 0x80;
	memcpy(patch->data + 8, patch->data + 12, 4);
	memset(patch->data + 16, 0, 4);
	printf("new key:  ");
	hexdump(patch->data, 20);

	patch = prepend_patch(patch, 1);
	patch->offset = header + 0xd;
	patch->data[0] = read_byte_at(fp, (header + 0xd)) ^ 0x80;

	return patch;
}

struct patch_struct *create_patch (FILE *fp)
{
	struct patch_struct *patch = NULL;
	int block_size, fatstart, fatend, fatcount;

	if ((read_byte_at(fp, 0) ^ read_byte_at(fp, 0x10)) != 'D' ||
			(read_byte_at(fp, 0) ^ read_byte_at(fp, 0x15)) != 'G')
		errexit("Not a garmin img file.\n");
	if (read_byte_at(fp, 0))
		errexit("XOR is not 0. Fix it first.\n");

	block_size = 1 << (read_byte_at(fp, 0x61) + read_byte_at(fp, 0x62));

	fatstart = read_byte_at(fp, 0x40) == 0 ? 3 : read_byte_at(fp, 0x40);
	if (read_4byte_at(fp, 0x40c) == 0) { // use root dir. assume it's the first file
		off_t offset = fatstart * 512;
		if (read_byte_at(fp, offset) != 1 ||
				read_byte_at(fp, offset + 0x1) != ' ' ||
				read_byte_at(fp, offset + 0x9) != ' ')
			errexit("imgheader.data_offset = 0 but the first file is not root dir!\n");
		fatstart ++;
		if (read_4byte_at(fp, offset + 0xc) % 512 != 0 ||
				read_4byte_at(fp, offset + 0xc) <= fatstart * 512)
			errexit("rootdir.size = %x which is bad\n", read_4byte_at(fp, offset + 0xc));
		fatend = read_4byte_at(fp, offset + 0xc) / 512;
		printf("Parsing fat use rootdir, fatstart=%d, fatend=%d\n", fatstart, fatend);
	} else {
		fatend = read_4byte_at(fp, 0x40c) / 512;
		printf("Parsing fat use data_offset, fatstart=%d, fatend=%d\n", fatstart, fatend);
	}

	for (fatcount = fatstart; fatcount < fatend; fatcount ++) {
		off_t offset = fatcount * 512;
		char subfile_name[16];

		if (read_byte_at(fp, offset) != 1)
			continue;
		if (read_byte_at(fp, offset + 0x1) == ' ') /* rootdir */
			continue;
		if (read_2byte_at(fp, offset + 0x10) != 0)
			continue;

		// TODO fix space padding
		read_bytes_at(fp, offset + 0x1, (unsigned char *)subfile_name, 8);
		subfile_name[8] = '.';
		read_bytes_at(fp, offset + 0x9, (unsigned char *)subfile_name + 9, 3);
		subfile_name[12] = '\0';

		if (read_byte_at(fp, offset + 0x9) == 'G' &&
				read_byte_at(fp, offset + 0xa) == 'M' &&
				read_byte_at(fp, offset + 0xb) == 'P') {
			off_t tre_offset;
			offset = read_2byte_at(fp, offset + 0x20) * block_size;
			if (read_byte_at(fp, offset + 0x2) != 'G' ||
					read_byte_at(fp, offset + 0x9) != 'G' ||
					read_byte_at(fp, offset + 0xa) != 'M' ||
					read_byte_at(fp, offset + 0xb) != 'P')
				errexit("Unknown GMP file header.\n");
			if (read_byte_at(fp, offset + 0xd) & 0x80)
				printf("WARNING: don't know how to unlock GMP file %s\n", subfile_name);
			tre_offset = offset + read_4byte_at(fp, offset + 0x19);
			if (read_byte_at(fp, tre_offset + 0x2) != 'G' ||
					read_byte_at(fp, tre_offset + 0x9) != 'T' ||
					read_byte_at(fp, tre_offset + 0xa) != 'R' ||
					read_byte_at(fp, tre_offset + 0xb) != 'E')
				errexit("Unknown TRE file header at %x in GMP.\n", (unsigned int)tre_offset);
			if (read_byte_at(fp, tre_offset + 0xd) & 0x80) {
				printf("Unlocking subfile %s\n", subfile_name);
				patch = unlock_tre(fp, patch, offset, tre_offset);
			} else {
				printf("Skipping plain subfile %s\n", subfile_name);
			}
			//TODO check other subsubfiles
		} else if (read_byte_at(fp, offset + 0x9) == 'T' &&
				read_byte_at(fp, offset + 0xa) == 'R' &&
				read_byte_at(fp, offset + 0xb) == 'E') {
			offset = read_2byte_at(fp, offset + 0x20) * block_size;
			if (read_byte_at(fp, offset + 0x2) != 'G' ||
					read_byte_at(fp, offset + 0x9) != 'T' ||
					read_byte_at(fp, offset + 0xa) != 'R' ||
					read_byte_at(fp, offset + 0xb) != 'E')
				errexit("Unknown TRE file header.\n");
			if (read_byte_at(fp, offset + 0xd) & 0x80) {
				printf("Unlocking subfile %s\n", subfile_name);
				patch = unlock_tre(fp, patch, offset, offset);
			} else {
				printf("Skipping plain subfile %s\n", subfile_name);
			}
		} else if (strcmp(subfile_name, "MAPSOURC.MPS") == 0) {
			printf("Skipping MAPSOURC.MPS\n");
		} else {
			offset = read_2byte_at(fp, offset + 0x20) * block_size;
			if (read_byte_at(fp, offset + 0xd) & 0x80) {
				printf("WARNING: don't know how to unlock subfile %s\n", subfile_name);
			} else {
				printf("Skipping plain subfile %s\n", subfile_name);
			}
		}
	}

	return patch;
}

void apply_patch (FILE *fp, struct patch_struct *patch_list)
{
	struct patch_struct *patch;

	for (patch = patch_list; patch != NULL; patch = patch->next) {
		if (myfseek64(fp, patch->offset)) {
			perror(NULL);
			return;
		}
		if (fwrite(patch->data, patch->size, 1, fp) != 1) {
			perror(NULL);
			return;
		}
	}
}

int main (int argc, char *argv[])
{
	FILE *fp;
	struct patch_struct *patch;

	if (argc != 2) {
		printf("usage: %s file.img\n", argv[0]);
		return 1;
	}
	if (strcmp(argv[1], "-h") == 0 ||
			strcmp(argv[1], "--help") == 0 ||
			strcmp(argv[1], "-?") == 0) {
		printf("usage: %s file.img\n", argv[0]);
		return 1;
	}

	printf("Analyzing file.\n");
	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}
	patch = create_patch(fp);
	fclose(fp);
	if (patch == NULL)
		return 1;

	printf("Writing to file.\n");
	fp = fopen(argv[1], "rb+");
	if (fp == NULL) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}
	apply_patch(fp, patch);
	fclose(fp);

	return 0;
}
