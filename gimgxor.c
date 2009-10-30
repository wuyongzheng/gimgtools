#include <stdio.h>

int main(int argc, char *argv[])
{
	FILE *inf, *outf;
	unsigned char buffer[4096];
	int buffer_size, ptr;
	int xorbyte;

	if (argc != 3) {
		printf("usage: %s infile outfile", argv[0]);
		return 1;
	}

	inf = fopen(argv[1], "rb");
	if (inf == NULL) {
		printf("can't open %s\n", argv[1]);
		return 1;
	}

	buffer_size = fread(buffer, 1, sizeof(buffer), inf);
	if (buffer_size == 0) {
		printf("empty file\n");
		return 1;
	}

	xorbyte = buffer[0];
	if (xorbyte == 0) {
		printf("file is not XORed\n");
		return 1;
	}
	if (buffer_size <= 600 ||
			(buffer[0x10] ^ xorbyte) != 'D' ||
			(buffer[0x15] ^ xorbyte) != 'G' ||
			(buffer[0x41] ^ xorbyte) != 'G' ||
			(buffer[0x46] ^ xorbyte) != 'N') {
		printf("file is not garmin img\n");
		return 1;
	}

	outf = fopen(argv[2], "wb");
	if (outf == NULL) {
		printf("can't create/write %s\n", argv[2]);
		return 1;
	}

	for (ptr = 0; ptr < buffer_size; ptr ++)
		buffer[ptr] ^= xorbyte;
	fwrite(buffer, 1, buffer_size, outf);

	while (!feof(inf)) {
		buffer_size = fread(buffer, 1, sizeof(buffer), inf);
		if (buffer_size == 0)
			break;
		for (ptr = 0; ptr < buffer_size; ptr ++)
			buffer[ptr] ^= xorbyte;
		fwrite(buffer, 1, buffer_size, outf);
	}

	fclose(inf);
	fclose(outf);
	return 0;
}
