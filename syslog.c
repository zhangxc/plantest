/* syslog.c 
 *
 * system log routine.
 * ----
 * @ D.Wick 2009.7
 */



/* syslog
 * 
 * This is syslog(), we want it to work.
 * 
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//#define syslog(fmt, args ...) do_syslog(fmt, ##args)

int syslog(char *msg)
{
	char *p = msg;
	char *mp;
	int current_log_level = p[1] - '0';
	int size = strlen(msg) - 3 + 8;

	mp = (char *)malloc(size);
	if (!mp) {
		fprintf(stderr, "[FATAL] %s: malloc error!\n", __FUNCTION__);
		return 1;
	}
	bzero(mp, size);

	switch(current_log_level){
	case 0:
		p += 3;
		strncpy(mp, "[EMERG] ", 8);
		break;
	case 1:
		p += 3;
		strncpy(mp, "[FATAL] ", 8);
		break;
	case 2:
		p += 3;
		strncpy(mp, "[ERROR] ", 8);
		break;
	case 3:
		p += 3;
		strncpy(mp, "[INFO ] ", 8);
		break;
	default:
		break;
	}

	strncat(mp, p, strlen(msg));
	fprintf(stderr, "%s", mp);
	free(mp);

	return 0;
}
