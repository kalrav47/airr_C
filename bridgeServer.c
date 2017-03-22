//gcc server.c -o server -lpthread  -- for pc
//~/CHIP-SDK/CHIP-buildroot/output/host/usr/bin/arm-linux-gnueabihf-gcc server.c -o server -lpthread -w   -- for chip

//General header files
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>
#include<string.h>
#include<signal.h>
#include<time.h>


// Global variables
int csd=0,csdd=0;
int socket_write, socket_read,port,portr, ssize,i;

pthread_t threads;


#define MAX 4096
#define MAX_CMD 20
#define MAXCONN 1000

void exit_function(char message[MAX])
{	
	printf("Exiting server with error : %s\n",message);
	exit(0);
}

void *start_listen_To_all( void *param)
{
	int *csdd = (int*)param;
	
	int sockfd = 0;
    	char msg[20];
    	struct sockaddr_in serv_addr; 
	time_t rawtime;
  	struct tm * timeinfo;

 
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET,"192.168.0.1", &serv_addr.sin_addr)<=0)
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
	
	read(*csdd,msg,20);

    	write(sockfd,msg,strlen(msg)+1);
	
	read(sockfd,msg,20);
	
	write(*csdd,msg,strlen(msg)+1);
 	close(sockfd);
 	close(*csdd);
}



int main(void)
{
	
	long int count=1,i;
	struct sockaddr_in saddress,raddress;
	struct sockaddr_in caddress[MAXCONN]; 
	
	char message[MAX];

	socket_write = socket( AF_INET, SOCK_STREAM, 0);
	if( socket_write == -1)
	{
		exit_function("Socket init write");
	}

	int on = 1;
	int ret = setsockopt( socket_write, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

	/* bind address to socket */
	saddress.sin_family = AF_INET;
	saddress.sin_port = htons(6000);
	saddress.sin_addr.s_addr = INADDR_ANY;

	if( bind( socket_write, (struct sockaddr*)&saddress, sizeof(saddress)) == -1)
	{
		exit_function("Socket bind write");
	}

	/* mark socket to listen for incoming connections */
	listen( socket_write, MAXCONN);
	
	/* wait for a client connection */
	ssize = sizeof(caddress[0]);
	while(1)
	{
		csd = accept( socket_write, (struct sockaddr*)NULL,NULL);
		pthread_create( &threads, NULL,start_listen_To_all, (void*)&csd);
		//printf("Thread started ..\n");
		pthread_join(threads,NULL);
		//printf("Thread finished.. \n");
	}	
		pthread_exit(NULL);
	
}
