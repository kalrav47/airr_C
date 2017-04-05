// Compile for PC : gcc server.c -o server -lpthread
// Compile for CHIP : ~/CHIP-SDK/CHIP-buildroot/output/host/usr/bin/arm-linux-gnueabihf-gcc server.c -o server -lpthread -w

// Header file
#include "bridgeServer.h"


// Global variables
int client_socketid=0;
int socket_write = 0;
pthread_t threads;

void exit_function(char message[])
{
    printf("Exiting server with error : %s\n",message);
    exit(0);
}

void *serveRequest( void *param)
{
    int *public_client_socketid = (int*)param;
    int internal_client_socketid = 0;
    char msg[MAX_CMD];

    // create socket to internal server
    struct sockaddr_in serv_addr;

    if((internal_client_socketid = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        exit_function("Could not create socket");
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(INTERNAL_SERVER_PORT);

    if(inet_pton(AF_INET,INTERNAL_SERVER_IP,&serv_addr.sin_addr)<=0)
    {
        exit_function("inet_pton error occured");
    }

    // connect to internal server
    if( connect(internal_client_socketid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        exit_function("Connect Failed");
    }

    // read request from public clinet
    read(*public_client_socketid,msg,MAX_CMD);

    // pass it on to internal server
    write(internal_client_socketid,msg,MAX_CMD+1);

    // get reply from internal server
    read(internal_client_socketid,msg,MAX_CMD);

    // reply back to public client
    write(*public_client_socketid,msg,MAX_CMD+1);

    // close both sockets as not required any more
    close(internal_client_socketid);
    close(*public_client_socketid);
}



void main(void)
{

    struct sockaddr_in saddress;

    socket_write = socket( AF_INET, SOCK_STREAM, 0);
    if( socket_write == -1)
    {
        exit_function("Socket init write");
    }

    int on = 1;
    int ret = setsockopt( socket_write, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );

    // bind address to socket
    saddress.sin_family = AF_INET;
    saddress.sin_port = htons(PUBLIC_SERVER_PORT);
    saddress.sin_addr.s_addr = INADDR_ANY;

    if( bind( socket_write, (struct sockaddr*)&saddress, sizeof(saddress)) == -1)
    {
        exit_function("Socket bind write");
    }

    // mark socket to listen for incoming connections
    listen( socket_write, MAXCONN);

    // wait for a client connection
    while(1)
    {
        client_socketid = accept( socket_write, (struct sockaddr*)NULL,NULL);
        pthread_create( &threads, NULL,serveRequest, (void*)&client_socketid);
        pthread_join(threads,NULL);
    }

    pthread_exit(NULL);
}
