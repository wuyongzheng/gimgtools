#ifndef UTILS_H
#define UTILS_H

#define errexit(...) do {printf(__VA_ARGS__); exit(1);} while (0)

#if defined(_MSC_VER) || defined(WIN32) || defined(WIN64) || defined(_WIN32) || defined(_WIN64)
typedef __int64 off_t;
#define myfseek64(fp,pos) _fseeki64(fp, pos, SEEK_SET)
#else
#define myfseek64(fp,pos) fseeko(fp, pos, SEEK_SET)
#endif

void hexdump (const unsigned char *data, int size);
unsigned int read_byte_at (FILE *fp, off_t offset);
unsigned int read_2byte_at (FILE *fp, off_t offset);
unsigned int read_4byte_at (FILE *fp, off_t offset);
void read_bytes_at (FILE *fp, off_t offset, unsigned char *buffer, int size);

#endif
