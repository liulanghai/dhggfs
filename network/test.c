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
struct devent_base
{
	int epollfd;
	struct epoll_event *dev;
	struct epoll_event *events;	
}
struct devent
{
	struct devent_base *deventbase;
	int deventfd;
	int (*readcllback) (int argc ,char* argv[]);
	int (*writecallbac) (int argc,char *argv[]);
	int (*errorandsignl) (int argc ,char *argv[]);
}
struct devent_base * deventinit();//return epoll_fd
struct devent_base * deventinit()
{
	struct devent_base * eventbase = (struct devent_base *) malloc(sizeof(struct devent_base));
	eventbase->epoll_event =(struct epoll_event *) malloc(sizeof(struct epoll_event)*MAXEPOLLEVENTSIZE);
	if((eventbase->epollfd = epoll_create(256))<0)
		return NULL;
	if(eventbase->epoll_event == NULL)
		return NULL;
	return eventbase;	
}
int deventset(struct devent *setdevent,int fd,int option)//return 0 success ,return <0 error
{
	setdevent->deventfd=fd;
	setdevent->
}
int deventadd(struct devent *setdevent);//set event 
{
	return epoll_ctl((setdevent->deventbase)->epollfd,EPOLL_CTL_ADD,setdevent->deventfd,(setdevent->deventbase)->dev);
}
int deventfree(struct devent *);
int deventpoll(struct devent *);

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
            if(events[i].data.fd==listenfd)
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
 	 	   	else              
			{
				if ( (sockfd = events[i].data.fd) < 0)
					continue;
					printf("read fd=%d\n",events[i].data.fd);
                if ( (n = read(events[i].data.fd, line, MAXLINE)) < 0) 
				{
					 if (errno == ECONNRESET) 
						{
	                        close(sockfd);
                   			epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev); 
						}
					 else
						printf("read erro");
			
               	 }
				 else if (n == 0)
				 {
					close(sockfd);
                   	epoll_ctl(epfd,EPOLL_CTL_DEL,sockfd,&ev);
					printf("read no data\n");
                }
                line[n] = '\0';
				printf("read ：%s\n",line);
				write(sockfd,line,n);
                ev.data.fd=sockfd;
                ev.events=EPOLLIN|EPOLLET;
                epoll_ctl(epfd,EPOLL_CTL_MOD,sockfd,&ev);

           	}
      	} 
	}	
   return 1;
}	
