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

	// A. rtc time "YYYYMMDDHHMMSSw"
	if (!(fp = fopen(RTC_DEV_FILE, "r"))) {
		syslog(SYS_ERROR "Cannot open "RTC_DEV_FILE"\n");
		return 1;
	}
	if (!fgets(buffer, RTC_LENGTH, fp)) {
		syslog(SYS_ERROR "Wrong RTC format\n");
		return 2;
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
	strncpy(msg.product, ptos[v->i_pto].ids, strlen(ptos[v->i_pto].ids) + 1);

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

	send_netlog(&msg);

	fclose(fp);
	return 0;
}

/* netlog_init:
 *
 *   an simplified netlog(), but force to get reponsed 
 */
int netlog_init(void)
{
	struct netlog_struct nlog;

	bzero(&nlog, sizeof(nlog));
	nlog.type = LOGTYPE_INIT;

	while (send_netlog(&nlog)) {
		printf(".");
		fflush(stdout);
	}
	printf("\n");

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
	strcpy(ifreq.ifr_name, TEST_NETDEV);
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
	if (wait_for_cmd() != NC_SET_RTC)
		return 4;

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
