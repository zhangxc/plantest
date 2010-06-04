/* netcmd.c
 */
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

/* wait_for_netcmd()
 * 
 * using asynchronous i/o to fetch commands from server
 *
 * returns:
 *     0,  select() timeout (means no command received)
 *    -1,  select() return with error
 *    -2,  recvfrom() with errors
 *     n,  number of bytes received;
 */
int recv_netcmd(struct netcmd_struct *nc)
{
	int n;
	int ret = 0;
	int maxfdpl;
	
	fd_set rset;
	struct timeval timeout = { TICK_TIMEOUT, 0 };

	FD_ZERO(&rset);
	FD_SET(confd, &rset);
	maxfdpl = confd + 1;

	if (select(maxfdpl, &rset, NULL, NULL, &timeout) < 0) {
		syslog(SYS_ERROR "select error\n");
		return -1;
	}

	if (FD_ISSET(confd, &rset)) {
		socklen_t len = sizeof(consa);
		n = recvfrom(confd, (void *)nc, sizeof(*nc), 0, 
			     (struct sockaddr *)&consa, &len);
		if (n < 0) {
			syslog(SYS_ERROR "recvfrom error\n");
			ret = -2;
			goto NO_CMD_RECEIVED;
		}

		PDEBUG("%s: cmd %8x\n", __FUNCTION__, nc->cmd);
		ret = n;
	}

NO_CMD_RECEIVED:
	FD_ZERO(&rset);
	return ret;
}


int wait_for_cmd(void)
{
	int ret = 0;
	struct netcmd_struct nc;

	if (recv_netcmd(&nc))
		switch(nc.cmd) {
		case NC_VERIFY:
			ret = NC_VERIFY;
			break;
		case NC_UPGRADE:
			pto->update();
			ret = NC_UPGRADE;
			break;
		case NC_SET_RTC:
			pto->set_rtc(nc.rtctime);
			ret = NC_SET_RTC;
			break;
                default:
			netlog_err(NL_NETCMD_BAD_FORMAT);
			ret = -3;
			break;
                }

	return ret;
}
