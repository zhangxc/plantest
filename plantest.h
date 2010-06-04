/* plantest.h */

#ifndef __PLANTEST_H__
#define __PLANTEST_H__

#include <time.h>
#include "config.h"

#undef PDEBUG
#ifdef PT_DEBUG
#  define PDEBUG(fmt, args ...) fprintf(stdout, fmt, ##args);
#else
#  define PDEBUG(fmt, args ...)
#endif

#define NB_OF(arr) sizeof(arr)/sizeof(arr[0])

struct tseq_struct {
	int pattern;
	int errors;
	char *info;
};

struct vars {
	int i_tst;		/* current running test */
	int i_pto;
	int pass;
	int ecounts;

	int  port;
	char inaddr[16];	/* IP  address */
	char hwaddr[18];	/* Mac address */
	char servd[16];		/* Server address */
	char httpd[16];		/* HTTP server */

	int debug;		/* debug mode */
};

/* lib.c */
extern unsigned long get_random(int seed);
extern char *substr(char *str, char *sub, int i, int l);
extern void str2tm(char rtc[16], struct tm *tm);
extern int validate_time(struct tm tm);

#endif // __PLANTEST_H__
