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
	if (fp == NULL) {
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

/* cmd_point_dev(x, y, 0): get deviation x of a point (x,y)
 * cmd_point_dev(x, y, 1): get deviation y of a point (x,y)
 * x, y are in sample unit.
 * returned value is in deviation unit. */
static inline double cmd_point_dev (double x, double y, int isdy)
{
	int ix = (int)x;
	int iy = (int)y;

	if (x <= 0) x = ix = 0;
	else if (ix > cmd_x_num - 2) x = ix = cmd_x_num - 2;
	if (y <= 0) y = iy = 0;
	else if (iy > cmd_y_num - 2) y = iy = cmd_y_num - 2;

	if (isdy) {
		return DEVY(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
			DEVY(ix+1,iy) * (x - ix) * (iy + 1 - y) +
			DEVY(ix,iy+1) * (ix + 1 - x) * (y - iy) +
			DEVY(ix+1,iy+1) * (x - ix) * (y - iy);
	} else {
		return DEVX(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
			DEVX(ix+1,iy) * (x - ix) * (iy + 1 - y) +
			DEVX(ix,iy+1) * (ix + 1 - x) * (y - iy) +
			DEVX(ix+1,iy+1) * (x - ix) * (y - iy);
	}
}

/* cmd_g24p_fix: given a deviated point, return the corrected point.
 * cmd_g24p_dev: given a correcte point, return the deviated point.
 * The input and output is in garmin's 24bit unit */
void cmd_g24p_fix (int *px, int *py)
{
	double x, y;

	x = (*px * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y = (*py * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	*px -= (int)lround(cmd_point_dev(x, y, 0) * 0x1000000 / 360 / CMD_DUPD);
	*py -= (int)lround(cmd_point_dev(x, y, 1) * 0x1000000 / 360 / CMD_DUPD);
}
void cmd_g24p_dev (int *px, int *py)
{
	double x, y;

	x = (*px * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y = (*py * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	*px += (int)lround(cmd_point_dev(x, y, 0) * 0x1000000 / 360 / CMD_DUPD);
	*py += (int)lround(cmd_point_dev(x, y, 1) * 0x1000000 / 360 / CMD_DUPD);
}

/* cmd_ls_dx: get average deviation x of a vertical line segment.
 * The purpose of it is to correct rectangle boundry.
 * x, y0, y1 are all in sample unit.
 * returned value is in deviation unit.
 * swapxy: I'm too lazy to implement two versions: cmd_ls_dx and cmd_ls_dy
 * swapxy = 0 means cmd_ls_dx(x, y0, y1)
 * swapxy = 1 means cmd_ls_dy(y, x0, x1) */
static double cmd_ls_dx (double x, double y0, double y1, int swapxy)
{
	int n, i, ix, iy0, iy1;
	const int my_x_num = swapxy ? cmd_y_num : cmd_x_num;
	const int my_y_num = swapxy ? cmd_x_num : cmd_y_num;
	double sum;

	assert(y0 <= y1);

	ix = (int)x;
	if (ix <= 0) ix = x = 0;
	else if (ix > my_x_num - 2) ix = x = my_x_num - 2;

	iy0 = lround(y0);
	iy1 = lround(y1);
	if (iy0 < 0) iy0 = 0;
	if (iy0 >= my_y_num) iy0 = my_y_num - 1;
	if (iy1 < 0) iy1 = 0;
	if (iy1 >= my_y_num) iy1 = my_y_num - 1;

	n = iy1 - iy0;
	if (n <= 1)
		return swapxy ?
			cmd_point_dev((y0 + y1) / 2, x, 1) :
			cmd_point_dev(x, (y0 + y1) / 2, 0);
	if (n > 50) n = 50; /* don't want too much computation */
	/* TODO: the deviation follows periodic pattern.
	 * if our samples are aliased against the period, we can get biased average. */
	for (sum = 0.0, i = 0; i <= n; i ++) {
		int iy = (i * (iy1 - iy0) + n * iy0) / n;
		if (swapxy)
			sum += DEVY(iy,ix) * (ix + 1 - x) + DEVY(iy,ix+1) * (x - ix);
		else
			sum += DEVX(ix,iy) * (ix + 1 - x) + DEVX(ix+1,iy) * (x - ix);
	}
	return sum / (n + 1);
}

/* cmd_rect_dev: get average deviation of a rectangle area.
 * input parameters are in sample unit.
 * returned value is in deviation unit. */
static double cmd_rect_dev (double x0, double x1, double y0, double y1, int isdy)
{
	int ix0, ix1, iy0, iy1, nx, ny, countx, county;
	double sum;

	assert(x0 <= x1 && y0 <= y1);

	ix0 = (int)x0;
	if (ix0 < 0) ix0 = 0;
	else if (ix0 >= cmd_x_num) ix0 = cmd_x_num - 1;
	ix1 = (int)x1;
	if (ix1 < 0) ix1 = 0;
	else if (ix1 >= cmd_x_num) ix1 = cmd_x_num - 1;
	iy0 = (int)y0;
	if (iy0 < 0) iy0 = 0;
	else if (iy0 >= cmd_y_num) iy0 = cmd_y_num - 1;
	iy1 = (int)y1;
	if (iy1 < 0) iy1 = 0;
	else if (iy1 >= cmd_y_num) iy1 = cmd_y_num - 1;

	nx = ix1 - ix0;
	ny = iy1 - iy0;

	if (nx <= 1)
		return cmd_ls_dx((x0 + x1) / 2, y0, y1, 0);
	if (ny <= 1)
		return cmd_ls_dx((y0 + y1) / 2, x0, x1, 1);
	if (nx > 10) nx = 10; /* don't want too much computation */
	if (ny > 10) ny = 10;
	/* TODO: the deviation follows periodic pattern.
	 * if our samples are aliased against the period, we can get biased average. */
	for (sum = 0.0, county = 0; county <= ny; county ++)
	for (countx = 0; countx <= nx; countx ++) {
		int ix = (countx * (ix1 - ix0) + nx * ix0) / nx;
		int iy = (county * (iy1 - iy0) + ny * iy0) / ny;
		sum += isdy ? DEVY(ix,iy) : DEVX(ix,iy);
	}
	return sum / ((nx + 1) * (ny + 1));
}

/* cmd_g24r_fix: given a divated rectangle, return the corrected one.
 * cmd_g24r_dev: given a correct rectangle, return the divated one.
 * avgopt: average option. 0: average the area; 1: average the boundary.
 * the input and output is in garmin's 24bit unit.
 * assume x0 <= x1 && y0 <= y1 */
void cmd_g24r_fix (int *px0, int *py0, int *px1, int *py1, int avgopt)
{
	double x0, y0, x1, y1;

	assert(*px0 <= *px1 && *py0 <= *py1);
	assert(avgopt == 0 || avgopt == 1);

	x0 = (*px0 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	x1 = (*px1 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y0 = (*py0 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;
	y1 = (*py1 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	if (avgopt == 0) {
		int dx = (int)lround(cmd_rect_dev(x0, x1, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		int dy = (int)lround(cmd_rect_dev(x0, x1, y0, y1, 1) * 0x1000000 / 360 / CMD_DUPD);
		*px0 -= dx;
		*px1 -= dx;
		*py0 -= dy;
		*py1 -= dy;
	} else {
		*px0 -= (int)lround(cmd_ls_dx(x0, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		*px1 -= (int)lround(cmd_ls_dx(x1, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		*py0 -= (int)lround(cmd_ls_dx(y0, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
		*py1 -= (int)lround(cmd_ls_dx(y1, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
	}
}
void cmd_g24r_dev (int *px0, int *py0, int *px1, int *py1, int avgopt)
{
	double x0, y0, x1, y1;

	assert(*px0 <= *px1 && *py0 <= *py1);
	assert(avgopt == 0 || avgopt == 1);

	x0 = (*px0 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	x1 = (*px1 * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	y0 = (*py0 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;
	y1 = (*py1 * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;

	if (avgopt == 0) {
		int dx = (int)lround(cmd_rect_dev(x0, x1, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		int dy = (int)lround(cmd_rect_dev(x0, x1, y0, y1, 1) * 0x1000000 / 360 / CMD_DUPD);
		*px0 += dx;
		*px1 += dx;
		*py0 += dy;
		*py1 += dy;
	} else {
		*px0 += (int)lround(cmd_ls_dx(x0, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		*px1 += (int)lround(cmd_ls_dx(x1, y0, y1, 0) * 0x1000000 / 360 / CMD_DUPD);
		*py0 += (int)lround(cmd_ls_dx(y0, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
		*py1 += (int)lround(cmd_ls_dx(y1, x0, x1, 1) * 0x1000000 / 360 / CMD_DUPD);
	}
}
