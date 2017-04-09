// Compile for PC : gcc server.c -o server -lpthread
// Compile for CHIP : ~/CHIP-SDK/CHIP-buildroot/output/host/usr/bin/arm-linux-gnueabihf-gcc server.c -o server -lpthread -w

// Header file
#include "server.h"

struct data {
    int csd;
    char id[12];
    struct data *next;
} n;

// Global variables
int socket_write;
pthread_t threads;
struct data *root=NULL;

void exitFunction(char message[])
{
    printf("Exiting server with error : %s\n",message);
    exit(0);
}

void broadcastToSwitchBoardAndGetReply(char message[],int *csdd)
{
    char reply[MAX_CMD];

    struct data *tempp;
    tempp=root;

    while(tempp  != NULL)
    {
        // Check specific switchboard according to id
        // through the linked list that
        if(strstr(message,tempp->id)!= NULL)
        {
#ifdef DEBUG
	    printf("Writing '%s' to sb\n",message);
#endif
            // Pass command to switch board
            write(tempp->csd,message, MAX_CMD);

#ifdef DEBUG
	    printf("Writing Done... waiting for reply\n");
#endif
            // get reply from switchboard
            read(tempp->csd,reply,MAX_CMD);

#ifdef DEBUG
	    printf("Got reply '%s' from sb and writing back to bridge server\n",reply);
#endif
            // write back to bridge server.
            write(*csdd,reply,MAX_CMD);

#ifdef DEBUG
	    printf("Sent reply to bridge server\n",reply);
#endif
            break;
        }
        else
        {
            tempp=tempp->next;
        }
    }

    if(tempp == NULL)
    {
        write(*csdd,NOSWITCHBOARD,MAX_CMD);
    }
}


void *serveRequest( void *param)
{
    int *csdd = (int*)param;
    char message[MAX_CMD];

    while(1) {
        if( read(*csdd,message,MAX_CMD) > 0 )
        {
#ifdef DEBUG
	    printf("Got message '%s' to server...\n",message);
#endif
            if(strcmp(message,SWITCHBOARD)==0)
            {
                // found that new switchboard is connected. We have found
                // it so acknowledge it.
                write(*csdd,ACKNOWLEDGE,MAX_CMD);
#ifdef DEBUG
	        printf("Ack sent !\n");
#endif
                // now sb will send its id so store it in linked list.
                read(*csdd,message,MAX_CMD);
#ifdef DEBUG
	        printf("Got id of switchBoard '%s' to server...\n",message);
#endif
                // acknowledge that we have got id.
                write(*csdd,ACKNOWLEDGE,MAX_CMD);
#ifdef DEBUG
	        printf("Ack sent again..!\n");
#endif
                // store that id in linked list so that we can communicate
                // with it lator to pass command.
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
                        temppp = temppp->next;
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

                break;
            }
            else if(strcmp(message,HEARTBEAT)==0)
            {
                // it have been asked for heartbeat. reply them that server is here.
                write(*csdd,IAMTHERE4U,MAX_CMD);
                close(*csdd);
                break;
            }
            else
            {
		// it is not heartbeat or switchboard, so beliving that it is request from
		// bridgeserver and pass command to perticular switchboard.
                broadcastToSwitchBoardAndGetReply(message,csdd);

                close(*csdd);
                break;
            }
        }
    }
}



int main(void)
{

    int csd;
    struct sockaddr_in saddress;

    // create socket
    socket_write = socket( AF_INET, SOCK_STREAM, 0);
    if( socket_write == -1)
    {
        exitFunction("Socket init write");
    }

    int on = 1;
    int ret = setsockopt( socket_write, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    // bind address to socket
    saddress.sin_family = AF_INET;
    saddress.sin_port = htons(SERVER_PORT);
    saddress.sin_addr.s_addr = inet_addr(SERVER_IP);

    if( bind( socket_write, (struct sockaddr*)&saddress, sizeof(saddress)) == -1)
    {
        exitFunction("Socket bind write");
    }

    // mark socket to listen for incoming connections
    listen( socket_write, MAXCONN);

    // wait for a client connection
    while(1)
    {
        csd = accept( socket_write, (struct sockaddr*)NULL,NULL);
        pthread_create( &threads, NULL,serveRequest, (void*)&csd);
        pthread_join(threads,NULL);
    }

    pthread_exit(NULL);

}




























