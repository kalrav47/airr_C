#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include<time.h>

int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 
time_t rawtime;
  struct tm * timeinfo;

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <message>\n",argv[0]);
        return 1;
    } 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(6000); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 
    printf("Connected.. and asking now...\n");
	 time ( &rawtime );
  timeinfo = localtime ( &rawtime );
	printf("1.\n");
	//send(sockfd,argv[2],strlen(argv[2])+1,MSG_NOSIGNAL  );
	char msg[4096];
	usleep(300);
	//printf("Writing..%s\n",asctime (timeinfo));
    	write(sockfd,argv[2],strlen(argv[2])+1);
	//printf("Writing done..%s\n",asctime (timeinfo));
	usleep(100);
	read(sockfd,msg,4096);
	printf("reading done.\n");
	printf("Message -> %s\n",msg);
	
	
	
 close(sockfd);
    return 0;
}
