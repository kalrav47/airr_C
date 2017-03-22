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

struct data {
int csd;
char id[12];
struct node *next;
}n;
// General macros
#define MAX 4096
#define MAX_CMD 20
#define MAXCONN 1000

// Global variables
int csd=0,csdd=0;
int socket_write, socket_read,port,portr, ssize,i;
int csd_all[MAXCONN] = {0};
int csd_general[MAXCONN] = {0};
int csd_sb[MAXCONN] = {0};
long int count_gen = 0;
long int count_sb = 0;
static int isGenRunning=0;
static int isRunning=0;
pthread_mutex_t lock;
pthread_t threads,threada;
fd_set readfds;
struct data *root=NULL;
void exit_function(char message[MAX])
{	
	printf("Exiting server with error : %s\n",message);
	exit(0);
}


// Broadcast msg for our switchboard. Strictly 11 chars only.
void broadcast_Message_For_SB(char message[],int *csdd)
{
	int i;
	char reply[20];

	struct data *tempp;
	tempp=root;
			//printf("start searching our desired board \n");	
		while(tempp  != NULL)
		{
			if(strstr(message,tempp->id)!= NULL)
			{
				printf("v1 got it \n");
				write(tempp->csd,message, strlen(message)+1);
				printf("sent it\n");
				read(tempp->csd,reply,20);
				printf("received reply %s\n",reply);
				write(*csdd,reply,strlen(reply)+1);
				printf("transaction over \n");
				break;
			}
			else
			{
				tempp=tempp->next;
			}
		}
		if(tempp == NULL)
		{
			write(*csdd,"NOSWITCHBOARD",14);
		}
//printf("cmd compelted successfully \n");
}


void *start_listen_To_all( void *param)
{
	int *csdd = (int*)param;
	int i = 0;
	char message[MAX_CMD];
	
	while(1){
		if( read(*csdd,message,20) > 0 )
		{
			//printf("Recevied command %s\n",message);
			if(strcmp(message,"SWITCHBOARD")==0)
			{
			
				//printf("Seems switchboard \n");
				write(*csdd,"ACKNOWLEDGE",12);
				read(*csdd,message,13);
				write(*csdd,"ACKNOWLEDGE",12);
				
				struct data *temp;
				struct data *ptr = (struct data *)malloc(sizeof(struct data));
				ptr->csd=*csdd;
				strcpy(ptr->id,message);

				struct data *temppp;
				temppp=root;
				
		while(temppp  != NULL)
		{
			if(strstr(message,temppp->id)!= NULL)
			{
				close(temppp->csd);
				temppp->csd = *csdd;
				free(ptr);
				break;
			}
			else
			{
				temppp=temppp->next;
			}
		}
		if(temppp == NULL)
		{
			if(root == NULL )
				{
					root = ptr;
					root->next = NULL;
				}
				else
				{
					temp=root;
					while(temp->next != NULL)
					{
						temp=temp->next;
					}
					temp->next=ptr;
					ptr->next=NULL;
				}
		}
					//printf("Done adding switchboard to database \n");		
				//pthread_create( &threada, NULL,start_listen_To_SB,(void *)csdd);
				break;
			}
			else if(strcmp(message,"HEARTBEATCK")==0)
			{
				write(*csdd,"IAMTHERE4U",12);
				close(*csdd);
				break;
			}	
			else
			{
				//printf("locking mutex \n");
				pthread_mutex_lock( &lock);
				//printf("locked mutex and asking board\n");
				broadcast_Message_For_SB(message,csdd);
				//printf("done asking board... unlocking mutex \n");
				pthread_mutex_unlock( &lock);
				//printf("unlocked mutex \n");
				close(*csdd);
				break;
			}
		}}

}



int main(void)
{
	
	long int count=1,i;
	struct sockaddr_in saddress,raddress;
	struct sockaddr_in caddress[MAXCONN]; 
	
	pthread_mutex_init( &lock, NULL);
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
	saddress.sin_port = htons(5000);
	saddress.sin_addr.s_addr = inet_addr("192.168.0.1");

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




























