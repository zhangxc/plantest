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
	void (*update)(void);		/* final updates */
};

#define MALLOC_1M   1024 * 1024 * 1
#define MALLOC_2M   1024 * 1024 * 2
#define MALLOC_4M   1024 * 1024 * 4
#define MALLOC_8M   1024 * 1024 * 8
#define MALLOC_16M  1024 * 1024 * 16
#define MALLOC_32M  1024 * 1024 * 32
#define MALLOC_64M  1024 * 1024 * 64
#define MALLOC_128M 1024 * 1024 * 128
#define MALLOC_TEST MALLOC_32M


#endif//__PTO_H__
