#ifndef GIMGLIB_H
#define GIMGLIB_H

/* It's either VC in Windows or POSIX */
#if defined(_MSC_VER) || defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
# define GT_WINDOWS
# include <stdio.h>
# include <stdlib.h>
# include <assert.h>
# include <string.h>
# include <windows.h>
# include "stdintvc.h"
# define inline __inline
#else
# define GT_POSIX
# include <stdio.h>
# include <stdlib.h>
# include <assert.h>
# include <string.h>
# include <sys/mman.h>
# include <sys/stat.h>
# include <unistd.h>
# include <fcntl.h>
# include <stdint.h>
#endif

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
	unsigned int offset; // same as base, but is abs file offset
	unsigned int size;
	char name[9];
	char type[4];
	char fullname[13]; /* 8.3 */
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

struct gimg_struct {
	const char *path;
	unsigned char *base;
	unsigned int size;
	struct subfile_struct *subfiles;
	struct submap_struct *submaps;
	struct subfile_struct *orphans; // files not belonging to any submap
};

static inline unsigned int bytes_to_uint24 (unsigned char *bytes) {
	return (*(unsigned int *)bytes) & 0x00ffffff;
}
static inline int bytes_to_sint24 (unsigned char *bytes) {
	int n = (*(int *)bytes) & 0x00ffffff;
	return (n < 0x00800000) ? n : (n | 0xff000000);
}

/* util.c */
const char *sint24_to_lat (int n);
const char *sint24_to_lng (int n);
const char *dump_unknown_bytes (uint8_t *bytes, int size);
void unlockml (unsigned char *dst, const unsigned char *src, int size, unsigned int key);
enum subtype get_subtype_id (const char *str); // only use 3 chars from str
const char *get_subtype_name (enum subtype id);
void string_trim (char *str, int length);

/* sf_typ.c */
void dump_typ (struct subfile_struct *typ);

/* sf_tre.c */
void dump_tre (struct subfile_struct *tre);

/* sf_mps.c */
void dump_mps (struct subfile_struct *mps);

/* gimglib.c */
void dump_comm (struct garmin_subfile *header);
void dump_img (struct gimg_struct *img);
void dump_subfile (struct gimg_struct *img, const char *subfile_name);
struct gimg_struct *gimg_open (const char *path, int readonly);
void gimg_close (struct gimg_struct *img);

#endif
