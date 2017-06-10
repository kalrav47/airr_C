//General header files
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>
#include<string.h>

#define MAX 4096
#define MAX_CMD 4096
#define MAXCONN 1000
#define INTERNAL_SERVER_IP "192.168.0.1"
#define INTERNAL_SERVER_PORT 5000
#define PUBLIC_SERVER_PORT 6000

// Quit program in case of error
// @ input : exit message
void exit_function(char[]);

// Serve the request request by mobile app or anything from public.
// It replies to specific command by sending request to appropriate switch board 
// and taking reply from it.
// @ input : cliend socket id
void *serveRequest( void *);

