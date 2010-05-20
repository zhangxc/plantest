/* plantest.h */

#ifndef __PLANTEST_H__
#define __PLANTEST_H__

#include "pto.h"
#include "syslog.h"
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
	int i_tst; // current running test
	int i_pto;
	int pass;

	char inaddr[16];     /* IP  address */
	char hwaddr[18];     /* Mac address */
};

// pto.c
extern struct plantest_operations *init_pto(void);

// syslog.c
extern int syslog(char *msg);

// netlog.c
extern int netlog(int msgno);
extern int init_netlog(void);
extern void exit_netlog(void);

// lib.c
extern char *substr(char *, char *, int, int);

#endif // __PLANTEST_H__
