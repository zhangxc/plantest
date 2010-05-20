/* pto.c 
 *
 * ----
 * @ D. Wick 2009.7
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <time.h>

#include "plantest.h"
#include "pto.h"
#include "netlog.h"

extern char clientid[];
extern char *serverid;
extern int netlog(int msgno);

int exec_cmdlines(char **commands)
{
	char **cmds = commands;
	int result;

	do {
		result = system(*cmds);
		if (result == -1) {
			netlog(NL_SYSTEM_ERROR);
			break;
		} else if (result) {
			netlog(NL_SHELL_ERROR);
			break;
		}
	} while (strcmp(*cmds++, ""));

	return 0;
}

int ks8695_memtest(void)
{
	struct sysinfo info;

	sysinfo(&info);
	if (info.totalram < MALLOC_TEST) {
		netlog(NL_SDRAM_ERROR);
		return 1;
	}
	netlog(NL_DEFAULT);

	return 0;
}

int ks8695_mtdtest(void)
{
	// scan /proc/partitions
	FILE *fp;
	char buf[64];
	int in;
	unsigned long size, mtdsize = 0;
	int result;

	fp = fopen("/proc/mtd", "r");
	if (fp == NULL) {
		netlog(NL_NO_MTD_IN_PROCFS);
		return 1;
	}

	fgets(buf, 64, fp);
	while (fgets(buf, 64, fp)) {
		sscanf(buf, "mtd%d: %08lx", &in, &size);
		mtdsize += size;
	}

	fclose(fp);
	if (mtdsize <= 0xf00000) {
		netlog(NL_PARTITION_DOWN);
		return 2;
	}

	// make some simple file operations to check MTD file system
	char *cmdline[] = {
		"cat /lib/*.so.? > /lib.all",
		"cp /lib.all /bin", 
		"cp /lib.all /home",
		"cp /lib.all /usr",
		""
	};

	exec_cmdlines(cmdline);
	result = system("cmp /lib.all /bin/lib.all || cmp /lib.all /home/lib.all || cmp /lib.all /usr/lib.all || rm -f /bin/lib.all /home/lib.all /usr/lib.all");
	if (result != 0) {
		netlog(NL_MTD_DIFF_ERROR);
		return 3;
	}

	return 0;
}

int ks8695_nettest(void)
{
	// eth0 test, just try to ping server

	return 0;
}

int ks8695_modtest(void)
{
	char *cmdline[] = {
		"echo 200904101654305 > /dev/sinfor/rtc",
		""
	};
	//200904101654305
	struct tm tm = {
		.tm_sec = 30,
		.tm_min = 54,
		.tm_hour = 16,
		.tm_mday = 10,
		.tm_mon = 3,
		.tm_year = 109,
	};
	time_t tt, tt_d;

	exec_cmdlines(cmdline);

	if (time(&tt) == (time_t)(-1)) {
		netlog(NL_GET_TIME_ERROR);
		return 1;
	}

	if ((tt_d = mktime(&tm)) == (time_t)(-1)) {
		netlog(NL_GET_TIME_ERROR);
		return 2;
	}

	if ((long) tt - (long) tt_d < 0 ||
	    (long) tt - (long) tt_d > 3600) {
		netlog(NL_RTC_ERROR);
		return 3;
	}

	return 0;
}

int ks8695_strtest(void)
{
	/* ... stressing test ... */
	return 0;
}

void ks8695_sysupdate(void)
{
	char st1[256], st2[256], st5[256];
	char *cmdline[] = {
		"mount -t ramfs none /tmp",
		"",
		"",
		"dd if=/tmp/zImage-16sf of=/dev/mtdblock1",
		"rm /tmp/*",
		"",
		"dd if=/tmp/cramfs-16sf.img of=/dev/mtdblock2",
		""
	};

	sprintf(st1, "ifconfig eth0 %s up", clientid);
	sprintf(st2, "cd /tmp && wget http://%s/zImage-16sf", serverid);
	sprintf(st5, "cd /tmp && wget http://%s/cramfs-16sf.img", serverid);
	cmdline[1] = st1;
	cmdline[2] = st2;
	cmdline[5] = st5;

	exec_cmdlines(cmdline);
}

static struct plantest_operations ptos[] = {
	/* Micrel Kendin's KS8695 */
	{"ks8695", ks8695_memtest, ks8695_mtdtest, ks8695_nettest, ks8695_modtest, ks8695_strtest, ks8695_sysupdate},
	{"p5100", ks8695_memtest, ks8695_mtdtest, ks8695_nettest, ks8695_modtest, ks8695_strtest, ks8695_sysupdate},
	/* Intel IXP4xx */
	{},
};

struct plantest_operations *init_pto(void)
{
	int i;
	char *ids = CURRENT_PTO;
	struct plantest_operations *pto = NULL;

	for (i = 0; i < sizeof(ptos)/sizeof(ptos[0]); i++)
		if (strcasecmp(ids, ptos[i].ids) == 0) {
			pto = &ptos[i];
			break;
		}


	if (!pto)
		netlog(NL_INVALID_PTO_ID);

	return pto;
}
