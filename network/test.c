#include "network.h"
int main(int argc, char *argv[])
{
 	struct sockaddr_in clientaddr;
	int socketfd; 
   	int listenfd=create_socket_and_listen(INADDR_ANY,2456,5);
	if(listenfd < 0)
	{
		printf("listen erro %d\n",listenfd);
		return 1 ;
	}	
	char buf[100]={0};
   	while(1)
	{
		bzero(buf,100);
 		socketfd=accept(listenfd, (struct sockaddr *)&clientaddr,sizeof(clientaddr));
		if((childprocess=fork())== 0)
		{	printf("accept error");
			return 1;
		}
		printf("a client connect\n");
	
		recv(socketfd,buf,10,0);
		printf("i recv :%s\n",buf);	
	    	if(send(socketfd,"hello",10,0)>0)
	    
        	close(socketfd);
 }  
}
