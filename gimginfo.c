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

enum subtype {
	ST_TRE, ST_RGN, ST_LBL, ST_NET, ST_NOD, // these 5 should match the GMP header
	ST_SRT,
	ST_GMP,
	ST_TYP, ST_MDR, ST_TRF, ST_MPS, ST_QSI,
	ST_UNKNOWN
};

struct submap_struct;
struct subfile_struct {
	struct garmin_subfile *header;
	unsigned char *base; // it's same as [header] if it's OF
	unsigned int size;
	char name[9];
	char type[4];
	enum subtype typeid;
	int isnt; // only two cases: OF, NT
	struct submap_struct *map;
	struct subfile_struct *next;
	struct subfile_struct *orphan_next;
};

struct submap_struct {
	char name[9];
	union {
		struct subfile_struct *subfiles[6];
		struct {
			struct subfile_struct *tre; // always set.
			struct subfile_struct *rgn;
			struct subfile_struct *lbl;
			struct subfile_struct *net;
			struct subfile_struct *nod;
			struct subfile_struct *srt;
		};
	};
	struct submap_struct *next;
};

int verbose = 1;
unsigned char *img_base;
unsigned int img_size;
struct subfile_struct *subfiles, *subfiles_tail;
struct submap_struct *submaps, *submaps_tail;
struct subfile_struct *orphans, *orphans_tail; // files not belonging to any submap
int subfile_num;
int block_size;

#define vlog(...) if (verbose) fprintf(stderr, "LOG: " __VA_ARGS__)
#define warn(...) fprintf(stderr, "WARNING: " __VA_ARGS__)

enum subtype get_subtype_id (const char *str) // only use 3 chars from str
{
	if (memcmp(str, "TRE", 3) == 0) return ST_TRE;
	if (memcmp(str, "RGN", 3) == 0) return ST_RGN;
	if (memcmp(str, "LBL", 3) == 0) return ST_LBL;
	if (memcmp(str, "NET", 3) == 0) return ST_NET;
	if (memcmp(str, "NOD", 3) == 0) return ST_NOD;
	if (memcmp(str, "SRT", 3) == 0) return ST_SRT;
	if (memcmp(str, "GMP", 3) == 0) return ST_GMP;
	if (memcmp(str, "TYP", 3) == 0) return ST_TYP;
	if (memcmp(str, "MDR", 3) == 0) return ST_MDR;
	if (memcmp(str, "TRF", 3) == 0) return ST_TRF;
	if (memcmp(str, "MPS", 3) == 0) return ST_MPS;
	if (memcmp(str, "QSI", 3) == 0) return ST_QSI;
	return ST_UNKNOWN;
}

const char *get_subtype_name (enum subtype id)
{
	const static char *type_names[] = {
		"TRE", "RGN", "LBL", "NET", "NOD",
		"SRT", "GMP", "TYP", "MDR", "TRF",
		"MPS", "QSI"};
	return id >= ST_UNKNOWN ? NULL : type_names[id];
}

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
	struct garmin_img *img = (struct garmin_img *)img_base;
	struct subfile_struct *subfile;
	struct submap_struct *submap;
	int fatstart, fatend, i, prev_block;

	if (img->xor_byte != 0) {
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
	fatstart = img->fat_offset == 0 ? 3 : img->fat_offset;
	if (img->data_offset == 0) { // use root dir. assume it's the first file
		struct garmin_fat *fat = (struct garmin_fat *)(img_base + fatstart * 512);
		if (fat->flag != 1 || memcmp(fat->name, "        ", 8) != 0 || memcmp(fat->type, "   ", 3) != 0) {
			printf("imgheader.data_offset = 0 but the first file is not root dir!\n");
			return 1;
		}
		fatstart ++;
		assert(fat->size % 512 == 0 && fat->size > fatstart * 512);
		fatend = fat->size / 512;
		vlog("parsing fat use rootdir, fatstart=%d, fatend=%d\n", fatstart, fatend);
	} else {
		fatend = img->data_offset / 512;
		vlog("parsing fat use data_offset, fatstart=%d, fatend=%d\n", fatstart, fatend);
	}

	for (i = fatstart; i < fatend; i ++) {
		struct garmin_fat *fat = (struct garmin_fat *)(img_base + i * 512);
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
				struct garmin_gmp *gmp = (struct garmin_gmp *)(img_base + fat->blocks[0] * block_size);
				int k;

				vlog("fat%d: gmp 0x%x+0x%x\n", i, fat->blocks[0] * block_size, fat->size);
				for (k = 0; k < 5; k ++) { // k matches ST_TRE to ST_NOD
					uint32_t abs_offset, rel_offset = *(&gmp->tre_offset + k);
					if (rel_offset == 0) {
						vlog("GMP has no %s\n", get_subtype_name(k));
						continue;
					}
					abs_offset = rel_offset + fat->blocks[0] * block_size;

					subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
					subfile->header = (struct garmin_subfile *)(img_base + abs_offset);
					subfile->base = (unsigned char *)gmp;
					subfile->size = fat->size;
					subfile->isnt = 1;
					memcpy(subfile->name, fat->name, 8); subfile->name[8] = '\0'; //TODO fix space padding
					strncpy(subfile->type, get_subtype_name(k), 3);
					subfile->typeid = k;
					vlog("%s.GMP.%s: reloff=0x%x, absoff=0x%x\n",
							subfile->name, subfile->type,
							rel_offset, abs_offset);

					subfile->next = NULL;
					if (subfiles_tail == NULL)
						subfiles_tail = subfiles = subfile;
					else
						subfiles_tail = subfiles_tail->next = subfile;
				}
			} else {
				subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
				subfile->header = (struct garmin_subfile *)(img_base + fat->blocks[0] * block_size);
				subfile->base = img_base + fat->blocks[0] * block_size;
				subfile->size = fat->size;
				subfile->isnt = 0;
				memcpy(subfile->name, fat->name, 8); subfile->name[8] = '\0'; //TODO fix space padding
				memcpy(subfile->type, fat->type, 3); subfile->type[8] = '\0';
				subfile->typeid = get_subtype_id(subfile->type);
				vlog("fat%d: %s %s 0x%x+0x%x\n", i, subfile->name, subfile->type,
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
	orphans = orphans_tail = NULL;
	for (subfile = subfiles; subfile != NULL; subfile = subfile->next) {
		if (subfile->typeid != ST_TRE)
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
		if (submap == NULL) { // orphan
			subfile->orphan_next = NULL;
			if (orphans_tail == NULL)
				orphans = orphans_tail = subfile;
			else
				orphans_tail = orphans_tail->orphan_next = subfile;
		} else {
			if (subfile->typeid <= ST_SRT) {
				if (submap->subfiles[subfile->typeid])
					warn("file %s has duplicate %s\n", subfile->name, subfile->type);
				submap->subfiles[subfile->typeid] = subfile;
			} else {
				warn("file %s's type %s not known\n", subfile->name, subfile->type);
			}
		}
	}

	return 0;
}

const char *dump_unknown_bytes (uint8_t *bytes, int size)
{
	static char *buffer = NULL;
	static int buffer_size = 0;
	int ptr, outptr, repeat_count, repeat_byte;

	if (buffer == NULL) {
		buffer_size = size * 4 > 4096 ? size * 4 : 4096;
		buffer = (char *)malloc(buffer_size);
	} else if (buffer_size < size * 4) {
		buffer_size = size * 4;
		buffer = (char *)realloc(buffer, buffer_size);
	}

	for (repeat_byte = bytes[0], ptr = 1, repeat_count = outptr = 0; ptr < size; ptr ++) {
		if (bytes[ptr] == repeat_byte) {
			repeat_count ++;
		} else {
			if (repeat_count > 1) {
				outptr += sprintf(buffer + outptr, "%02x(%d)", repeat_byte, repeat_count);
			} else {
				outptr += sprintf(buffer + outptr, "%02x", repeat_byte);
			}
			repeat_byte = bytes[ptr];
			repeat_count = 1;
		}
	}
	if (repeat_count > 1) {
		outptr += sprintf(buffer + outptr, "%02x(%d)", repeat_byte, repeat_count);
	} else {
		outptr += sprintf(buffer + outptr, "%02x", repeat_byte);
	}

	return buffer;
}

void dump_img (void)
{
	struct garmin_img *img = (struct garmin_img *)img_base;
	struct subfile_struct *subfile;
	struct submap_struct *submap;
	char buffer[100];

	printf("=== IMAGE HEADER ===\n");
	printf("unknown_001   %s\n", dump_unknown_bytes(img->unknown_001, sizeof(img->unknown_001)));
	printf("umonth        %d\n", img->umonth);
	printf("uyear         %d\n", img->uyear);
	printf("unknown_00c   %s\n", dump_unknown_bytes(img->unknown_00c, sizeof(img->unknown_00c)));
	printf("checksum      %d\n", img->checksum);
	memcpy(buffer, img->signature, sizeof(img->signature));
	buffer[sizeof(img->signature)] = '\0';
	printf("signature     \"%s\"\n", buffer);
	printf("unknown_017   %d\n", img->unknown_017);
	printf("sectors       %d\n", img->sectors);
	printf("heads         %d\n", img->heads);
	printf("cylinders     %d\n", img->cylinders);
	printf("unknown_01e   %d\n", img->unknown_01e);
	printf("unknown_020   %s\n", dump_unknown_bytes(img->unknown_020, sizeof(img->unknown_020)));
	printf("cyear         %d\n", img->cyear);
	printf("cmonth        %d\n", img->cmonth);
	printf("cdate         %d\n", img->cdate);
	printf("chour         %d\n", img->chour);
	printf("cminute       %d\n", img->cminute);
	printf("csecond       %d\n", img->csecond);
	printf("fat_offset    %d\n", img->fat_offset);
	memcpy(buffer, img->identifier, sizeof(img->identifier));
	buffer[sizeof(img->identifier)] = '\0';
	printf("identifier    \"%s\"\n", buffer);
	printf("unknown_048   %d\n", img->unknown_048);
	memcpy(buffer, img->desc1, sizeof(img->desc1));
	memcpy(buffer + sizeof(img->desc1), img->desc2, sizeof(img->desc2));
	buffer[sizeof(img->desc1) + sizeof(img->desc2)] = '\0';
	printf("desc          \"%s\"\n", buffer);
	printf("heads1        %d\n", img->heads1);
	printf("sectors1      %d\n", img->sectors1);
	printf("blockexp1     %d\n", img->blockexp1);
	printf("blockexp2     %d\n", img->blockexp2);
	printf("unknown_063   %d\n", img->unknown_063);
	printf("unknown_083   %d\n", img->unknown_083);
	printf("unknown_084   %s\n", dump_unknown_bytes(img->unknown_084, sizeof(img->unknown_084)));
	printf("data_offset   %d\n", img->data_offset);
	printf("unknown_410   %s\n", dump_unknown_bytes(img->unknown_410, sizeof(img->unknown_410)));

	printf("=== SUBMAPS ===\n");
	for (submap = submaps; submap != NULL; submap = submap->next) {
		if (submap->tre->isnt) {
			int k;
			printf("%s: NT 0x%x+0x%x\n",
					submap->name,
					(int)(submap->tre->base - img_base),
					submap->tre->size);
			for (k = 0; k < 5; k ++) {
				if (submap->subfiles[k])
					printf(" %s abs=0x%x rel=0x%x\n",
							get_subtype_name(k),
							(int)((uint8_t *)submap->subfiles[k]->header - img_base),
							(int)((uint8_t *)submap->subfiles[k]->header - submap->subfiles[k]->base));
				else
					printf(" %s NIL\n", get_subtype_name(k));
			}
			if (submap->srt)
				printf(" SRT off=0x%x size=0x%x\n",
						(int)(submap->srt->base - img_base),
						submap->srt->size);
			else
				printf(" SRT NIL\n");
		} else { // OF
			int k;
			printf("%s: OF\n", submap->name);
			for (k = 0; k <= ST_SRT; k ++) {
				if (submap->subfiles[k])
					printf(" %s off=0x%x size=0x%x\n",
							get_subtype_name(k),
							(int)(submap->subfiles[k]->base - img_base),
							submap->subfiles[k]->size);
				else {
					if (k != ST_SRT)
						printf(" %s NIL\n", get_subtype_name(k));
				}
			}
		}
	}
	printf("=== OTHER SUBFILES ===\n");
	for (subfile = orphans; subfile != NULL; subfile = subfile->orphan_next) {
		printf("%s.%s: off=0x%x, size=0x%x\n",
				subfile->name, subfile->type,
				(int)(subfile->base - img_base), subfile->size);
	}
}

int main (int argc, char **argv)
{
	assert(sizeof(struct garmin_img) == 0x600);

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
