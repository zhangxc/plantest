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
#include <stdio.h>

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
 *    -3,  inavaild command
 *    NC_VERIFY,  verify received
 *    NC_UPGRADE, upgrade executed
 *    NC_SET_RTC, set_rtc executed
 */
int wait_for_netcmd(void)
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
		return -1;
	}

	if (FD_ISSET(confd, &rset)) {
		n = recvfrom(confd, (struct netcmd_struct *)&c, 
			 sizeof(struct netcmd_struct), 0, NULL, NULL);
		if (n == 0) {
			goto NO_CMD_RECEIVED;
		} else if (n < 0) {
			syslog("error when performing recvfrom\n");
			return -2;
		}

		PDEBUG("%s: cmd %8x\n", __FUNCTION__, c.cmd);
		if (c.cmd != NC_VERIFY)
			netlog_vry();

		switch(c.cmd) {
		case NC_VERIFY:
			return NC_VERIFY;
			break;
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

NO_CMD_RECEIVED:
	return 0;
}
