#ifndef CMDLIB_H
#define CMDLIB_H

/* CMD_DUPD deviation unit per degree:
 * Since the max deviation observed is 0.0085509,
 * 0.0085509 * 2500000  = 21377.25 fits will under signed short. */
#define CMD_DUPD 2500000
#define CMD_X0 72
#define CMD_X1 135
#define CMD_Y0 18
#define CMD_Y1 54

#define DEVX(idx,idy) cmd_devs[((idy)*cmd_x_num + (idx)) * 2]
#define DEVY(idx,idy) cmd_devs[((idy)*cmd_x_num + (idx)) * 2 + 1]

extern int cmd_spd; /* sample per degree */
extern int cmd_x_num, cmd_y_num;
extern short *cmd_devs;

int cmd_init (const char *path);
void cmd_fini (void);

static inline int cmd_g24p_inchina (int x, int y)
{
	/* 72 * 2^24 / 360 = 3355443.2 */
	return (x > 3355443 && x < 6291456 &&
			y > 838860 && y < 2516583);
}

/* assume x0 <= x1 && y0 <= y1
 * both are inclusive */
static inline int cmd_g24r_inchina (int x0, int y0, int x1, int y1)
{
	return (x0 > 3355443 && x1 < 6291456 &&
			y0 > 838860 && y1 < 2516583);
}

void cmd_g24p_fix (int *px, int *py);
void cmd_g24p_dev (int *px, int *py);

#endif
