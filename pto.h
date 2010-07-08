#ifndef __PTO_H__
#define __PTO_H__

// PTO indexes
#define DEFAULT 0
#define KS8695 DEFAULT+1
#define P5100  DEFAULT+2
#define S5100  DEFAULT+3

struct board_description {
	char name[16];
	char eth[16];		/* ethernet interface */
	int  memsize;
	int  mtdsize;		/* rootfs size */
	char rtc_file[16];
	char kernel[16];
	char kdisk[16];
};

struct plantest_operations {
	struct board_description *bds;
	int (*memtest)(void);		/* SDRAM test */
	int (*mtdtest)(void);		/* MTD test */
	int (*nettest)(void);		/* network test */
	int (*modtest)(void);		/* module test */
	int (*strtest)(void);		/* stress test */
	int (*dmgchck)(void);		/* dmesg check */
	void (*update)(void);		/* final updates */

	int (*set_rtc)(char[16]);
};

#endif//__PTO_H__
