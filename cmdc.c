/* example input:
 * 18.300000 109.600000 736 367
 * 18.300000 109.650000 746 358
 * 18.300000 109.700000 765 340
 * 18.300000 109.750000 775 330
 * output is binary
 * */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

/* CMD_DUPD deviation unit per degree:
 * Since the max deviation observed is 0.0085509,
 * 0.0085509 * 2500000  = 21377.25 fits will under signed short. */
#define CMD_DUPD 2500000
#define CMD_X0 72
#define CMD_X1 135
#define CMD_Y0 18
#define CMD_Y1 54


int main (int argc, char *argv[])
{
	int spd; /* sample per degree */
	short *devs;
	unsigned char *assign_round;
	int x_num, y_num;
	int loaded = 0, interpolated = 0;

	if (argc != 2 || (spd = atoi(argv[1])) == 0) {
		fprintf(stderr, "Usage: %s <sample per degree>\n", argv[0]);
		fprintf(stderr, "Example: %s 20 <in.txt >out.txt\n", argv[0]);
		return 1;
	}
	if (spd <= 0 || spd > 100) {
		fprintf(stderr, "wrong spd %d\n", spd);
		return 1;
	}

	y_num = (CMD_Y1 - CMD_Y0) * spd;
	x_num = (CMD_X1 - CMD_X0) * spd;
	devs = malloc(y_num * x_num * 2 * sizeof(short));
	memset(devs, 0, y_num * x_num * 2 * sizeof(short));
	assign_round = malloc(y_num * x_num);
	memset(assign_round, 0, y_num * x_num);
	fprintf(stderr, "spd=%d, %d (EW) x %d (NS) = %d samples\n",
			spd, x_num, y_num, x_num * y_num);

	while (!feof(stdin)) {
		double x, y, dx, dy;
		int ix, iy, idx, idy;

		if (scanf("%lf\t%lf\t%d\t%d\n", &y, &x, &idx, &idy) != 4)
			break;
		dx = idx / ((1 << 26) / 360.0);
		dy = -idy * cos(y * (M_PI / 180)) / ((1 << 26) / 360.0);
		if (fabs(dx * CMD_DUPD) >= 32767 || fabs(dy * CMD_DUPD) >= 32767) {
			fprintf(stderr, "error: offset too large: %f %f %.7f %.7f\n", x, y, dx, dy);
			return 1;
		}
		ix = (int)round((x - CMD_X0) * spd);
		iy = (int)round((y - CMD_Y0) * spd);
		if (fabs((x - CMD_X0) * spd - ix) > 0.1 || fabs((y - CMD_Y0) * spd - iy) > 0.1) {
			fprintf(stderr, "warning: misaligned %f %f %.2f %.2f\n", x, y, (x - CMD_X0) * spd - ix, (y - CMD_X0) * spd - iy);
			continue;
		}
		if (ix < 0 || ix >= x_num || iy < 0 || iy >= y_num) {
			fprintf(stderr, "warning: OOB %f %f %d %d\n", x, y, ix, iy);
			continue;
		}
		devs[iy * x_num * 2 + ix * 2 + 0] = idx;
		devs[iy * x_num * 2 + ix * 2 + 1] = idy;
		assign_round[iy * x_num + ix] = 2;
		loaded ++;
	}
	fprintf(stderr, "%d samples loaded\n", loaded);

	/* apply interpolation */
	while (1) {
		int x, y, this_itpd = 0;

		for (y = 0; y < y_num; y ++) for (x = 0; x < x_num; x ++) {
			int sumdx = 0, sumdy = 0, n = 0;
			if (assign_round[y * x_num + x] != 0)
				continue;
			if (x > 0         && assign_round[y * x_num + (x - 1)] == 2) {
				sumdx += devs[y * x_num * 2 + (x - 1) * 2 + 0];
				sumdy += devs[y * x_num * 2 + (x - 1) * 2 + 1];
				n ++;
			}
			if (x < x_num - 1 && assign_round[y * x_num + (x + 1)] == 2) {
				sumdx += devs[y * x_num * 2 + (x + 1) * 2 + 0];
				sumdy += devs[y * x_num * 2 + (x + 1) * 2 + 1];
				n ++;
			}
			if (y > 0         && assign_round[(y - 1) * x_num + x] == 2) {
				sumdx += devs[(y - 1) * x_num * 2 + x * 2 + 0];
				sumdy += devs[(y - 1) * x_num * 2 + x * 2 + 1];
				n ++;
			}
			if (y < y_num - 1 && assign_round[(y + 1) * x_num + x] == 2) {
				sumdx += devs[(y + 1) * x_num * 2 + x * 2 + 0];
				sumdy += devs[(y + 1) * x_num * 2 + x * 2 + 1];
				n ++;
			}
			if (n == 0)
				continue;
			devs[y * x_num * 2 + x * 2 + 0] = sumdx / n;
			devs[y * x_num * 2 + x * 2 + 1] = sumdy / n;
			assign_round[y * x_num + x] = 1;
			interpolated ++;
			this_itpd ++;
		}
		if (this_itpd == 0)
			break;
		for (y = 0; y < y_num; y ++) for (x = 0; x < x_num; x ++) {
			if (assign_round[y * x_num + (x - 1)] == 1)
				assign_round[y * x_num + (x - 1)] = 2;
		}
	}
	fprintf(stderr, "%d samples interpolated\n", interpolated);

	fwrite(&spd, 4, 1, stdout);
	fwrite(devs, y_num * x_num * 2 * sizeof(short), 1, stdout);

	return 0;
}
