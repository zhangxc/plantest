/* netlog.c 
 *
 * Make reports or logs, and transfer them to server over TCP/IP socket.
 * ----
 * @ D.Wick 2009.7
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h> 	/* struct ifreq */
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include "plantest.h"
#include "netlog.h"
#include "syslog.h"
#include "netcmd.h"
#include "pto.h"	/* for ptos */

extern struct vars * const v;
extern struct plantest_operations ptos[];

extern int syslog(char *, ...);
extern int wait_for_cmd(void);
extern int recv_netcmd(struct netcmd_struct *);

int confd = 0; // Hold the socket to plantest server
struct sockaddr_in consa;

static int get_systime(struct tm *tm)
{
	time_t tt;
	struct tm *tp;

	time(&tt);
	tp = localtime(&tt);
	memcpy(tm, tp, sizeof(struct tm));

	return 0;
}


static int validate_reply(struct netcmd_struct *r)
{
	PDEBUG("%s: cmd %8x\n", __FUNCTION__, r->cmd);
	if (r->cmd == NC_VERIFY)
		return 0;
	else
		return 1;
}

static int send_netlog(struct netlog_struct *log)
{
	int retry = 3;
	struct netcmd_struct reply;

	PDEBUG("%s: type %8x, err %8lx\n", __FUNCTION__, log->type, log->errcode);
	while (retry-- > 0) {
		sendto(confd, (void *)log, sizeof(struct netlog_struct), 0, 
		       (struct sockaddr *)&consa, sizeof(struct sockaddr));
		// delay
		if (log->type == LOGTYPE_VRY) 
			return 0;
		recv_netcmd(&reply);
		if (validate_reply(&reply) == 0)
			return 0;
	}

	return 1;
}


int netlog(int errcode, int type) 
{
	struct netlog_struct msg;
	FILE *fp;
	char buffer[16]; // must fit netlog_struct definition
	struct tm tm_sys, tm_rtc;
	struct sysinfo info;
	int updays, uphours, upmins;
	int ret;

	// A. rtc time "YYYYMMDDHHMMSSw"
	if (!(fp = fopen(v->board->rtc_file, "r"))) {
		syslog(SYS_ERROR "Cannot open %s\n", v->board->rtc_file);
		return 1;
	}
	if (!fgets(buffer, 16, fp)) {
		syslog(SYS_ERROR "Wrong RTC format\n");
		ret = 2;
		goto NETLOG_FAIL;
	}

	// B. sys time "YYYYMMDDHHMMSSw"
	get_systime(&tm_sys);
	// C. up  time "DDHHMM"
	sysinfo(&info);

	// encapsulation
	str2tm(buffer, &tm_rtc);

	updays = (int) info.uptime / (60*60*24);
	upmins = (int) info.uptime / 60;
	uphours = (upmins / 60) % 24;
	upmins %= 60;

	bzero(&msg, sizeof(struct netlog_struct));
	/* log type */
	msg.type = type;

	// error code
	msg.errcode = errcode;
	strncpy(msg.product, v->board->name, strlen(v->board->name) + 1);

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d%1d",
		tm_rtc.tm_year + 1900, tm_rtc.tm_mon + 1, tm_rtc.tm_mday,
		tm_rtc.tm_hour, tm_rtc.tm_min, tm_rtc.tm_sec,
		tm_rtc.tm_wday);
	strncpy(msg.rtc_time, buffer, 16);

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d%1d",
		tm_sys.tm_year + 1900, tm_sys.tm_mon + 1, tm_sys.tm_mday,
		tm_sys.tm_hour, tm_sys.tm_min, tm_sys.tm_sec,
		tm_sys.tm_wday);
	strncpy(msg.sys_time, buffer, 16);

	sprintf(buffer, "%02d%02d%02d", 
		updays, uphours, upmins);
	strncpy(msg.up_time, buffer, 8);

	strncpy(msg.inaddr, v->inaddr, 16);
	strncpy(msg.hwaddr, v->hwaddr, 18);

	
	if (type == LOGTYPE_INIT) {
		/* netlog_init:
		 *
		 *   an simplified netlog(), but force to get reponsed 
		 */
		while (send_netlog(&msg)) {
			printf(".");
			fflush(stdout);
		}
		printf("\n");
	} else 
		send_netlog(&msg);
NETLOG_FAIL:
	fclose(fp);
	return 0;
}

int report_netlog(void)
{
	netlog_tick();

	return 0;
}

int init_netlog(void) 
{
	struct ifreq ifreq;
	int ret = 0;

	/* create a UDP socket to connect to the server */
	bzero(&consa, sizeof(consa));
	consa.sin_family = AF_INET;
	consa.sin_port = htons(v->port);
	inet_pton(AF_INET, v->servd, &consa.sin_addr);

	confd = socket(AF_INET, SOCK_DGRAM, 0);
	if (confd < 0) {
		syslog(SYS_FATAL "Cannot create a socket\n");
		return 1;
	}

	/* fetch the ip addr of net device */
	strcpy(ifreq.ifr_name, v->board->eth);
	if (ioctl(confd, SIOCGIFADDR, (char *)&ifreq) < 0) {
		syslog(SYS_ERROR "Ioctl with an error\n");
		ret = 2;
		goto FAIL;
	}
	strncpy(v->inaddr, inet_ntoa(((struct sockaddr_in *)(&ifreq.ifr_addr))->sin_addr), 16);
	/* fetch the mac address of net device */
	if (ioctl(confd, SIOCGIFHWADDR, &ifreq) < 0) {
		syslog(SYS_ERROR "Ioctl with an error\n");
		ret = 3;
		goto FAIL;
	}

	sprintf(v->hwaddr, "%02x:%02x:%02x:%02x:%02x:%02x",
		(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5]);

	/* the initiazation netlog (plantest protocol):
	 *   netlog_init to be the first datagram sent to server, some jobs to 
	 *   be finished: 
	 *
	 *     - set the rtc time
	 *     - ...
	 */
	PDEBUG("plantest protocol: set rtc time during initialization\n");
	netlog_init();
	while (wait_for_cmd() != NC_SET_RTC);

	return 0;
FAIL:
	close(confd);
	return ret;
}

void exit_netlog(void)
{
	if (confd)
		close(confd);
}
