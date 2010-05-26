/* syslog.c 
 *
 * system log routine.
 * ----
 * @ D.Wick 2009.7
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* syslog
 * 
 * This is syslog(), we want it to work.
 * 
 */
int syslog(char *fmt, ...)
{
	char *p = fmt;
	char *mp;
	int current_log_level = p[1] - '0';
	int size = 0;
	FILE *fout;
	va_list args;

	va_start(args, fmt);

	switch(current_log_level){
	case 0 ... 2:
		p += 3;
		size = strlen(fmt) - 3;
		fout = stderr;
		break;
	case 3:
		p += 3;
		size = strlen(fmt) - 3;
		fout = stdout;
		break;
	default:
		size = strlen(fmt);
		fout = stdout;
		break;
	}

	mp = (char *)malloc(size);
	if (!mp) {
		fprintf(stderr, "[FATAL] %s: malloc error!\n", __FUNCTION__);
		return 1;
	}

	bzero(mp, size);
	strncat(mp, p, size);
	vfprintf(fout, mp,  args);
	free(mp);

	return 0;
}
