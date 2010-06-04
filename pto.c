/* pto.c 
 *
 * ----
 * @ D. Wick 2009.7
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
#include <sys/klog.h>
#include <time.h>

#include "plantest.h"
#include "syslog.h"
#include "netlog.h"
#include "pto.h"

extern char clientid[];
extern char serverid[];
extern struct vars * const v;
extern unsigned long get_random(int seed);
extern char *substr(char *str, char *sub, int i, int l);
extern int syslog(char *, ...);
extern int netlog(int, int);


unsigned long calc_chksum(char *mem, unsigned long size)
{
	unsigned long sum = 0;
	unsigned long *p = (unsigned long *)mem;

	while ((size -= 4) > 0)
		sum += *p++;
	return sum;
}


int memtest_chksum(char *start, unsigned long size)
{
	unsigned long *p = (unsigned long *)start;
	unsigned long sum = 0, rand = 0;
	int i = 0;

	for (i = 1; i < size/4; i++) {
		rand = get_random(i * time(NULL));

		*p++ = rand;
		sum += rand;
	}
	if (sum != calc_chksum(start, size))
		return 1;

	return 0;
}


int get_rtctime(struct tm *tm) 
{
	FILE *fp;
	char buffer[16]; /* must fit netlog_struct definition */

	if (!(fp = fopen(RTC_DEV_FILE, "r"))) {
		netlog_err(NL_RTC_NO_DEV_FILE);
		syslog(SYS_ERROR "Cannot open %s\n", RTC_DEV_FILE);
		return 1;
	}
	if (!fgets(buffer, RTC_LENGTH, fp)) {
		netlog_err(NL_RTC_BAD_FORMAT);
		syslog(SYS_ERROR "Wrong RTC format\n");
		return 2;
	}

	str2tm(buffer, tm);

	return 0;
}


int exec_cmdlines(char **commands)
{
	char **cmds = commands;
	int result = 0;

	do {
		result = system(*cmds);
		if (result == -1) {
			netlog_err(NL_LIBC_SYSTEM);
			break;
		} else if (result) {
			netlog_err(NL_SHELL_ERROR);
			break;
		}
	} while (strcmp(*cmds++, ""));

	return result;
}


int ks8695_memtest(void)
{
	struct sysinfo info;
	char *mem;
	unsigned long msize;
	int ret = 0;

	/* capacity check */
	sysinfo(&info);
	if (info.totalram < (TEST_MEMSIZE/10)*9) {
		netlog_err(NL_MEM_SIZE);
		return 1;
	}

	/* memory test */
	msize = (info.freeram/10) * 9;
	mem = (char *)malloc(msize);
	if (!mem) {
		syslog(SYS_ERROR "malloc error");
		netlog_err(NL_MEM_MALLOC);
		ret = 2;
		return ret;
	}

	msize = 4*1024*1024;
	if (memtest_chksum(mem, msize) != 0) {
		netlog_err(NL_MEM_CHECKSUM);
		ret = 3;
		goto DONE;
	}

DONE:
	free(mem);
	return ret;
}

int ks8695_mtdtest(void)
{
	struct statfs stat;
	unsigned long mtdsize = 0;
	int result;
	char st1[256];

	if (statfs(MTD_MNT_POINT, &stat)) {
		syslog(SYS_ERROR "statfs error");
		netlog_err(NL_LIBC_STATFS);
		return 1;
	}

	mtdsize = (stat.f_bsize / 1024) * stat.f_blocks;
	if (mtdsize <= ((TEST_MTDSIZE/1024)/4) * 3) {
		PDEBUG("mtdsize %08lx\n", mtdsize);
		netlog_err(NL_MTD_SIZE);
		return 2;
	}

	// make some simple file operations to check MTD file system
	char *cmd1[] = {
		"mount -t tmpfs none /tmp",
		NULL,
		"cp /tmp/"MTD_TEST_FILE" /",
		"",
	};

	char *cmd0[] = {
		"rm -f /"MTD_TEST_FILE,
		"umount /tmp",
		"",
	};

	bzero(st1, 256);
	sprintf(st1, "cd /tmp && wget -q http://%s/%s/%s",
		HTTP_SERVER, TEST_PTO, MTD_TEST_FILE);
	cmd1[1] = st1;

	if (exec_cmdlines(cmd1)) {
		syslog("%s: shell excution with errors\n", __FUNCTION__);
		return 3;
	}
	result = system("cmp /"MTD_TEST_FILE" /tmp/"MTD_TEST_FILE);
	if (result != 0) {
		netlog_err(NL_MTD_RWCHECK);
		return 4;
	}
	if (exec_cmdlines(cmd0)) {
		syslog("%s: shell excution with errors\n", __FUNCTION__);
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
	struct tm tm_rtc;

	/* rtc test */
	if (get_rtctime(&tm_rtc))
		return 1;

	if (validate_time(tm_rtc)) {
		netlog_err(NL_RTC_BAD_FORMAT);
		return 2;
	}

	return 0;
}


int ks8695_strtest(void)
{
	/* ... stressing test ... */
	return 0;
}


int ks8695_dmgchck(void)
{
	int i = 0, n = 0;
	int sz;
	int ret = 0;
	char *buf;
	char *needle[] = {
		"segmentation fault",	/* 0 */
		"segfault",		/* 1 */
		" fault",		/* 2 */
		"error",		/* 3 */
	};
	char **nd = needle;

	sz = 16392;
	while (1) {
		buf = (char *)malloc(sz);
		if (!buf) {
			syslog(SYS_ERROR "malloc error");
			netlog_err(NL_MEM_MALLOC);
			return 1;
		}
		n = klogctl(3, buf, sz);	/* read only */
		if (n != sz || sz > (1<<28))
			break;
		free(buf);
		sz *= 4;
	}

	if (n < 0) {
		syslog(SYS_ERROR "klogctl with an error\n");
		netlog_err(NL_LIBC_KLOGCTL);
		ret = 2;
		goto DONE;
	}

	// check the dmesg buffer
	for (i = 0; i < NB_OF(needle); i++) {
		if (!strcasestr(buf, *nd++))
			continue;

		// capture the needle
		switch (i) {
		case 0:
		case 1:
			netlog_err(NL_DMG_SEGFAULT);
			break;
		case 2:
			netlog_err(NL_DMG_FAULT);
			break;
		case 3:
			netlog_err(NL_DMG_ERROR);
			break;
		default:
			netlog_err(NL_DEFAULT);
			break;
		}
	}

DONE:
	free(buf);
	return ret;
}

void ks8695_sysupdate(void)
{
	char st1[256], st2[256], st5[256];
	char *cmdline[] = {
		"mount -t ramfs none /tmp",
		NULL,
		NULL,
		"dd if=/tmp/zImage-16sf of=/dev/mtdblock1",
		"rm /tmp/*",
		NULL,
		"dd if=/tmp/cramfs-16sf.img of=/dev/mtdblock2",
		""
	};

	sprintf(st1, "ifconfig eth0 %s up", clientid);
	sprintf(st2, "cd /tmp && wget -q http://%s/%s/%s", 
		HTTP_SERVER, TEST_PTO, IMAGE_KERNEL);
	sprintf(st5, "cd /tmp && wget -q http://%s/%s/%s", 
		HTTP_SERVER, TEST_PTO, IMAGE_DISK);
	cmdline[1] = st1;
	cmdline[2] = st2;
	cmdline[5] = st5;

	if (exec_cmdlines(cmdline))
		syslog("%s: Shell excution with errors\n", __FUNCTION__);
}


int ks8695_set_rtctime(char rtc[16])
{
	struct tm tm;
	char cmd[64];
	int result;

	str2tm(rtc, &tm);
	if (validate_time(tm)) {
		syslog(SYS_ERROR "Invalid time %s\n", rtc);
		netlog_err(NL_NETCMD_BAD_RTC);
		return 1;
	}

	sprintf(cmd, "echo %s > %s", rtc, RTC_DEV_FILE);
	result = system(cmd);
	if (result != 0) {
		netlog_err(NL_NETCMD_SET_RTC);
		return 2;
	}
	return 0;
}


static struct plantest_operations ptos[] = {
	/* Micrel Kendin's KS8695 */
	{"ks8695", ks8695_memtest, ks8695_mtdtest, ks8695_nettest, ks8695_modtest, ks8695_strtest, ks8695_dmgchck, ks8695_sysupdate, ks8695_set_rtctime},
	{"p5100",  ks8695_memtest, ks8695_mtdtest, ks8695_nettest, ks8695_modtest, ks8695_strtest, ks8695_dmgchck, ks8695_sysupdate, ks8695_set_rtctime},
	{"mig100", ks8695_memtest, ks8695_mtdtest, ks8695_nettest, ks8695_modtest, ks8695_strtest, ks8695_dmgchck, ks8695_sysupdate, ks8695_set_rtctime},
	/* Intel IXP4xx */
	{},
};


struct plantest_operations *init_pto(void)
{
	int i;
	char *ids = TEST_PTO;
	struct plantest_operations *pto = NULL;

	for (i = 0; i < sizeof(ptos)/sizeof(ptos[0]); i++)
		if (strcasecmp(ids, ptos[i].ids) == 0) {
			pto = &ptos[i];
			break;
		}

	return pto;
}
