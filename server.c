#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>
#include <time.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

#include "netlog.h"
#include "types.h"
#include "netcmd.h"
#include "config.h"

int get_rtctime(struct tm *tm);

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	struct netlog_struct nlog;
	struct netcmd_struct ncmd;
	int sockfd;
	socklen_t len;
	int n;
	struct tm tm;

	/* create a UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);

	bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	while(1) {
		len = sizeof(cliaddr);
		n = recvfrom(sockfd, &nlog, 
			     sizeof(struct netlog_struct), 0, (SA *)&cliaddr, &len);

		printf("type: %8x\terrcode: %8lx\n", nlog.type, nlog.errcode);

		if (nlog.type == LOGTYPE_VRY)
			continue;

		bzero(&ncmd, sizeof(struct netcmd_struct));
		ncmd.cmd = NC_VERIFY;
		sendto(sockfd, (void *)&ncmd, sizeof(struct netcmd_struct), 0, 
		       (struct sockaddr *)&cliaddr, sizeof(struct sockaddr));

		switch(nlog.type) {
		case LOGTYPE_INIT:
			get_rtctime(&tm);
			ncmd.cmd = NC_SET_RTC;
			sprintf(ncmd.rtctime, "201005291442306");
			sendto(sockfd, (void *)&ncmd, sizeof(struct netcmd_struct), 0, 
			       (struct sockaddr *)&cliaddr, sizeof(struct sockaddr));
			break;
		case LOGTYPE_TICK:
		case LOGTYPE_ERROR:
			break;
		default:
			break;
		}


		//DEBUG routine
		if (nlog.type == LOGTYPE_ERROR && nlog.errcode == NL_DEFAULT)
			break;
	}

	return 0;
}

int get_rtctime(struct tm *tm) 
{
	int rtcfd;

	rtcfd = open("/dev/rtc", O_RDONLY);
	ioctl(rtcfd, RTC_RD_TIME, (struct rtc_time *)&tm);

	return 0;
}
