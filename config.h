#ifndef __CONFIG_H__
#define __CONFIG_H__


//#define SERVER_ID  "127.0.0.1"
#define SERVER_ID "192.168.16.91"
#define SERVER_PORT 27015

#define TEST_PTO "p5100" // 16 bits at most
#define TEST_MEMSIZE 1024*1024*32
#define TEST_MTDSIZE 1024*1024*16
#define TEST_NETDEV "eth2"
#define TICK_TIMEOUT 2

#define MTD_MNT_POINT "/"
#define MTD_TEST_FILE "mtdtest_9m"

#define RTC_DEV_FILE "/dev/sinfor/rtc"
#define RTC_LENGTH 16

#define DMESG_FILE "/var/log/dmesg" /* absolete */

#define HTTP_SERVER "192.168.16.90"
#define IMAGE_KERNEL "zImage-16sf"
#define IMAGE_DISK  "cramfs-16sf.img"

#endif//__CONFIG_H__
