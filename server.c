#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <stdio.h>

typedef struct sockaddr SA;
typedef unsigned int uint32;

struct netlog_struct {
	char product[16];
	char hwaddr[18];
	char inaddr[16];
	char rtc_time[16];
	char sys_time[16];
	char up_time[8];
	uint32 errcode;
};

int serverport = 8000;

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	struct netlog_struct nlog;
	int sockfd;
	int len;
	int n;

	/* create a UDP socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(serverport);

	bind(sockfd, (SA *)&servaddr, sizeof(servaddr));

	while(1) {
		len = sizeof(cliaddr);
		n = recvfrom(sockfd, &nlog, 
			     sizeof(struct netlog_struct), 0, (SA *)&cliaddr, &len);
		if (n > 0)
			printf("product id: %s\n", nlog.product);
	}
}
