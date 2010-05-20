#ifndef __SYSLOG_H__
#define __SYSLOG_H__

struct msg_struct {
	int msgno;
	char *message;
};


// Message level 0-3
#define SYS_EMERG "<0>"
#define SYS_FATAL "<1>"
#define SYS_ERROR "<2>"
#define SYS_INFO  "<3>"

#endif //__SYSLOG_H__
