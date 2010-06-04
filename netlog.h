#ifndef __LOG_H__
#define __LOG_H__

#define MAX_LOG_SIZE  64
#define MAX_LOG_LENGTH 256

#include "types.h"


struct netlog_struct {
	int  type;		/* log type */
	char product[16];
	char hwaddr[18];
	char inaddr[16];
	char rtc_time[16];
	char sys_time[16];
	char up_time[8];
	uint32 errcode;
};

/* log type, int
 */
#define LOGTYPE_VRY		0x565259
#define LOGTYPE_INIT		1
#define LOGTYPE_TICK		2
#define LOGTYPE_ERROR		3


#define netlog_err(errcode)  netlog(errcode, LOGTYPE_ERROR)
#define netlog_vry()   netlog(0, LOGTYPE_VRY)
//#define netlog_init()  netlog(0, LOGTYPE_INIT) /* improved implementation */
#define netlog_tick()  netlog(0, LOGTYPE_TICK)

/* errcode, 32 bits 
 * bit 0- 7, driver errors, max 255
 */
#define NL_DEFAULT		0

#define NL_MEM_SIZE		1	// 内存大小错误
#define NL_MEM_MALLOC		2	// 内存分配失败
#define NL_MEM_CHECKSUM		3	// 内存检测失败

#define NL_MTD_NO_PROCFS	16	// /proc/mtd 读取失败
#define NL_MTD_SIZE		17	// Flash 大小错误
#define NL_MTD_RWCHECK		18	// MTD 读写检测失败

#define NL_RTC_NO_DEV_FILE	32	// RTC 设备文件读取失败
#define NL_RTC_BAD_FORMAT	33	// RTC 输出格式错误
#define NL_RTC_INACCURACY	34	// RTC 漂移过量

/* bit 8-15, kernel faults, max 255 */
#define NL_DMG_NO_DMG_FILE	1 << 8	// DMESG 文件读取失败
#define NL_DMG_FAULT		2 << 8	// 捕捉到致命错误
#define NL_DMG_ERROR		3 << 8	// 捕捉到普通错误
#define NL_DMG_SEGFAULT		4 << 8	// 捕捉到段错误

/* bit 16-23, user space errors, max 255 */
#define NL_PTO_ID_INAVAIL	1 << 16
#define NL_SHELL_ERROR		2 << 16

#define NL_LIBC_SYSTEM		16 << 16
#define NL_LIBC_TIME		17 << 16
#define NL_LIBC_MKTIME		18 << 16
#define NL_LIBC_STATFS		19 << 16
#define NL_LIBC_KLOGCTL		20 << 16

#define NL_NETCMD_BAD_FORMAT	32 << 16	// 服务器的格式命令错误
#define NL_NETCMD_BAD_RTC       33 << 16
#define NL_NETCMD_SET_RTC	34 << 16


/* bit 24-31, reserved */

#endif //__LOG_H__
