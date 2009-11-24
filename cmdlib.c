#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "cmdlib.h"

int cmd_spd; /* sample per degree */
int cmd_x_num, cmd_y_num;
short *cmd_devs = NULL;

int cmd_init (const char *path)
{
	FILE *fp = fopen(path, "rb");
	if (fopen == NULL) {
		printf("cannot open %s\n", path);
		goto errout;
	}

	cmd_spd = 0;
	if (fread(&cmd_spd, 4, 1, fp) != 1) {
		printf("%s is invalid cmd DB\n", path);
		goto errout;
	}
	if (cmd_spd < 0 || cmd_spd > 100) {
		printf("%s has invalid sample per degree\n", path);
		goto errout;
	}
	cmd_x_num = (CMD_X1 - CMD_X0) * cmd_spd;
	cmd_y_num = (CMD_Y1 - CMD_Y0) * cmd_spd;
	cmd_devs = (short *)malloc(cmd_x_num * cmd_y_num * 4);
	if (fread(cmd_devs, cmd_x_num * cmd_y_num * 4, 1, fp) != 1) {
		printf("%s is truncated\n", path);
		goto errout;
	}
	fclose(fp);
	return 0;

errout:
	if (cmd_devs)
		free(cmd_devs);
	cmd_devs = NULL;
	if (fp)
		fclose(fp);
	return 1;
}

void cmd_fini (void)
{
	if (cmd_devs != NULL)
		free(cmd_devs);
	cmd_devs = NULL;
}

/* x, y are in sample unit.
 * returned value is in deviation unit. */
static inline double cmd_get_dx (double x, double y)
{
	int ix = (int)x;
	int iy = (int)y;

	if (ix == 0) x = ix = 0;
	else if (ix >= cmd_x_num - 1) x = ix = cmd_x_num - 1;
	if (iy == 0) y = iy = 0;
	else if (iy >= cmd_y_num - 1) y = iy = cmd_y_num - 1;

	return DEVX(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
		DEVX(ix+1,iy) * (x - ix) * (iy + 1 - y) +
		DEVX(ix,iy+1) * (ix + 1 - x) * (y - iy) +
		DEVX(ix+1,iy+1) * (x - ix) * (y - iy);
}
static inline double cmd_get_dy (double x, double y)
{
	int ix = (int)x;
	int iy = (int)y;

	if (ix == 0) x = ix = 0;
	else if (ix >= cmd_x_num - 1) x = ix = 0;
	if (iy == 0) y = iy = 0;
	else if (iy >= cmd_y_num - 1) y = iy = 0;

	return DEVY(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
		DEVY(ix+1,iy) * (x - ix) * (iy + 1 - y) +
		DEVY(ix,iy+1) * (ix + 1 - x) * (y - iy) +
		DEVY(ix+1,iy+1) * (x - ix) * (y - iy);
}

void cmd_g24p_fix (int *px, int *py)
{
	double x, y;

	x = (*px * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y = (*py * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	*px -= (int)lround(cmd_get_dx(x, y) * 0x1000000 / 360 / CMD_DUPD);
	*py -= (int)lround(cmd_get_dy(x, y) * 0x1000000 / 360 / CMD_DUPD);
}

void cmd_g24p_dev (int *px, int *py)
{
	assert(0);
}

/* x, y0, y1 are all in sample unit.
 * returned value is in deviation unit.
 * swapxy: I'm too lazy to implement two versions: get_avg_dx and get_avg_dy
 * swapxy = 0 means get_avg_dx(x, y0, y1)
 * swapxy = 1 means get_avg_dy(y, x0, x1) */
static double get_avg_dx (double x, double y0, double y1, int swapxy)
{
	int n, i, ix, iy0, iy1;
	int my_x_num = swapxy ? cmd_y_num : cmd_x_num;
	int my_y_num = swapxy ? cmd_x_num : cmd_y_num;
	double sum;

	assert(y0 <= y1);

	ix = (int)x;
	if (ix < 0) {
		ix = 0;
		x = 0;
	} else if (ix >= my_x_num - 1) {
		ix = my_x_num - 1;
		x = my_x_num - 1;
	}
	iy0 = lround(y0);
	iy1 = lround(y1);
	if (iy0 < 0) iy0 = 0;
	if (iy1 >= my_y_num) iy1 = my_y_num - 1;

	n = iy1 - iy0;
	if (n <= 1)
		return swapxy ? cmd_get_dy((y0 + y1) / 2, x) : cmd_get_dx(x, (y0 + y1) / 2);
	if (n > 10) n = 10; /* don't want too much computation */
	for (sum = 0.0, i = 0; i <= n; i ++) {
		int iy = (i * (iy1 - iy0) + n * iy0) / n;
		if (swapxy)
			sum += DEVY(iy,ix) * (ix + 1 - x) + DEVY(iy,ix+1) * (x - ix);
		else
			sum += DEVX(ix,iy) * (ix + 1 - x) + DEVX(ix+1,iy) * (x - ix);
	}
	return sum / (n + 1);
}

/* assume x0 <= x1 && y0 <= y1 */
void cmd_g24r_fix (int *px0, int *py0, int *px1, int *py1)
{
	double x0, y0, x1, y1;

	x0 = (*px0 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	x1 = (*px1 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y0 = (*py0 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;
	y1 = (*py1 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	assert(x0 <= x1 && y0 <= y1);

	*px0 -= (int)lround(get_avg_dx(x0, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
	*px1 -= (int)lround(get_avg_dx(x1, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
	*py0 -= (int)lround(get_avg_dx(y0, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
	*py1 -= (int)lround(get_avg_dx(y1, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
}

void cmd_g24r_dev (int *px0, int *py0, int *px1, int *py1)
{
	assert(0);
}
