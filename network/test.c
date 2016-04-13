#include "network.h"
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE 10000
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000
int main(int argc, char *argv[])
{
 	struct sockaddr_in clientaddr;	
	int sockfd,len=0;
	int i=0,n,connfd =0 ,epfd,nfds; 
	char line[MAXLINE];
	struct epoll_event ev,events[20];
   	int listenfd=create_socket_and_listen(INADDR_ANY,2456,5);
	if(listenfd < 0)
	{
		printf("listen erro %d\n",listenfd);
		return 1 ;
	}	
	len=sizeof(clientaddr);
	char buf[100]={0};
	epfd=epoll_create(256);
	ev.data.fd=listenfd;
        ev.events=EPOLLIN|EPOLLET;
        epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);

	for ( ; ; ) 
	{
     	nfds=epoll_wait(epfd,events,20,500);
        for(i=0;i<nfds;i++)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。

            {
                connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &len);
                if(connfd<0)
				{
                    perror("connfd<0");
                    exit(1);
                }
				printf("a new client connect \n");
                ev.data.fd=connfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
 	//          if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。
      else       {
			//	if ( (sockfd = events[i].data.fd) < 0)
             //   {
			//		  printf("sockfd = %d\n",sockfd);
			//		  continue;
			//	}
                if ( (n = read(events[i].data.fd, line, MAXLINE)) < 0) 
				{
					 if (errno == ECONNRESET) 
						{
							printf("rest\n");
	                        close(sockfd);
	                        events[i].data.fd = -1;
                   		 }
					 else
						printf("read erro");
			
               	 }
				 else if (n == 0)
				 {
					printf("read no data\n");
					break;
                    close(sockfd);
                    events[i].data.fd = -1;
                 }
                line[n] = '\0';
				printf("read ：%s\n",line);

                ev.data.fd=sockfd;
                //设置用于注测的写操作事件

                ev.events=EPOLLIN|EPOLLET;
                //修改sockfd上要处理的事件为EPOLLOUT

                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

           	 }
      		} 
}
   return 1;
}	
