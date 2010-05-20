#ifndef __LOG_H__
#define __LOG_H__

#define MAX_LOG_SIZE  64
#define MAX_LOG_LENGTH 256

#include "types.h"


struct netlog_struct {
	char product[16];
	char hwaddr[18];
	char inaddr[16];
	char rtc_time[16];
	char sys_time[16];
	char up_time[8];
	uint32 errcode;
};

/* errcode, 32 bits
 *  0- 3  Memory test
 *  4- 7  Flash test
 *  8-11  Clock test
 * 12-15  dmesg check
 * 16-31  reserved
 */
#define NL_DEFAULT		0xffffffff
#define NL_MEMORY		0x00000001
#define NL_MEM_ALLOC		0x00000002
#define NL_MEM_CHECK		0x00000004
#define NL_MEM_FAULT		0x00000008

#define NL_MTD			0x00000010
#define NL_MTD_UNAVAIL		0x00000020
#define NL_MTD_PART_SIZE	0x00000040
#define NL_MTD_FAULT		0x00000080

#define NL_TIME			0x00000100
#define NL_RTC_FETCH		0x00000200
#define NL_RTC_FORMAT		0x00000400
#define NL_RTC_SET		0x00000800

#define NL_DMESG		0x00001000
#define NL_DMESG_SEGFAUTL	0x00002000
#define NL_DMESG_ERROR		0x00004000
#define NL_DMESG_FAULT		0x00008000

#define NL_RESERVED		0x11110000

/* message number */
#define NL_ALL_OK		0
#define NL_MALLOC_ERROR		1
#define NL_SDRAM_ERROR		2
#define NL_PARTITION_DOWN	3
#define NL_MTD_DIFF_ERROR	4
#define NL_UPTIME_ERROR		5
#define NL_SYSTEM_ERROR		6
#define NL_CMDLINE_ERROR	7
#define NL_SHELL_ERROR		8
#define NL_SOCKET_ERROR		9
#define NL_INIT_SYS_LOG		10
#define NL_GET_TIME_ERROR	11
#define NL_RTC_ERROR		12
#define NL_NO_MTD_IN_PROCFS	13
#define NL_UNKNOWN_ERROR	14
#define NL_HERE_WE_GO		15
#define NL_INVALID_PTO_ID	16


#endif //__LOG_H__
