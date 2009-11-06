#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define errexit(...) do {printf(__VA_ARGS__); exit(1);} while (0)

struct patch_struct {
	unsigned long offset;
	unsigned long size;
	unsigned char *data;
	struct patch_struct *next;
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
	struct patch_struct *new_patch = (struct patch_struct *)malloc(sizeof(struct patch_struct) + size);
	if (new_patch == NULL)
		errexit("out of memory\n");
	memset(new_patch, 0, sizeof(struct patch_struct) + size);
	new_patch->size = size;
	new_patch->data = (unsigned char *)new_patch + sizeof(struct patch_struct);
	new_patch->next = patch_list;
	return new_patch;
}

struct patch_struct *create_patch (FILE *fp)
{
	return NULL;
}

void apply_patch (FILE *fp, struct patch_struct *patch_list)
{
	struct patch_struct *patch;

	for (patch = patch_list; patch != NULL; patch = patch->next) {
		if (fseek(fp, patch->offset, SEEK_SET)) {
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
	struct patch_struct *patch_list;

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

	fp = fopen(argv[1], "rb");
	if (fp == NULL) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}
	patch_list = create_patch(fp);
	fclose(fp);
	if (patch_list == NULL)
		return 1;

	fp = fopen(argv[1], "rb+");
	if (fp == NULL) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}
	apply_patch(fp, patch_list);
	fclose(fp);

	return 0;
}
