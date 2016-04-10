#include "network.h"
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MAXLINE 5
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

int main(int argc, char *argv[])
{
 	struct sockaddr_in clientaddr;
	int sockfd;
	int i=0,n,connfd ,epfd,nfds; 
	char line[MAXLINE];
	struct epoll_event ev,events[20];
   	int listenfd=create_socket_and_listen(INADDR_ANY,2456,5);
	if(listenfd < 0)
	{
		printf("listen erro %d\n",listenfd);
		return 1 ;
	}	
	char buf[100]={0};
	for ( ; ; ) 
	{
        //等待epoll事件的发
     	nfds=epoll_wait(epfd,events,20,500);
		ev.data.fd=listenfd;
		ev.events=EPOLLIN|EPOLLET;
		epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
       	 //处理所发生的所有事件
		epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
        for(i=0;i<nfds;++i)
        {
            if(events[i].data.fd==listenfd)//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口，建立新的连接。

            {
                connfd = accept(listenfd,(struct sockaddr *)&clientaddr, sizeof(clientaddr));
                if(connfd<0)
				{
                    perror("connfd<0");
                    exit(1);
                }
                //setnonblocking(connfd);

               // char *str = inet_ntoa(clientaddr.sin_addr);
              //  cout << "accapt a connection from " << str << endl;
                //设置用于读操作的文件描述符

                ev.data.fd=connfd;
                //设置用于注测的读操作事件

                ev.events=EPOLLIN|EPOLLET;
                //ev.events=EPOLLIN;

                //注册ev

                epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入。

            {
               // cout << "EPOLLIN" << endl;
                if ( (sockfd = events[i].data.fd) < 0)
                    continue;
                if ( (n = read(sockfd, line, MAXLINE)) < 0) 
					{
	                    if (errno == ECONNRESET) 
						{
	                        close(sockfd);
	                        events[i].data.fd = -1;
                   		 } else
                       // std::cout<<"readline error"<<std::endl;
							printf("read erro");
			
               		 } else if (n == 0) {
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                line[n] = '/0';
					printf("read ：%s",line);
                //cout << "read " << line << endl;
                //设置用于写操作的文件描述符

                ev.data.fd=sockfd;
                //设置用于注测的写操作事件

                ev.events=EPOLLOUT|EPOLLET;
                //修改sockfd上要处理的事件为EPOLLOUT

                //epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

            }
       
        }
}
   return 1;
}
