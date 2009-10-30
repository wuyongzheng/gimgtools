#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "garmin_struct.h"

struct submap_struct;
struct subfile_struct {
	struct subfile_header_struct *header;
	unsigned int size;
	char name[9];
	char type[4];
	int isnt;
	struct submap_struct *map;
	struct subfile_struct *next;
};

struct submap_struct {
	char name[9];
	struct subfile_struct *tre;
	struct subfile_struct *rgn;
	struct subfile_struct *lbl;
	struct subfile_struct *net;
	struct subfile_struct *nod;
	struct subfile_struct *srt;
	struct submap_struct *next;
};

int verbose = 1;
unsigned char *img_base;
unsigned int img_size;
struct subfile_struct *subfiles, *subfiles_tail;
struct submap_struct *submaps, *submaps_tail;
int subfile_num;
int block_size;

#define vlog(...) if (verbose) fprintf(stderr, "LOG: " __VA_ARGS__)
#define warn(...) fprintf(stderr, "WARNING: " __VA_ARGS__)

int map_img (const char *path)
{
	int img_fd;
	struct stat sb;

	img_fd = open(path, O_RDONLY);
	if (img_fd == -1)
		return 1;

	if (fstat(img_fd, &sb) == -1)
		return 1;
	img_size = sb.st_size;
	vlog("file size = %u\n", img_size);

	img_base = mmap(NULL, img_size, PROT_READ, MAP_PRIVATE, img_fd, 0);
	if (img_base == MAP_FAILED)
		return 1;

	close(img_fd);

	return 0;
}

void unmap_img (void)
{
	munmap(img_base, img_size);
}

/* assign global variables */
int parse_img (void)
{
	struct img_header_struct *img = (struct img_header_struct *)img_base;
	struct subfile_struct *subfile;
	struct submap_struct *submap;
	int fatstart, fatend, i, prev_block;

	if (img->xor != 0) {
		printf("XOR is not 0. Fix it first.\n");
		return 1;
	}
	if (strcmp(img->signature, "DSKIMG") != 0) {
		printf("not DSKIMG\n");
		return 1;
	}

	block_size = 1 << (img->blockexp1 + img->blockexp2);
	vlog("block size = 1 << (%d + %d) = %d\n", img->blockexp1, img->blockexp2, block_size);

	subfiles = subfiles_tail = NULL;
	fatstart = img->fatoffset == 0 ? 3 : img->fatoffset;
	if (img->dataoffset == 0) { // use root dir. assume it's the first file
		struct fat_header_struct *fat = (struct fat_header_struct *)(img_base + fatstart * 512);
		if (fat->flag != 1 || memcmp(fat->name, "        ", 8) != 0 || memcmp(fat->type, "   ", 3) != 0) {
			printf("imgheader.dataoffset = 0 but the first file is not root dir!\n");
			return 1;
		}
		fatstart ++;
		assert(fat->size % 512 == 0 && fat->size > fatstart * 512);
		fatend = fat->size / 512;
		vlog("parsing fat use rootdir, fatstart=%d, fatend=%d\n", fatstart, fatend);
	} else {
		fatend = img->dataoffset / 512;
		vlog("parsing fat use dataoffset, fatstart=%d, fatend=%d\n", fatstart, fatend);
	}

	for (i = fatstart; i < fatend; i ++) {
		struct fat_header_struct *fat = (struct fat_header_struct *)(img_base + i * 512);
		int j;

		if (fat->flag != 1) {
			vlog("fat%d.flag = %d, skipped\n", i, fat->flag);
			continue;
		}
		if (memcmp(fat->name, "        ", 8) == 0) {
			vlog("fat%d is rootdir (size=%d), skipped\n", i, fat->size);
			continue;
		}

		/* scan fat table  */
		if (fat->part == 0) {
			if (memcmp(fat->type, "GMP", 3) == 0) {
				struct gmp_header_struct *gmp = (struct gmp_header_struct *)(img_base + fat->blocks[0] * block_size);
				int k;

				vlog("fat%d: gmp 0x%x+%d\n", i, fat->blocks[0] * block_size, fat->size);
				for (k = 0; k < 5; k ++) {
					uint32_t offset = *(&gmp->tre_offset + k);
					if (offset == 0) {
						vlog("GMP has no such type\n");
						continue;
					}
					offset += fat->blocks[0] * block_size;

					subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
					subfile->header = (struct subfile_header_struct *)(img_base + offset);
					subfile->size = fat->size - *(&gmp->tre_offset + k);
					subfile->isnt = 1;
					memcpy(subfile->name, fat->name, 8); subfile->name[8] = '\0'; //TODO fix space padding
					switch (k) {
						case 0: strncpy(subfile->type, "TRE", 3); break;
						case 1: strncpy(subfile->type, "RGN", 3); break;
						case 2: strncpy(subfile->type, "LBL", 3); break;
						case 3: strncpy(subfile->type, "NET", 3); break;
						case 4: strncpy(subfile->type, "NOD", 3); break;
					}
					vlog("%s.GMP.%s: reloff=0x%x, absoff=0x%x\n",
							subfile->name, subfile->type,
							*(&gmp->tre_offset + k), offset);

					subfile->next = NULL;
					if (subfiles_tail == NULL)
						subfiles_tail = subfiles = subfile;
					else
						subfiles_tail = subfiles_tail->next = subfile;
				}
			} else {
				subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
				subfile->header = (struct subfile_header_struct *)(img_base + fat->blocks[0] * block_size);
				subfile->size = fat->size;
				subfile->isnt = 0;
				memcpy(subfile->name, fat->name, 8); subfile->name[8] = '\0'; //TODO fix space padding
				memcpy(subfile->type, fat->type, 3); subfile->type[8] = '\0';
				vlog("fat%d: %s %s 0x%x+%u\n", i, subfile->name, subfile->type,
						fat->blocks[0] * block_size, subfile->size);

				subfile->next = NULL;
				if (subfiles_tail == NULL)
					subfiles_tail = subfiles = subfile;
				else
					subfiles_tail = subfiles_tail->next = subfile;
			}

			prev_block = fat->blocks[0] - 1;
		}

		for (j = 0; j < 240; j ++) {
			if (fat->blocks[j] != 0xffff && fat->blocks[j] != ++ prev_block) {
				printf("file block number is not continuous\n");
				return 1;
			}
		}
	}
	vlog("parsing fat finished\n");

	/* build submaps link list */
	submaps = submaps_tail = NULL;
	for (subfile = subfiles; subfile != NULL; subfile = subfile->next) {
		if (memcmp(subfile->type, "TRE", 3) != 0)
			continue;
		submap = (struct submap_struct *)malloc(sizeof(struct submap_struct));
		memset(submap, 0, sizeof(struct submap_struct));
		strcpy(submap->name, subfile->name);
		submap->next = NULL;
		if (submaps_tail == NULL)
			submaps_tail = submaps = submap;
		else
			submaps_tail = submaps_tail->next = submap;
	}
	for (subfile = subfiles; subfile != NULL; subfile = subfile->next) {
		for (submap = submaps; submap != NULL; submap = submap->next)
			if (strcmp(subfile->name, submap->name) == 0)
				break;
		subfile->map = submap;
		if (submap == NULL)
			break;
		if (strcmp(subfile->type, "TRE")) {
			if (submap->tre)
				warn("file %s has duplicate TRE\n", subfile->name);
			submap->tre = subfile;
		} else if (strcmp(subfile->type, "RGN")) {
			if (submap->rgn)
				warn("file %s has duplicate RGN\n", subfile->name);
			submap->rgn = subfile;
		} else if (strcmp(subfile->type, "LBL")) {
			if (submap->lbl)
				warn("file %s has duplicate LBL\n", subfile->name);
			submap->lbl = subfile;
		} else if (strcmp(subfile->type, "NET")) {
			if (submap->net)
				warn("file %s has duplicate NET\n", subfile->name);
			submap->net = subfile;
		} else if (strcmp(subfile->type, "NOD")) {
			if (submap->nod)
				warn("file %s has duplicate NOD\n", subfile->name);
			submap->nod = subfile;
		} else if (strcmp(subfile->type, "SRT")) {
			if (submap->srt)
				warn("file %s has duplicate SRT\n", subfile->name);
			submap->srt = subfile;
		} else {
			warn("file %s's type %s not known\n", subfile->name, subfile->type);
		}
	}

	return 0;
}

void dump_img (void)
{
	struct img_header_struct *img = (struct img_header_struct *)img_base;
	struct subfile_struct *subfile;

	for (subfile = subfiles; subfile != NULL; subfile = subfile->next) {
		printf("%s.%s: offset=0x%x, size=%u\n",
				subfile->name, subfile->type,
				(int)((uint8_t *)subfile->header - img_base), subfile->size);
	}
}

int main (int argc, char **argv)
{
	assert(sizeof(struct img_header_struct) == 0x600);

	if (argc != 2) {
		printf("Usage: %s file.img\n", argv[0]);
		return 1;
	}

	if (map_img(argv[1]))
		return 1;

	if (parse_img())
		return 1;

	dump_img();

	unmap_img();

	return 0;
}
