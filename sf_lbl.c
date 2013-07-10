#include "gimglib.h"
#include <iconv.h>

// 6-bit decoder cpoied from http://libgarmin.sourceforge.net/
static const char str6tbl1[] = {
	' ','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R'
	,'S','T','U','V','W','X','Y','Z'
	,0,0,0,0,0
	,'0','1','2','3','4','5','6','7','8','9'
	,0,0,0,0,0,0
};

static const char str6tbl2[] = {
	//@   !   "   #   $   %   &    '   (   )   *   +   ,   -   .   /
	'@','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/'
	,0,0,0,0,0,0,0,0,0,0
	//:   ;   <   =   >   ?
	,':',';','<','=','>','?'
	,0,0,0,0,0,0,0,0,0,0,0
	//[    \   ]   ^   _
	,'[','\\',']','^','_'
};


static const char str6tbl3[] = {
    '`','a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r',
's','t','u','v','w','x','y','z'
};

struct bitstream_struct {
	unsigned char *next;
	unsigned int buff;
	int bufflen;
};

static void bitstream_init (struct bitstream_struct *bi, unsigned char *ptr)
{
	bi->next = ptr;
	bi->buff = 0;
	bi->bufflen = 0;
}

static unsigned int bitstream_getint (struct bitstream_struct *bi, int bits)
{
	unsigned int retval;

	assert(bits > 0 && bits < 24);
	while (bi->bufflen < bits) {
		bi->buff = (bi->buff << 8) | *(bi->next);
		bi->bufflen += 8;
		bi->next ++;
	}
	retval = bi->buff >> (bi->bufflen - bits);
	bi->buff &= (1 << (bi->bufflen - bits)) - 1;
	bi->bufflen -= bits;
	return retval;
}

// hex dump
static void lbl_decoden (unsigned char *code, int bits, int count, char *out)
{
	struct bitstream_struct bi;

	bitstream_init(&bi, code);
	while (count -- > 0) {
		out += sprintf(out, "%02x", bitstream_getint(&bi, bits));
		if (count != 0)
			out += sprintf(out, "-");
	}
}


static int lbl_decode6 (unsigned char *code, char *out, ssize_t outlen)
{
	unsigned char ch;
	int sz = 0;
	struct bitstream_struct bi;

	bitstream_init(&bi, code);
	while (1) {
		int c = bitstream_getint(&bi, 6);
		//printf("got %d(0x%x)\n", c, c);
		if (c > 0x2f)
			break;
		ch = str6tbl1[c];
		if (ch == 0) {
			if (c == 0x1C) {
				c = bitstream_getint(&bi, 6);
				if (c < 0)
					break;
				ch = str6tbl2[c];
				*out++ = ch;
				sz++;
			} else if (c == 0x1B) {
				c = bitstream_getint(&bi, 6);
				if (c < 0)
					break;
				ch = str6tbl3[c];
				*out++ = ch;
				sz++;
			} else if (c == 0x1D) {	// delimiter for formal name
				*out++ = '|';
				sz++;
			} else if (c == 0x1E) {
				*out++ = '_';	// hide previous symbols
				sz++;
			} else if (c == 0x1F) {
				*out++ = '^';	// hide next symbols
				sz++;
			} else if (c >= 0x20 && c <= 0x2F ) {
				*out++ = '@';
				sz++;
			}
		} else {
			*out++ = ch;
			sz++;
		}
		if (sz >= outlen-1)
			break;
	}
	*out = '\0';
	return sz;
}

static int lbl_decode8x (unsigned char *code, int codepage, char *out, ssize_t outlen)
{
	if (codepage >= 37 && codepage <= 16804) {
		char cpstr[10];
		iconv_t cd;
		char *inbuf = (char *)code, *outbuf = out;
		size_t inbytesleft = outlen, outbytesleft = outlen;

		sprintf(cpstr, "CP%d", codepage);
		cd = iconv_open("UTF-8", cpstr); //TODO: use setlocal to determine console encoding
		assert(cd != (iconv_t)-1);
		iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft); //FIXME: this converts outlen bytes regardless of null-terminator. outlen is large. performance prob.
		outbuf[0] = '\0';
		iconv_close(cd);

		for (outbuf = out; *outbuf; outbuf ++) {
			if (*outbuf == '\x1d')
				*outbuf = '|';
			else if (*outbuf < ' ' && *outbuf > 0)
				*outbuf = '.';
		}
		return strlen(out);
	} else {
		int i;
		for (i = 0; i < outlen - 1 && code[i]; i ++) {
			if (code[i] > ' ' && code[i] <= '~')
				out[i] = code[i];
			else
				out[i] = '.';
		}
		out[i] = '\0';
		return i;
	}
}

static const char *lbl_decode_static (unsigned char *code, int bits, int codepage)
{
	static char buf[4][512];
	static unsigned int bufid = 0;
	int currid = (bufid ++) % 4;

	buf[currid][0] = '\0';
	switch (bits) {
		//case 11: lbl_decoden(code, 11, 16, buf[currid]); break;
		case 6: lbl_decode6(code, buf[currid], sizeof(buf[0])); break;
		case 8:
		case 9:
		case 10: lbl_decode8x(code, codepage, buf[currid], sizeof(buf[0])); break;
		default: return dump_unknown_bytes(code, 16);
		//default: lbl_decoden(code, bits, 10, buf[currid]);
	}
	return buf[currid];
}

const char *lbl_lbl_static (struct subfile_struct *lbl, int lbl_index)
{
	struct garmin_lbl *header = (struct garmin_lbl *)lbl->header;
	return lbl_decode_static(lbl->base + header->lbl1_offset +
			lbl_index * (1 << header->addr_shift),
			header->coding, header->codepage);
}

// return number of bytes parsed.
int dump_poi (struct subfile_struct *lbl, int poi_index)
{
	struct garmin_lbl *header = (struct garmin_lbl *)lbl->header;
	unsigned char *lbl6_poi = lbl->base + header->lbl6_offset;
	int offset = poi_index * (1 << header->lbl6_addr_shift);
	int poi_data = lbl6_poi[offset] + (lbl6_poi[offset+1] << 8) + (lbl6_poi[offset+2] << 16);
	int local_mask = header->lbl6_glob_mask;

	printf("poi %d: %s, %s\n", poi_index, lbl_lbl_static(lbl, poi_data & 0x3fffff), dump_unknown_bytes(lbl6_poi + offset, 16));

	offset += 3;
	if (poi_data >> 23) {
		int i, j;
		for (i = j = 0; i <= 7; i ++) {
			if (local_mask & (1 << i)) {
				if (!(lbl6_poi[offset] & (1 << j)))
					local_mask -= 1 << i;
				j ++;
			}
		}
		offset ++;
	}
	poi_data &= (1 << 22) - 1;

	printf("localmask: %x\n", local_mask);
	if (local_mask & 1) { // street_number
		if (lbl6_poi[offset] & 0x80) {
			char buf[32];
			char *ptr = buf;
			int i;
			for (i = 0; ; i++) {
				int n = lbl6_poi[offset+i] & ~ 0x80;
				int n1 = n / 11;
				int n2 = n % 11;
				if (n2 == 10)
					ptr += sprintf(ptr, "%d", n1);
				else
					ptr += sprintf(ptr, "%d%d", n1, n2);
				if (i != 0 && lbl6_poi[offset+i] & 0x80)
					break;
				assert(buf + sizeof(buf) - ptr > 3);
			}
			printf("property_mask: %s\n", buf);
			offset += i;
		} else {
			int lbl_off = lbl6_poi[offset] + (lbl6_poi[offset+1] << 8) + (lbl6_poi[offset+2] << 16);
			printf("property_mask: 0x%x\n", lbl_off);
			offset += 3;
		}
	}

	// TODO
	return offset - poi_index * (1 << header->lbl6_addr_shift);
}

void dump_lbl (struct subfile_struct *sf)
{
	struct garmin_lbl *header = (struct garmin_lbl *)sf->header;
	int progress = 0;
	unsigned char *lbl1_data = NULL;
	unsigned char *lbl2_country = NULL;
	unsigned char *lbl3_region = NULL;
	unsigned char *lbl4_city = NULL;
	unsigned char *lbl6_poi = NULL;

	assert(sf->typeid == ST_LBL);

	dump_comm(sf->header);

	printf("=== LBL HEADER ===\n");
	if (header->comm.hlen < 0xaa)
		goto headerfini;
	progress = 0xaa;
	printf("LBL1 Data:          reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl1_offset, sf->offset + header->lbl1_offset, header->lbl1_length);
	if (header->lbl1_length)
		lbl1_data = sf->base + header->lbl1_offset;
	printf("Shift, Coding:      %d, %d\n",
			header->addr_shift, header->coding);
	printf("LBL2 Country:       reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?029=0x%x\n",
			header->lbl2_offset, sf->offset + header->lbl2_offset,
			header->lbl2_length, header->lbl2_recsize, header->lbl2_u);
	if (header->lbl2_length)
		lbl2_country = sf->base + header->lbl2_offset;
	printf("LBL3 Region:        reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?037=0x%x\n",
			header->lbl3_offset, sf->offset + header->lbl3_offset,
			header->lbl3_length, header->lbl3_recsize, header->lbl3_u);
	if (header->lbl3_length)
		lbl3_region = sf->base + header->lbl3_offset;
	printf("LBL4 City:          reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?045=0x%x\n",
			header->lbl4_offset, sf->offset + header->lbl4_offset,
			header->lbl4_length, header->lbl4_recsize, header->lbl4_u);
	if (header->lbl4_length)
		lbl4_city = sf->base + header->lbl4_offset;
	printf("LBL5 POI:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?053=0x%x\n",
			header->lbl5_offset, sf->offset + header->lbl5_offset,
			header->lbl5_length, header->lbl5_recsize, header->lbl5_u);
	printf("LBL6 POI Prop:      reloff=0x%x absoff=0x%x size=0x%x shift=%d mask=0x%x ?061=0x%02x%02x%02x\n",
			header->lbl6_offset, sf->offset + header->lbl6_offset, header->lbl6_length,
			header->lbl6_addr_shift, header->lbl6_glob_mask,
			header->lbl6_u[0], header->lbl6_u[1], header->lbl6_u[2]);
	if (header->lbl6_length)
		lbl6_poi = sf->base + header->lbl6_offset;
	printf("LBL7 POI Type:      reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?06e=0x%x\n",
			header->lbl7_offset, sf->offset + header->lbl7_offset,
			header->lbl7_length, header->lbl7_recsize, header->lbl7_u);
	printf("LBL8 Zip:           reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?07c=0x%x\n",
			header->lbl8_offset, sf->offset + header->lbl8_offset,
			header->lbl8_length, header->lbl8_recsize, header->lbl8_u);
	printf("LBL9 Highway:       reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?08a=0x%x\n",
			header->lbl9_offset, sf->offset + header->lbl9_offset,
			header->lbl9_length, header->lbl9_recsize, header->lbl9_u);
	printf("LBL10 Exit:         reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?098=0x%x\n",
			header->lbl10_offset, sf->offset + header->lbl10_offset,
			header->lbl10_length, header->lbl10_recsize, header->lbl10_u);
	printf("LBL11 Highway Data: reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?0a6=0x%x\n",
			header->lbl11_offset, sf->offset + header->lbl11_offset,
			header->lbl11_length, header->lbl11_recsize, header->lbl11_u);

	if (header->comm.hlen < 0xb0)
		goto headerfini;
	progress = 0xb0;
	if (header->codepage >= 1250 && header->codepage <= 1258)
		printf("Codepage:           Windows-%d\n", header->codepage);
	else if (header->codepage == 950)
		printf("Codepage:           Big5\n");
	else if (header->codepage == 936)
		printf("Codepage:           GBK\n");
	else
		printf("Codepage:           %d\n", header->codepage);
	printf("Codepage2:          0x%d\n", header->codepage2);
	printf("Codepage3:          0x%x\n", header->codepage3);

	if (header->comm.hlen < 0xb8)
		goto headerfini;
	progress = 0xb8;
	printf("LBL12:              reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl12_offset, sf->offset + header->lbl12_offset, header->lbl12_length);
	if (header->comm.hlen < 0xc4)
		goto headerfini;
	progress = 0xc4;
	printf("LBL13:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl13_offset, sf->offset + header->lbl13_offset, header->lbl13_length,
			header->lbl13_recsize, header->lbl13_u);
	if (header->comm.hlen < 0xd0)
		goto headerfini;
	progress = 0xd0;
	printf("LBL14:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl14_offset, sf->offset + header->lbl14_offset, header->lbl14_length,
			header->lbl14_recsize, header->lbl14_u);
	if (header->comm.hlen < 0xde)
		goto headerfini;
	progress = 0xde;
	printf("LBL15:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl15_offset, sf->offset + header->lbl15_offset, header->lbl15_length,
			header->lbl15_recsize, header->lbl15_u);
	if (header->comm.hlen < 0xec)
		goto headerfini;
	progress = 0xec;
	printf("LBL16:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl16_offset, sf->offset + header->lbl16_offset, header->lbl16_length,
			header->lbl16_recsize, header->lbl16_u);
	if (header->comm.hlen < 0xfa)
		goto headerfini;
	progress = 0xfa;
	printf("LBL17:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl17_offset, sf->offset + header->lbl17_offset, header->lbl17_length,
			header->lbl17_recsize, header->lbl17_u);
	if (header->comm.hlen < 0x108)
		goto headerfini;
	progress = 0x108;
	printf("LBL18:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl18_offset, sf->offset + header->lbl18_offset, header->lbl18_length,
			header->lbl18_recsize, header->lbl18_u);
	if (header->comm.hlen < 0x116)
		goto headerfini;
	progress = 0x116;
	printf("LBL19:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl19_offset, sf->offset + header->lbl19_offset, header->lbl19_length,
			header->lbl19_recsize, header->lbl19_u);
	if (header->comm.hlen < 0x124)
		goto headerfini;
	progress = 0x124;
	printf("LBL20:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl20_offset, sf->offset + header->lbl20_offset, header->lbl20_length,
			header->lbl20_recsize, header->lbl20_u);
	if (header->comm.hlen < 0x132)
		goto headerfini;
	progress = 0x132;
	printf("LBL21:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl21_offset, sf->offset + header->lbl21_offset, header->lbl21_length,
			header->lbl21_recsize, header->lbl21_u);
	if (header->comm.hlen < 0x140)
		goto headerfini;
	progress = 0x140;
	printf("LBL22:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl22_offset, sf->offset + header->lbl22_offset, header->lbl22_length,
			header->lbl22_recsize, header->lbl22_u);
	if (header->comm.hlen < 0x14e)
		goto headerfini;
	progress = 0x14e;
	printf("LBL23:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl23_offset, sf->offset + header->lbl23_offset, header->lbl23_length,
			header->lbl23_recsize, header->lbl23_u);
	if (header->comm.hlen < 0x15a)
		goto headerfini;
	progress = 0x15a;
	printf("LBL24:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl24_offset, sf->offset + header->lbl24_offset, header->lbl24_length,
			header->lbl24_recsize, header->lbl24_u);
	if (header->comm.hlen < 0x168)
		goto headerfini;
	progress = 0x168;
	printf("LBL25:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl25_offset, sf->offset + header->lbl25_offset, header->lbl25_length,
			header->lbl25_recsize, header->lbl25_u);
	if (header->comm.hlen < 0x176)
		goto headerfini;
	progress = 0x176;
	printf("LBL26:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl26_offset, sf->offset + header->lbl26_offset, header->lbl26_length,
			header->lbl26_recsize, header->lbl26_u);
	if (header->comm.hlen < 0x184)
		goto headerfini;
	progress = 0x184;
	printf("LBL27:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl27_offset, sf->offset + header->lbl27_offset, header->lbl27_length,
			header->lbl27_recsize, header->lbl27_u);
	if (header->comm.hlen < 0x192)
		goto headerfini;
	progress = 0x192;
	printf("LBL28:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl28_offset, sf->offset + header->lbl28_offset, header->lbl28_length,
			header->lbl28_recsize, header->lbl28_u);
	if (header->comm.hlen < 0x19a)
		goto headerfini;
	progress = 0x19a;
	printf("LBL29:              reloff=0x%x absoff=0x%x size=0x%x\n",
			header->lbl29_offset, sf->offset + header->lbl29_offset, header->lbl29_length);
	if (header->comm.hlen < 0x1a6)
		goto headerfini;
	progress = 0x1a6;
	printf("LBL30:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl30_offset, sf->offset + header->lbl30_offset, header->lbl30_length,
			header->lbl30_recsize, header->lbl30_u);
	if (header->comm.hlen < 0x1b2)
		goto headerfini;
	progress = 0x1b2;
	printf("LBL31:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl31_offset, sf->offset + header->lbl31_offset, header->lbl31_length,
			header->lbl31_recsize, header->lbl31_u);
	if (header->comm.hlen < 0x1be)
		goto headerfini;
	progress = 0x1be;
	printf("LBL32:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl32_offset, sf->offset + header->lbl32_offset, header->lbl32_length,
			header->lbl32_recsize, header->lbl32_u);
	if (header->comm.hlen < 0x1ca)
		goto headerfini;
	progress = 0x1ca;
	printf("LBL33:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl33_offset, sf->offset + header->lbl33_offset, header->lbl33_length,
			header->lbl33_recsize, header->lbl33_u);
	if (header->comm.hlen < 0x1d8)
		goto headerfini;
	progress = 0x1d8;
	printf("LBL34:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl34_offset, sf->offset + header->lbl34_offset, header->lbl34_length,
			header->lbl34_recsize, header->lbl34_u);
	if (header->comm.hlen < 0x1e6)
		goto headerfini;
	progress = 0x1e6;
	printf("LBL35:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl35_offset, sf->offset + header->lbl35_offset, header->lbl35_length,
			header->lbl35_recsize, header->lbl35_u);
	if (header->comm.hlen < 0x1f2)
		goto headerfini;
	progress = 0x1f2;
	printf("LBL36:              reloff=0x%x absoff=0x%x size=0x%x recsize=0x%x ?=0x%x\n",
			header->lbl36_offset, sf->offset + header->lbl36_offset, header->lbl36_length,
			header->lbl36_recsize, header->lbl36_u);

headerfini:
	if (header->comm.hlen > progress)
		printf("from 0x%x to 0x%x (0x%x bytes): %s\n",
				progress, header->comm.hlen - 1,
				header->comm.hlen - progress,
				dump_unknown_bytes((uint8_t *)header + progress, header->comm.hlen - progress));

	if (lbl2_country) {
		int offset, id;
		printf("=== COUNTRY ===\n");
		for (offset = 0, id = 1; offset < header->lbl2_length; offset += header->lbl2_recsize, id ++) {
			int dataoff = lbl2_country[offset] + (lbl2_country[offset+1] << 8) + (lbl2_country[offset+2] << 16);
			printf("%d: \"%s\", \"%s\"\n", id,
					lbl_decode_static(lbl1_data + (dataoff << header->addr_shift), header->coding, header->codepage),
					//dump_unknown_bytes(lbl1_data + (dataoff << header->addr_shift), 32),
					dump_unknown_bytes(lbl2_country + offset + 3, header->lbl2_recsize - 3));
		}
	}

	if (lbl3_region) {
		int offset, id;
		printf("=== REGION ===\n");
		for (offset = 0, id = 1; offset < header->lbl3_length; offset += header->lbl3_recsize, id ++) {
			int dataoff = lbl3_region[offset+2] + (lbl3_region[offset+3] << 8) + (lbl3_region[offset+4] << 16);
			printf("%d: country=%d, \"%s\", \"%s\"\n", id,
					(lbl3_region[offset] + (lbl3_region[offset+1] << 8)),
					lbl_decode_static(lbl1_data + (dataoff << header->addr_shift), header->coding, header->codepage),
					//dump_unknown_bytes(lbl1_data + (dataoff << header->addr_shift), 32),
					dump_unknown_bytes(lbl3_region + offset + 5, header->lbl3_recsize - 5));
		}
	}

	if (lbl4_city) {
		int offset, id;
		printf("=== CITY ===\n");
		for (offset = 0, id = 1; offset < header->lbl4_length; offset += header->lbl4_recsize, id ++) {
			int region = lbl4_city[offset+3] + (lbl4_city[offset+4] << 8);
			int pointref = region >> 15;
			region &= (1 << 14) - 1;
			if (pointref) {
				printf("%d: region=%d, point_index=%d, subdiv_index=%d, \"%s\"\n", id, region,
						lbl4_city[offset],
						lbl4_city[offset+1] + (lbl4_city[offset+2] << 8),
						dump_unknown_bytes(lbl4_city + offset + 5, header->lbl4_recsize - 5));
			} else {
				int dataoff = lbl4_city[offset] + (lbl4_city[offset+1] << 8) + (lbl4_city[offset+2] << 16);
				printf("%d: region=%d, \"%s\", \"%s\"\n", id, region,
						lbl_decode_static(lbl1_data + (dataoff << header->addr_shift), header->coding, header->codepage),
						//dump_unknown_bytes(lbl1_data + (dataoff << header->addr_shift), 32),
						dump_unknown_bytes(lbl4_city + offset + 5, header->lbl4_recsize - 5));
			}
		}
	}

/*	if (lbl6_poi) {
		int index;
		const int indexmul = 1 << header->lbl6_addr_shift;
		printf("=== PoI ===\n");
		for (index = 0; index < (header->lbl6_length - 3) / indexmul; ) {
			int poi_size = dump_poi(sf, index);
			if (poi_size <= 0) {
				printf("poi parse error\n");
				break;
			}
			index += (poi_size - 1) / indexmul + 1;
		}
	}
*/
	return;
}
