/* netcmd.h
 */
#ifndef __NETCMD_H__
#define __NETCMD_H__


struct netcmd_struct {
	int cmd;
	union {
		char product[16]; /* 01 */
		char rtctime[16]; /* 03 */
	};
};


/* cmd definition */
#define NC_VERIFY	0x565259	/* "VRY" */
#define NC_PLAY		1
#define NC_PAUSE	2
#define NC_STOP		3
#define NC_EXIT		4

#define NC_UPGRADE	16
#define NC_GET_RTC	17
#define NC_SET_RTC	18
#endif//__NETCMD_H__
