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
#include <string.h>
#include <getopt.h>

#include "pto.h"
#include "syslog.h"
#include "plantest.h"

extern int sysloged;

extern struct plantest_operations *init_pto(char[]);
extern int init_netlog(void);
extern void exit_netlog(void);
extern int report_netlog(void);
extern int wait_for_cmd(void);
extern int syslog(char *msg, ...);

#define VERSION "0.0.1"
#define AUTHOR "@Sunthink June, 2010"

struct vars variables = {};
struct vars * const v = &variables;
struct tseq_struct tseq[] = {
	{1, 0, "SDRAM device test                   "},
	{2, 0, "MTD device test                     "},
	{3, 0, "Time check, such as rtc and uptime  "},
	{4, 0, "Dmesg check like segfault           "},
//	{5, 0, "Stress test                         "},
	{0, 0, NULL},
};
struct plantest_operations *pto = NULL;

void display_header(void);
void display_ui(void);
void clean_ui(void);
void cleanup(void);
void do_tick(void);

void usage(void)
{
	fprintf(stderr, "plantest %s %s\n", VERSION, AUTHOR);
	fprintf(stderr, "Usage:\tplantest [Options...]\n\n");
	fprintf(stderr, "  -s, --server=SERVER\n");
	fprintf(stderr, "  -p, --port=SERVER_PORT\n");
	fprintf(stderr, "  -t, --pto=PLAN\n");
	fprintf(stderr, "  -w, --httpd=HTTPD\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "  -h, --help\n");
}

int main(int argc, char **argv)
{
	int result = 0;
	int c;
	char pto_name[16];

	/* Init 1. defaults argument */
	strncpy(pto_name, TEST_PTO, strlen(TEST_PTO) + 1);
	strncpy(v->servd, SERVER_ID, strlen(SERVER_ID) + 1);
	strncpy(v->httpd, HTTP_SERVER, strlen(HTTP_SERVER) + 1);
	v->port = SERVER_PORT;
	v->debug = 0;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"server", 1, 0, 's'},
			{"port", 1, 0, 'p'},
			{"debug", 0, 0, 0},
			{"pto", 1, 0, 't'},
			{"httpd", 1, 0, 'w'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0},
		};

		c = getopt_long(argc, argv, "s:p:t:w:hT",
				long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 0:
			v->debug = 1;
			break;
		case 's':
			bzero(v->servd, strlen(v->servd));
			strncpy(v->servd, optarg, strlen(optarg) + 1);
			break;
		case 'p':
			v->port = atoi(optarg);
			break;
		case 't':
			bzero(pto_name, strlen(pto_name));
			strncpy(pto_name, optarg, strlen(optarg) + 1);
			break;
		case 'w':
			bzero(v->httpd, strlen(v->httpd));
			strncpy(v->httpd, optarg, strlen(optarg) + 1);
			break;
		case 'h':
		default:
			usage();
			exit(1);
			break;
		}
	}

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", argv[optind++]);
		printf("\n");
		exit(1);
	}

	/* Init 2. extern inits */
	if (!(pto = init_pto(pto_name))) {
		syslog(SYS_FATAL "pto initiazation error, check this out!\n");
		exit(1);
	}

	if (init_netlog()) {
		/* What am i supposed to do without a communication ? */
		syslog(SYS_FATAL "socket init error. Exit!\n");
		exit(1);
	}

	v->i_tst = 0;
	display_header();

	// processing the test
	{
	do_test:

		if (v->debug == 0)
			display_ui();

		switch(tseq[v->i_tst].pattern) {
		case 1:
			result = pto->memtest();
			break;
		case 2:
			result = pto->mtdtest();
			break;
		case 3:
			result = pto->modtest();
			break;
		case 4:
			result = pto->dmgchck();
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
			result = 0;
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
	clean_ui();
	return result;
}

void display_header()
{
	printf("server - ip %s:%d\n", v->servd, v->port);
	printf("client - ip %s:%d, hw %s, %s\n", 
	       v->inaddr, v->port, v->hwaddr, v->board->eth);
}

void display_ui()
{
	printf("\r");
	printf("[%4d.%4d] %d/%u %s", 
	       v->pass, v->ecounts, v->i_tst+1, 
	       NB_OF(tseq) - 1, tseq[v->i_tst].info);
	printf("\r");
	fflush(stdout);

	/* ready to syslog */
	sysloged = 0;
}

void clean_ui()
{
	printf("\n");
}

void do_tick()
{
	// send a packet to report the status
	report_netlog();

	// check the socket to see if comes the commands from the server
	wait_for_cmd();
}


void cleanup()
{
	// sockets cleanup
	exit_netlog();

	// warning with alarm
	if (v->ecounts)
		system("echo 1 > /dev/alarm");
			
}
