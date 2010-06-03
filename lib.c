/* lib.c
 */

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stdio.h>
#include "plantest.h"

unsigned long get_random(int seed)
{
	srandom(seed);
	return random();
}


char *substr(char *str, char *sub, int i, int l)
{
	int j = 0;

	while (i-- > 0) str++;
	for (j = 0; j < l && str[j] != '\0'; j++)
		sub[j] = str[j];
	if (j <= l) sub[j] = '\0'; // encounter a null terminator

	return sub;
}


/* convert the char[16] to tm structure */
void str2tm(char rtc[16], struct tm *tm) 
{
	char f[5];

	substr(rtc, f, 0, 4);
	tm->tm_year = atoi(f) - 1900;

	substr(rtc, f, 4, 2);
	tm->tm_mon = atoi(f) - 1;

	substr(rtc, f, 6, 2);
	tm->tm_mday = atoi(f);

	substr(rtc, f, 8, 2);
	tm->tm_hour = atoi(f);

	substr(rtc, f, 10, 2);
	tm->tm_min = atoi(f);

	substr(rtc, f, 12, 2);
	tm->tm_sec = atoi(f);

	substr(rtc, f, 14, 1);
	tm->tm_wday = atoi(f);
}


int validate_time(struct tm tm) 
{
	if (tm.tm_year > 200 ||	/* since 1900 */
	    tm.tm_mon  > 11  ||	/* [0, 11] */
	    tm.tm_mday > 31  ||	/* [1, 31] */
	    tm.tm_hour > 23  ||	/* [0, 23] */
	    tm.tm_min  > 59  ||	/* [0, 59] */
	    tm.tm_sec  > 59  ||	/* [0, 59] */
	    tm.tm_wday > 6) {	/* [0,  6] */
		return 1;
	}

	return 0;
}
