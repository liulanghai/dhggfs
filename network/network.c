#include "network.h"
int create_socket_and_listen(uint32_t bindip,uint32_t port,uint32_t listenmqsize)
{
	struct sockaddr_in svraddr;
	int listenfd;
	bzero(&svraddr,sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(port);
	svraddr.sin_addr.s_addr = htons(bindip);
	listenfd=socket(AF_INET,SOCK_STREAM,0);
	if(bind(listenfd,(struct sockaddr *)&svraddr,sizeof(svraddr))<0)
		return -1;
	if(listen(listenfd,listenmqsize)<0)
		return -2;
	return listenfd;
}
