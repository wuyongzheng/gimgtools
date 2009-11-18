#include "gimglib.h"

//#define vlog(...) if (option_verbose) fprintf(stderr, "LOG: " __VA_ARGS__)
//#define warn(...) fprintf(stderr, "WARNING: " __VA_ARGS__)

#ifdef GT_POSIX
static int map_img (const char *path, int readonly, uint8_t **pbase, unsigned int *psize)
{
	int img_fd;
	struct stat sb;
	uint8_t *base;

	img_fd = open(path, readonly ? O_RDONLY : O_RDWR);
	if (img_fd == -1) {
		fprintf(stderr, "cannot open file %s\n", path);
		return 1;
	}

	if (fstat(img_fd, &sb) == -1) {
		fprintf(stderr, "cannot get size of file %s\n", path);
		return 1;
	}
	if (sb.st_size == 0) {
		fprintf(stderr, "file %s is an empty file\n", path);
		return 1;
	}
	//vlog("file size = %u\n", img_size);

	base = mmap(NULL, sb.st_size, readonly ? PROT_READ : PROT_READ | PROT_WRITE,
			MAP_PRIVATE, img_fd, 0);
	if (base == MAP_FAILED) {
		fprintf(stderr, "cannot map file %s into memory\n", path);
		return 1;
	}

	close(img_fd);

	*pbase = base;
	*psize = sb.st_size;
	return 0;
}

static void unmap_img (uint8_t *base, unsigned int size)
{
	munmap(base, size);
}
#else
static int map_img (const char *path, int readonly, uint8_t **pbase, unsigned int *psize)
{
	HANDLE hfile, hmapping;
	uint8_t *base;
	unsigned int size;

	hfile = CreateFile(path, readonly ? GENERIC_READ : GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hfile == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "cannot open file %s\n", path);
		return 1;
	}

	size = GetFileSize(hfile, NULL);
	if (size == INVALID_FILE_SIZE) {
		fprintf(stderr, "cannot get size of file %s\n", path);
		return 1;
	}
	if (size == 0) {
		fprintf(stderr, "file %s is an empty file\n", path);
		return 1;
	}
	//vlog("file size = %u\n", size);

	hmapping = CreateFileMapping(hfile, NULL,
			readonly ? PAGE_READONLY : PAGE_READWRITE,
			0, 0, NULL);
	if (hmapping == NULL) {
		fprintf(stderr, "cannot map (1) file %s into memory\n", path);
		return 1;
	}
	base = MapViewOfFile(hmapping, readonly ? FILE_MAP_READ : FILE_MAP_WRITE, 0, 0, 0);
	if (base == NULL) {
		fprintf(stderr, "cannot map (2) file %s into memory\n", path);
		return 1;
	}

	CloseHandle(hmapping);
	CloseHandle(hfile);
	*pbase = base;
	*psize = size;
	return 0;
}

static void unmap_img (uint8_t *base, unsigned int size)
{
	UnmapViewOfFile(base);
}
#endif

static int parse_img (struct gimg_struct *img)
{
	struct garmin_img *img_header = (struct garmin_img *)(img->base);
	struct subfile_struct *subfile, *subfiles, *subfiles_tail, *orphans, *orphans_tail;
	struct submap_struct *submap, *submaps, *submaps_tail;
	unsigned int block_size, fatstart, fatend, i, prev_block;

	if (img_header->xor_byte != 0) {
		fprintf(stderr, "XOR is not 0. Fix it first.\n");
		return 1;
	}
	if (strcmp(img_header->signature, "DSKIMG") != 0) {
		fprintf(stderr, "not DSKIMG\n");
		return 1;
	}

	block_size = 1 << (img_header->blockexp1 + img_header->blockexp2);
	//vlog("block size = 1 << (%d + %d) = %d\n", img_header->blockexp1, img_header->blockexp2, block_size);

	subfiles = subfiles_tail = NULL;
	fatstart = img_header->fat_offset == 0 ? 3 : img_header->fat_offset;
	if (img_header->data_offset == 0) { // use root dir. assume it's the first file
		struct garmin_fat *fat = (struct garmin_fat *)(img->base + fatstart * 512);
		if (fat->flag != 1 || memcmp(fat->name, "        ", 8) != 0 || memcmp(fat->type, "   ", 3) != 0) {
			fprintf(stderr, "imgheader.data_offset = 0 but the first file is not root dir!\n");
			return 1;
		}
		fatstart ++;
		assert(fat->size % 512 == 0 && fat->size > (unsigned)fatstart * 512);
		fatend = fat->size / 512;
		//vlog("parsing fat use rootdir, fatstart=%d, fatend=%d\n", fatstart, fatend);
	} else {
		fatend = img_header->data_offset / 512;
		//vlog("parsing fat use data_offset, fatstart=%d, fatend=%d\n", fatstart, fatend);
	}

	for (i = fatstart; i < fatend; i ++) {
		struct garmin_fat *fat = (struct garmin_fat *)(img->base + i * 512);
		int j;

		if (fat->flag != 1) {
			//vlog("fat%d.flag = %d, skipped\n", i, fat->flag);
			continue;
		}
		if (memcmp(fat->name, "        ", 8) == 0) {
			//vlog("fat%d is rootdir (size=%d), skipped\n", i, fat->size);
			continue;
		}

		/* scan fat table  */
		if (fat->part == 0) {
			if (memcmp(fat->type, "GMP", 3) == 0) {
				struct garmin_gmp *gmp = (struct garmin_gmp *)(img->base + fat->blocks[0] * block_size);
				int k;

				//vlog("fat%d: gmp 0x%x+0x%x\n", i, fat->blocks[0] * block_size, fat->size);
				for (k = 0; k < 5; k ++) { // k matches ST_TRE to ST_NOD
					uint32_t abs_offset, rel_offset = *(&gmp->tre_offset + k);
					if (rel_offset == 0) {
						//vlog("GMP has no %s\n", get_subtype_name(k));
						continue;
					}
					abs_offset = rel_offset + fat->blocks[0] * block_size;

					subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
					memset(subfile, 0, sizeof(struct subfile_struct));
					subfile->header = (struct garmin_subfile *)(img->base + abs_offset);
					subfile->base = (unsigned char *)gmp;
					subfile->offset = fat->blocks[0] * block_size;
					subfile->size = fat->size;
					subfile->isnt = 1;
					memcpy(subfile->name, fat->name, 8);
					string_trim(subfile->name, -1);
					strncpy(subfile->type, get_subtype_name(k), 3);
					sprintf(subfile->fullname, "%s.%s", subfile->name, subfile->type);
					subfile->typeid = k;
					//vlog("%s.GMP.%s: reloff=0x%x, absoff=0x%x\n",
					//		subfile->name, subfile->type,
					//		rel_offset, abs_offset);

					subfile->next = NULL;
					if (subfiles_tail == NULL)
						subfiles_tail = subfiles = subfile;
					else
						subfiles_tail = subfiles_tail->next = subfile;
				}
			} else {
				subfile = (struct subfile_struct *)malloc(sizeof(struct subfile_struct));
				memset(subfile, 0, sizeof(struct subfile_struct));
				subfile->header = (struct garmin_subfile *)(img->base + fat->blocks[0] * block_size);
				subfile->base = img->base + fat->blocks[0] * block_size;
				subfile->offset = fat->blocks[0] * block_size;
				subfile->size = fat->size;
				subfile->isnt = 0;
				memcpy(subfile->name, fat->name, 8);
				string_trim(subfile->name, -1);
				memcpy(subfile->type, fat->type, 3);
				string_trim(subfile->type, -1);
				sprintf(subfile->fullname, "%s.%s", subfile->name, subfile->type);
				subfile->typeid = get_subtype_id(subfile->type);
				//vlog("fat%d: %s %s 0x%x+0x%x\n", i, subfile->name, subfile->type,
				//		fat->blocks[0] * block_size, subfile->size);

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
				fprintf(stderr, "file block number is not continuous\n");
				return 1;
			}
		}
	}
	//vlog("parsing fat finished\n");

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
					fprintf(stderr, "Warning: file %s has duplicate %s\n", subfile->name, subfile->type);
				submap->subfiles[subfile->typeid] = subfile;
			} else {
				fprintf(stderr, "Warning: file %s's type %s not known\n", subfile->name, subfile->type);
			}
		}
	}

	img->subfiles = subfiles;
	img->submaps = submaps;
	img->orphans = orphans;
	return 0;
}

void dump_comm (struct garmin_subfile *header)
{
	char buffer[12];

	printf("=== COMMON HEADER ===\n");
	printf("hlen          %d (0x%x)\n", header->hlen, header->hlen);
	memcpy(buffer, header->type, sizeof(header->type));
	buffer[sizeof(header->type)] = '\0';
	printf("type          %s\n", buffer);
	printf("unknown_00c   %d\n", header->unknown_00c);
	printf("locked        0x%x\n", header->locked);
	printf("year          %d\n", header->year);
	printf("month         %d\n", header->month);
	printf("date          %d\n", header->date);
	printf("hour          %d\n", header->hour);
	printf("minute        %d\n", header->minute);
	printf("second        %d\n", header->second);
}

void dump_img (struct gimg_struct *img)
{
	struct garmin_img *img_header = (struct garmin_img *)(img->base);
	struct subfile_struct *subfile;
	struct submap_struct *submap;
	char buffer[100];

	printf("=== IMAGE HEADER ===\n");
	printf("unknown_001   %s\n", dump_unknown_bytes(img_header->unknown_001, sizeof(img_header->unknown_001)));
	printf("umonth        %d\n", img_header->umonth);
	printf("uyear         %d\n", img_header->uyear);
	printf("unknown_00c   %s\n", dump_unknown_bytes(img_header->unknown_00c, sizeof(img_header->unknown_00c)));
	printf("checksum      %d\n", img_header->checksum);
	memcpy(buffer, img_header->signature, sizeof(img_header->signature));
	buffer[sizeof(img_header->signature)] = '\0';
	printf("signature     \"%s\"\n", buffer);
	printf("unknown_017   %d\n", img_header->unknown_017);
	printf("sectors       %d\n", img_header->sectors);
	printf("heads         %d\n", img_header->heads);
	printf("cylinders     %d\n", img_header->cylinders);
	printf("unknown_01e   %d\n", img_header->unknown_01e);
	printf("unknown_020   %s\n", dump_unknown_bytes(img_header->unknown_020, sizeof(img_header->unknown_020)));
	printf("cyear         %d\n", img_header->cyear);
	printf("cmonth        %d\n", img_header->cmonth);
	printf("cdate         %d\n", img_header->cdate);
	printf("chour         %d\n", img_header->chour);
	printf("cminute       %d\n", img_header->cminute);
	printf("csecond       %d\n", img_header->csecond);
	printf("fat_offset    %d\n", img_header->fat_offset);
	memcpy(buffer, img_header->identifier, sizeof(img_header->identifier));
	buffer[sizeof(img_header->identifier)] = '\0';
	printf("identifier    \"%s\"\n", buffer);
	printf("unknown_048   %d\n", img_header->unknown_048);
	memcpy(buffer, img_header->desc1, sizeof(img_header->desc1));
	memcpy(buffer + sizeof(img_header->desc1), img_header->desc2, sizeof(img_header->desc2));
	buffer[sizeof(img_header->desc1) + sizeof(img_header->desc2)] = '\0';
	string_trim(buffer, -1);
	printf("desc          \"%s\"\n", buffer);
	printf("heads1        %d\n", img_header->heads1);
	printf("sectors1      %d\n", img_header->sectors1);
	printf("blockexp1     %d\n", img_header->blockexp1);
	printf("blockexp2     %d\n", img_header->blockexp2);
	printf("unknown_063   %d\n", img_header->unknown_063);
	printf("unknown_083   %d\n", img_header->unknown_083);
	printf("unknown_084   %s\n", dump_unknown_bytes(img_header->unknown_084, sizeof(img_header->unknown_084)));
	printf("data_offset   %d\n", img_header->data_offset);
	printf("unknown_410   %s\n", dump_unknown_bytes(img_header->unknown_410, sizeof(img_header->unknown_410)));

	printf("=== SUBMAPS ===\n");
	for (submap = img->submaps; submap != NULL; submap = submap->next) {
		if (submap->tre->isnt) {
			int k;
			printf("%s: NT off=0x%x size=0x%x\n",
					submap->name,
					submap->tre->offset,
					submap->tre->size);
			for (k = 0; k < 5; k ++) {
				if (submap->subfiles[k])
					printf(" %s abs=0x%x rel=0x%x\n",
							get_subtype_name(k),
							(int)((uint8_t *)submap->subfiles[k]->header - img->base),
							(int)((uint8_t *)submap->subfiles[k]->header - submap->subfiles[k]->base));
				else
					printf(" %s NIL\n", get_subtype_name(k));
			}
			if (submap->srt)
				printf(" SRT off=0x%x size=0x%x\n",
						submap->srt->offset,
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
							submap->subfiles[k]->offset,
							submap->subfiles[k]->size);
				else {
					if (k != ST_SRT)
						printf(" %s NIL\n", get_subtype_name(k));
				}
			}
		}
	}
	printf("=== OTHER SUBFILES ===\n");
	for (subfile = img->orphans; subfile != NULL; subfile = subfile->orphan_next) {
		printf("%s: off=0x%x, size=0x%x\n",
				subfile->fullname, subfile->offset, subfile->size);
	}
}

void dump_subfile (struct gimg_struct *img, const char *subfile_name)
{
	if (strlen(subfile_name) >= 5 &&
			strcmp(".GMP", subfile_name + strlen(subfile_name) - 4) == 0) {
		struct submap_struct *submap;
		for (submap = img->submaps; submap != NULL; submap = submap->next) {
			if (memcmp(submap->name, subfile_name, strlen(submap->name)) == 0) {
				dump_comm((struct garmin_subfile *)submap->tre->base);
				return;
			}
		}
	} else {
		struct subfile_struct *subfile;
		for (subfile = img->subfiles; subfile != NULL; subfile = subfile->next) {
			if (strcmp(subfile_name, subfile->fullname) == 0) {
				switch(subfile->typeid) {
					case ST_TRE: dump_tre(subfile); break;
					case ST_TYP: dump_typ(subfile); break;
					case ST_MPS: dump_mps(subfile); break;
					default: dump_comm(subfile->header);
				}
				return;
			}
		}
	}
	printf("subfile %s not found\n", subfile_name);
}

struct gimg_struct *gimg_open (const char *path, int readonly)
{
	struct gimg_struct *img = (struct gimg_struct *)malloc(sizeof(struct gimg_struct));

	img->path = path;
	if (map_img(path, readonly, &img->base, &img->size))
		goto errout;
	if (parse_img(img))
		goto errout;
	return img;

errout:
	free(img);
	return NULL;
}

void gimg_close (struct gimg_struct *img)
{
	unmap_img(img->base, img->size);
	//TODO how to free?
}
