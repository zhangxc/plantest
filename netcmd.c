/* netcmd.c
 */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "plantest.h"
#include "syslog.h"
#include "netlog.h"
#include "netcmd.h"
#include "pto.h"

extern int confd;
extern struct sockaddr_in consa;
extern struct plantest_operations *pto;

extern int syslog(char *msg, ...);
extern int netlog(int errcode, int type);

int check_netcmd(void)
{
	struct netcmd_struct c;
	int n;
	fd_set rset;
	int maxfdpl;
	struct timeval timeout = { TICK_TIMEOUT, 0 };

	FD_ZERO(&rset);
	FD_SET(confd, &rset);
	maxfdpl = confd + 1;

	if (select(maxfdpl, &rset, NULL, NULL, &timeout) < 0) {
		syslog("select error\n");
		return 1;
	}

	if (FD_ISSET(confd, &rset)) {
		n = recvfrom(confd, (struct netcmd_struct *)&c, 
			 sizeof(struct netcmd_struct), 0, NULL, NULL);
		if (n <= 0) {
			syslog("error when performing recvfrom\n");
			return 2;
		}
			
		switch(c.cmd) {
		case NC_UPGRADE:
			pto->update();
			break;

		case NC_SET_RTC:
			pto->set_rtc(c.rtctime);
			break;
		default:
			netlog_err(NL_NETCMD_BAD_FORMAT);
			break;
		}
	}

	return 0;
}
