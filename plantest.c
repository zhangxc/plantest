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
#include "plantest.h"

int errcode;
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

void cleanup(void);
void do_tick(void);


int main(int argc, char **argv)
{
	struct plantest_operations *pto = NULL;
	int ret = 0;

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
			pto->memtest();
			break;
		case 2:
//			pto->mtdtest();
			syslog(SYS_INFO "2\n");
			break;
		case 3:
//			pto->nettest();
			syslog(SYS_INFO "3\n");
			break;
		case 4:
//			pto->modtest();
			syslog(SYS_INFO "4\n");
			break;
		case 5:
//			pto->strtest();
			syslog(SYS_INFO "5\n");
			break;
		default:
			syslog(SYS_INFO "0\n");
			break;
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
	return ret;
}

void do_tick()
{
	sleep(1);
	// send a packet to report the status
	// check the socket to see if comes the commands from the server
}


void cleanup()
{
	// sockets cleanup
	// warning with alarm
	exit_netlog();
}
