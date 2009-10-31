#ifndef GIMGINFO_H
#define GIMGINFO_H

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

extern int option_verbose;
extern const char *option_img_path;
extern const char *option_subfile;
extern unsigned char *img_base;
extern unsigned int img_size;
extern struct subfile_struct *subfiles;
extern struct submap_struct *submaps;
extern struct subfile_struct *orphans; // files not belonging to any submap
extern int subfile_num;
extern int block_size;

#define vlog(...) if (option_verbose) fprintf(stderr, "LOG: " __VA_ARGS__)
#define warn(...) fprintf(stderr, "WARNING: " __VA_ARGS__)

static inline unsigned int bytes_to_uint24 (unsigned char *bytes) {
	return (*(unsigned int *)bytes) & 0x00ffffff;
}
static inline int bytes_to_sint24 (unsigned char *bytes) {
	int n = (*(int *)bytes) & 0x00ffffff;
	return (n < 0x00800000) ? n : (n | 0xff000000);
}

/* util.c */
const char *dump_unknown_bytes (uint8_t *bytes, int size);
enum subtype get_subtype_id (const char *str); // only use 3 chars from str
const char *get_subtype_name (enum subtype id);
void string_trim (char *str, int length);

/* sf_tre.c */
void dump_tre (struct subfile_struct *tre);

/* gimginfo.c */
void dump_comm (struct garmin_subfile *header);

#endif
