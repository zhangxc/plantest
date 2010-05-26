/* plantest.c 
 *
 * KS8695 plant test program
 * ----
 * @ D.Wick 2009.7
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "pto.h"
#include "syslog.h"
#include "plantest.h"


extern struct plantest_operations *init_pto(void);
extern int init_netlog(void);
extern void exit_netlog(void);
extern int report_netlog(void);
extern int check_netcmd(void);
extern int syslog(char *msg, ...);

struct vars variables = {};
struct vars * const v = &variables;
struct tseq_struct tseq[] = {
	{1, 0, "SDRAM device test                   "},
	{2, 0, "MTD device test                     "},
	{3, 0, "Time check, such as rtc and uptime  "},
	{4, 0, "Dmesg check like segfault           "},
	{5, 0, "Stress test                         "},
	{0, 0, NULL},
};
struct plantest_operations *pto = NULL;

void cleanup(void);
void do_tick(void);

int main(int argc, char **argv)
{
	int result = 0;

	// initialization
	if (init_netlog()) {
		/* What am i supposed to do without a communication ? */
		syslog(SYS_FATAL "socket init error. Exit!\n");
		exit(1);
	}

	if (!(pto = init_pto())) {
		syslog(SYS_FATAL "pto initiazation error, check this out!\n");
		exit(1);
	}

	v->i_tst = 0;

	// processing the test
	{
	do_test:
		switch(tseq[v->i_tst].pattern) {
		case 1:
			result = pto->memtest();
			break;
		case 2:
			result = pto->mtdtest();
			break;
		case 3:
			result = pto->nettest();
			break;
		case 4:
			result = pto->modtest();
			break;
		case 5:
			result = pto->strtest();
			break;
		default:
			break;
		}

		if (result) {
			tseq[v->i_tst].errors++;
			v->ecounts++;
		}

		v->i_tst ++;
		if (v->i_tst > NB_OF(tseq) - 2) {
			v->pass ++;
			v->i_tst = 0;
		}

		do_tick();
		goto do_test;
	}

	cleanup();
	return result;
}

void do_tick()
{
	// send a packet to report the status
	report_netlog();

	// check the socket to see if comes the commands from the server
	check_netcmd();
}


void cleanup()
{
	// sockets cleanup
	exit_netlog();

	// warning with alarm
	if (v->ecounts)
		(void)system("echo 1 > /dev/alarm");
			
}
