#include "network.h"
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<stdlib.h>

#define MAXEPOLLEVENTSIZE 200
#define MAXLINE 10000
#define OPEN_MAX 100
#define LISTENQ 20
#define SERV_PORT 5000
#define INFTIM 1000

struct devent_base
{
    int epollfd;
    struct epoll_event *events;
    struct deventlist  *registereventlist;
};
struct devent
{

    struct devent_base *deventbase;
    int deventfd;
    int option;
    void (*readcllback)();
    void (*writecallbac)();
    void (*errorandsignl)();
};
struct deventlist
{
    struct devent *event;
    struct deventlist * next;
};
struct devent_base * deventinit()
{
    struct devent_base * eventbase = (struct devent_base *) malloc(sizeof(struct devent_base));
    eventbase->events =(struct epoll_event *) malloc(sizeof(struct epoll_event)*MAXEPOLLEVENTSIZE);
    eventbase->registereventlist=NULL;
    if((eventbase->epollfd = epoll_create(256))<0)
        return NULL;
    if(eventbase->events == NULL)
        return NULL;
    return eventbase;
}
int register_list_del(struct devent * de)
{
    struct deventbase *devbase;
    struct deventlist *temp,*freetemp;
    devbase=de->deventbase;
    temp=(de->deventbase)->registereventlist;
    if(temp== NULL)
           return -1;
    if((temp->event)->deventfd == de->deventfd)
    {
        (de->deventbase)->registereventlist=temp->next;
        free(temp);
        return 0;
    }
    while(temp->next)
    {
        if(((temp->next)->event)->deventfd ==de->deventfd)
        {
            freetemp=temp->next;
            temp->next=freetemp->next;
            free(freetemp);
        }
    }
    return 0;
}
int register_list_add(struct devent * de)
{
    if((de->deventbase)->registereventlist == NULL)
    {
        struct deventlist *head;
        head= (struct deventlist *)malloc (sizeof(struct deventlist));
        head->event =(struct devent * )malloc (sizeof(struct devent));
        memcpy(head->event,de,sizeof(struct devent ));
        head->next=NULL;
        (de->deventbase)->registereventlist=head;

    }else
    {
        struct deventlist *head;
        head= (struct deventlist * )malloc (sizeof(struct deventlist));
        head->event =(struct devent *)malloc (sizeof(struct devent));
        memcpy(head->event,de,sizeof(struct devent ));
        head->next=(de->deventbase)->registereventlist;
        (de->deventbase)->registereventlist =head;
    }
}
int devent_add(struct devent *setdevent)//set event
{
        struct epoll_event dev;
        dev.data.fd=setdevent->deventfd;
        dev.events=setdevent->option;
        register_list_add(setdevent);
        return epoll_ctl((setdevent->deventbase)->epollfd,EPOLL_CTL_ADD,setdevent->deventfd,&dev);
}

int devent_del(struct devent *deldevent)
{
    struct epoll_event dev;
    dev.data.fd=deldevent->deventfd;
    dev.events=deldevent->option;
    if(epoll_ctl((deldevent->deventbase)->epollfd,EPOLL_CTL_DEL,deldevent->deventfd,&dev) <0)
        return -1;
    return register_list_del(deldevent);
}
 void dothing(int fd,struct  devent_base* deventbase)
{
    struct deventlist *temp=deventbase->registereventlist;
    while(temp!=NULL)
    {
        if((temp->event->deventfd) == fd)
        {
            (*(temp->event)->readcllback)(temp->event);
        }
        temp=temp->next;
    }
}
int devent_poll(struct  devent_base* deventbase)
{
    int nfds;
    int i;
    struct epoll_event *temp;
    temp=deventbase->events;
    while(1)
    {
          nfds=epoll_wait(deventbase->epollfd,temp,20,500);
          for(i=0;i<nfds;i++)
          {

              dothing(temp[i].data.fd,deventbase);
          }

    }
}
struct sockaddr_in clientaddr;
int listenfd,len;

void myread(struct devent * readevent)
{
    char buf[100];
    int n=-1;
    n=read(readevent->deventfd,buf,100);
    if(n==0)
    {
        printf("read nothing\n");
        devent_del(readevent);
    }
    if(n>0)
    {
        buf[n]=0;
        printf("read :%s\n",buf);
        write(readevent->deventfd,buf,n);
    }
}
void myaccept(struct devent * readevent)
{
    int connfd = accept(listenfd,(struct sockaddr *)&clientaddr, &len);

    struct devent listenevent;
    listenevent.deventbase=readevent->deventbase;
    listenevent.deventfd=connfd;
    listenevent.readcllback=myread;
    listenevent.writecallbac=NULL;
    listenevent.option=EPOLLIN|EPOLLET;
    devent_add(&listenevent);

    if(connfd>0)
        printf("a client is connect\n");
   // if(devent_del(readevent)>-1)
     //   printf("del is ok");
}
int main(int argc, char *argv[])
{
    //struct sockaddr_in clientaddr;
    int sockfd;
    int i=0,n,connfd =0 ,epfd,nfds;
    char line[MAXLINE];
    struct epoll_event ev,events[20];
    listenfd=create_socket_and_listen(INADDR_ANY,2456,5);
    if(listenfd < 0)
    {
            printf("listen erro %d\n",listenfd);
            return 1 ;
    }
   struct  devent_base *mydevenbase=deventinit();
   struct devent listenevent;
   listenevent.deventbase=mydevenbase;
   listenevent.deventfd=listenfd;
   listenevent.option=EPOLLIN|EPOLLET;
   listenevent.readcllback= myaccept;
   listenevent.writecallbac=NULL;
   devent_add(&listenevent);
   // ev.events=EPOLLIN|EPOLLET;
   // epoll_ctl(epfd,EPOLL_CTL_ADD,listenfd,&ev);
   devent_poll(mydevenbase);
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
