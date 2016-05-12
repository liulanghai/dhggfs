#include "network.h"
#include <sys/epoll.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include<stdlib.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>

//#include <readline/readline.h>
//#include <readline/history.h>


#define MAXEPOLLEVENTSIZE 2000
#define MAXLINE 16384
#define OPEN_MAX 100
#define LISTENQ 2000
#define SERV_PORT 5000
#define INFTIM 1000
#define MAXN    16384       /* max # bytes client can request */

 void handler(int num)
   {
       int status;
       int pid = waitpid(-1,&status,WNOHANG);
   }

char SendBuf[MAXLINE]={'A'};
struct deventbase
{
    int epollfd;
    struct epoll_event *events;
    struct deventlist  *registereventlist;
};
struct devent
{

    struct deventbase *devbase;
    int deventfd;
    int option;
    void (*readcllback)(struct devent *);
    void (*writecallbac)(struct devent *);
    void (*errorandsignl)(struct devent *);
};
struct deventlist
{
    struct devent *event;
    struct deventlist * next;
};
struct deventbase * deventinit()
{
    struct deventbase * eventbase = (struct deventbase *) malloc(sizeof(struct deventbase));
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
    devbase=de->devbase;
    temp=(de->devbase)->registereventlist;
    if(temp== NULL)
           return -1;
    if((temp->event)->deventfd == de->deventfd)
    {
        (de->devbase)->registereventlist=temp->next;
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
        temp=temp->next;
    }
    return 0;
}
int register_list_add(struct devent * de)
{
    if((de->devbase)->registereventlist == NULL)
    {
        struct deventlist *head;
        head= (struct deventlist *)malloc (sizeof(struct deventlist));
        head->event =(struct devent * )malloc (sizeof(struct devent));
        memcpy(head->event,de,sizeof(struct devent ));
        head->next=NULL;
        (de->devbase)->registereventlist=head;

    }else
    {
        struct deventlist *head;
        head= (struct deventlist * )malloc (sizeof(struct deventlist));
        head->event =(struct devent *)malloc (sizeof(struct devent));
        memcpy(head->event,de,sizeof(struct devent ));
        head->next=(de->devbase)->registereventlist;
        (de->devbase)->registereventlist =head;
    }
    return 0;
}
int devent_add(struct devent *setdevent)//set event
{
        struct epoll_event dev;
        struct devent * connectevent = ( struct devent *)malloc(sizeof(struct devent));
      //  memcpy(connectevent,setdevent,sizeof(struct devent ));
        connectevent->devbase=setdevent->devbase;
        connectevent->deventfd=setdevent->deventfd;
        connectevent->errorandsignl=setdevent->errorandsignl;
        connectevent->option=setdevent->option;
        connectevent->readcllback=setdevent->readcllback;
        connectevent->writecallbac=setdevent->writecallbac;

        dev.data.fd=setdevent->deventfd;
        dev.events=setdevent->option;
        register_list_add(connectevent);
        return epoll_ctl((connectevent->devbase)->epollfd,EPOLL_CTL_ADD,connectevent->deventfd,&dev);
}
int devent_del(struct devent *deldevent)
{
    struct epoll_event dev;

    dev.data.fd=deldevent->deventfd;
    dev.events=deldevent->option;
    if(epoll_ctl((deldevent->devbase)->epollfd,EPOLL_CTL_DEL,deldevent->deventfd,&dev) <0)
        return -1;
    close(deldevent->deventfd);
    return register_list_del(deldevent);
}
void web_child(struct devent * readevent);
void readandreturn(struct devent * readevent);
void myaccept(struct devent * readevent)
{
    int len=0;
    struct sockaddr_in clientaddr;
    int connfd = accept(readevent->deventfd,(struct sockaddr *)&clientaddr, &len);

    struct devent listenevent;
    listenevent.devbase=readevent->devbase;
    listenevent.deventfd=connfd;
    listenevent.readcllback=readandreturn;
    listenevent.writecallbac=NULL;
    listenevent.option=EPOLLIN|EPOLLET;
    if(devent_add(&listenevent)<0)
        printf("devent add is error\n");

    if(connfd>0)
        printf("a client is connect\n");
}

void dothing(void * arg)
{
    int fd;
    struct  deventbase* devbase;
    memcpy(&fd,arg,sizeof(int));
    memcpy(devbase,&arg[sizeof(int)],sizeof(devbase));
    int c=-1;
    struct deventlist *temp=devbase->registereventlist;
    while(temp!=NULL)
    {
        if(((temp->event)->deventfd) == fd)
        {
            printf("read from client\n");
            (*((temp->event)->readcllback))(temp->event);
            break;
        }
        temp=temp->next;
    }
}
int devent_poll(struct  deventbase* devbase)
{
    int nfds=0;
    int i;
    struct epoll_event *temp;
    temp=devbase->events;
    int count=0;
    while(1)
    {
          nfds=epoll_wait(devbase->epollfd,temp,1000,500);
          for(i=0;i<nfds;i++)
          {
              printf("count=%d\n",count);
              count++;
              char *changhsu=(char *)malloc(sizeof(char)*100);
              memcpy(changhsu,&temp[i].data.fd,sizeof(int));
              memcpy(&changhsu[sizeof(int)],devbase,sizeof(devbase));
              pthread_t tid1;
              int err  = pthread_create(tid1,NULL,dothing,changhsu);
              if(err!=0)
              {
                  printf("create pthread error\n");
              }
          }

    }
}
void
pr_cpu_time(void)
{
    double          user, sys;
    struct rusage   myusage, childusage;

    if (getrusage(RUSAGE_SELF, &myusage) < 0)
    {
        printf("getrusage error\n");
        return ;
    }
    if (getrusage(RUSAGE_CHILDREN, &childusage) < 0)
     {
        printf("getrusage error");
        return ;
    }
    user = (double) myusage.ru_utime.tv_sec +
                    myusage.ru_utime.tv_usec/1000000.0;
    user += (double) childusage.ru_utime.tv_sec +
                     childusage.ru_utime.tv_usec/1000000.0;
    sys = (double) myusage.ru_stime.tv_sec +
                   myusage.ru_stime.tv_usec/1000000.0;
    sys += (double) childusage.ru_stime.tv_sec +
                    childusage.ru_stime.tv_usec/1000000.0;

    printf("\nuser time = %g, sys time = %g\n", user, sys);
}
struct sockaddr_in clientaddr;
int listenfd,len;

void myread(struct devent * readevent)
{
    char buf[100];
    int n=-1;
    int ntowrite;

    n=read(readevent->deventfd,buf,100);
    if(n==0)
    {
        printf("read nothing\n");
        devent_del(readevent);
        return;
    }
    if ((ntowrite <= 0) || (ntowrite > MAXLINE))

          write(readevent->deventfd, SendBuf, 100);
}

void sig_int(int signo)
 {
    void    pr_cpu_time(void);
    pr_cpu_time();
    exit(0);
}
void readandreturn(struct devent * readevent)
{
    int         ntowrite;
    ssize_t     nread;
    int writesize=0,temp=0;

    char        line[MAXLINE], *result="I read you !";
    printf("web child\n");
        if ( (nread = read(readevent->deventfd, line, MAXLINE)) == 0)
           {
              if(  devent_del(readevent)<0);
                printf("devent_del error \n");
                return;     /* connection closed by other end */
           }
     temp=write(readevent->deventfd, result, 15);

}

void web_child(struct devent * readevent)
{
    int         ntowrite;
    ssize_t     nread;
    int writesize=0,temp=0;
    char        line[MAXLINE], result[MAXN];
    printf("web child\n");
        if ( (nread = read(readevent->deventfd, line, MAXLINE)) == 0)
           {
              if(  devent_del(readevent)<0);
                printf("devent_del error \n");
                return;
           }  
        ntowrite = atol(line);
        if ((ntowrite <= 0) || (ntowrite > MAXN))
        {
            printf("client request for %d bytes\n", ntowrite);
            return ;
        }else
              printf("client request for %d bytes\n", ntowrite);
        while(writesize<ntowrite)
        {
          temp=write(readevent->deventfd, &result[writesize], ntowrite);
          writesize+=temp;
        }
}
int main(int argc, char *argv[])
{
    int sockfd;
    char line[1024];

    int i=0,n,connfd =0 ,epfd,nfds;
    struct epoll_event ev,events[20];
    int listenfd=create_socket_and_listen(INADDR_ANY,2456,10000);
    if(listenfd < 0)
    {
            printf("listen erro %d\n",listenfd);
            return 1 ;
    }
    signal(SIGINT, sig_int);
    memset(SendBuf,'A',MAXLINE);
    signal(SIGCHLD,handler);
   struct  deventbase *mydevenbase=deventinit();
   struct devent listenevent;
   listenevent.devbase=mydevenbase;
   listenevent.deventfd=listenfd;
   listenevent.option=EPOLLIN|EPOLLET;;
   listenevent.readcllback= myaccept;
   listenevent.writecallbac=NULL;
   devent_add(&listenevent);
   devent_poll(mydevenbase);
    return 1;

}
