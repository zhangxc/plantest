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
#include <net/if.h> // struct ifreq
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include "plantest.h"
#include "netlog.h"
#include "syslog.h"

char clientid[20];
char serverid[] = SERVER_ID;
int  serverport = SERVER_PORT;

static int confd; // Hold the socket to plantest server
static struct sockaddr_in consa;

extern struct vars * const v;

extern char *substr(char *str, char *sub, int i, int l);

static int get_systime(struct tm *tm)
{
	time_t tt;
	struct tm *tp;

	time(&tt);
	tp = localtime(&tt);
	memcpy(tm, tp, sizeof(struct tm));

	return 0;
}


static int validate_reply(char *r)
{
	/* not implemented yet */
	return 0;
}

static int send_netlog(struct netlog_struct *log)
{
	int retry = 3;
	char reply[64];

	while (retry-- > 0) {
		sendto(confd, (void *)log, sizeof(struct netlog_struct), 0, 
		       (struct sockaddr *)&consa, sizeof(struct sockaddr));
		// delay
		recvfrom(confd, reply, 64, 0, NULL, NULL);
		if (validate_reply(reply) == 0)
			return 0;
	}

	syslog(SYS_ERROR "Error sending log to "SERVER_ID);
	return 1;
}


int netlog(int errcode) 
{
	struct netlog_struct msg;
//	FILE *fp;
	char buffer[16]; // must fit netlog_struct definition
	char f[4];
	struct tm tm_sys, tm_rtc;
	struct sysinfo info;
	int updays, uphours, upmins;

	// A. rtc time "YYYYMMDDHHMMSSw"
/*	if (!(fp = fopen(RTC_DEV_FILE, "r"))) {
		syslog(SYS_ERROR "Cannot open "RTC_DEV_FILE"\n");
		return 1;
	}
	if (!fgets(buffer, RTC_LENGTH, fp)) {
		syslog(SYS_ERROR "Wrong RTC format\n");
		return 2;
	}
*/
	// test
	strncpy(buffer, "201005181510303\0", RTC_LENGTH);
	// B. sys time "YYYYMMDDHHMMSSw"
	get_systime(&tm_sys);
	// C. up  time "DDHHMM"
	sysinfo(&info);

	// encapsulation
	substr(buffer, f, 0, 4);
	tm_rtc.tm_year = atoi(f) - 1900;

	substr(buffer, f, 4, 2);
	tm_rtc.tm_mon = atoi(f) - 1;

	substr(buffer, f, 6, 2);
	tm_rtc.tm_mday = atoi(f) - 1;

	substr(buffer, f, 8, 2);
	tm_rtc.tm_hour = atoi(f);

	substr(buffer, f, 10, 2);
	tm_rtc.tm_min = atoi(f);

	substr(buffer, f, 12, 2);
	tm_rtc.tm_sec = atoi(f);

	substr(buffer, f, 14, 2);
	tm_rtc.tm_wday = atoi(f) - 1;

	updays = (int) info.uptime / (60*60*24);
	upmins = (int) info.uptime / 60;
	uphours = (upmins / 60) % 24;
	upmins %= 60;

	// error code
	bzero(&msg, sizeof(struct netlog_struct));
	msg.errcode = errcode;
	strncpy(msg.product, TEST_PTO, strlen(TEST_PTO));

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d%1d",
		tm_rtc.tm_year+1900, tm_rtc.tm_mon+1, tm_rtc.tm_mday+1,
		tm_rtc.tm_hour, tm_rtc.tm_min, tm_rtc.tm_sec,
		tm_rtc.tm_wday+1);
	strncpy(msg.rtc_time, buffer, 16);

	sprintf(buffer, "%04d%02d%02d%02d%02d%02d%1d",
		tm_sys.tm_year + 1900, tm_sys.tm_mon + 1, tm_sys.tm_mday + 1,
		tm_sys.tm_hour, tm_sys.tm_min, tm_sys.tm_sec,
		tm_sys.tm_wday + 1);
	strncpy(msg.sys_time, buffer, 16);

	sprintf(buffer, "%02d%02d%02d", 
		updays, uphours, upmins);
	strncpy(msg.up_time, buffer, 8);

	strncpy(msg.inaddr, v->inaddr, 16);
	strncpy(msg.hwaddr, v->hwaddr, 18);

	send_netlog(&msg);
	return 0;
}


int init_netlog() 
{
	struct ifreq ifreq;
	int ret = 0;

	/* create a UDP socket to connect to the server */
	bzero(&consa, sizeof(consa));
	consa.sin_family = AF_INET;
	consa.sin_port = htons(serverport);
	inet_pton(AF_INET, serverid, &consa.sin_addr);

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

	sprintf(v->hwaddr, "%02x:%02x:%02x:%02x:%02x:%02x%c",
		(unsigned char)ifreq.ifr_hwaddr.sa_data[0],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[1],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[2],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[3],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[4],
		(unsigned char)ifreq.ifr_hwaddr.sa_data[5],
		'\0');

	/* Set the RTC clock */
	// not implemented yet

	PDEBUG("%s: local IP addr %s, MAC addr %s\n", 
	       TEST_NETDEV, v->inaddr, v->hwaddr);
	return 0;
FAIL:
	close(confd);
	return ret;
}

void exit_netlog(void)
{
	close(confd);
}
