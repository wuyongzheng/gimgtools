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

void cmd_g24p_fix (int *px, int *py)
{
	if (!cmd_g24p_inchina(*px, *py))
		return;
	double x = (*px * 360.0 / 0x1000000 - CMD_X0) * cmd_spd;
	double y = (*py * 360.0 / 0x1000000 - CMD_Y0) * cmd_spd;
	int ix = (int)x;
	int iy = (int)y;
	double dx = DEVX(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
		DEVX(ix+1,iy) * (x - ix) * (iy + 1 - y) +
		DEVX(ix,iy+1) * (ix + 1 - x) * (y - iy) +
		DEVX(ix+1,iy+1) * (x - ix) * (y - iy);
	dx /= CMD_DUPD;
	double dy = DEVY(ix,iy) * (ix + 1 - x) * (iy + 1 - y) +
		DEVY(ix+1,iy) * (x - ix) * (iy + 1 - y) +
		DEVY(ix,iy+1) * (ix + 1 - x) * (y - iy) +
		DEVY(ix+1,iy+1) * (x - ix) * (y - iy);
	dy /= CMD_DUPD;
	*px -= dx * 0x1000000 / 360;
	*py -= dy * 0x1000000 / 360;
}

void cmd_g24p_dev (int *px, int *py)
{
	assert(0);
}

void cmd_g24r_fix (int *px0, int *py0, int *px1, int *py1)
{
	assert(0);
}

void cmd_g24r_dev (int *px0, int *py0, int *px1, int *py1)
{
	assert(0);
}
