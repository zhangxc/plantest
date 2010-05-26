#ifndef __PTO_H__
#define __PTO_H__

// PTO indexes
#define DEFAULT 0
#define KS8695 DEFAULT+1
#define P5100  DEFAULT+2
#define S5100  DEFAULT+3

struct plantest_operations {
	char ids[16];
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
