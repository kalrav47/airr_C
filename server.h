//General header files
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>
#include<string.h>
#include<signal.h>
#include<time.h>
#include <netinet/tcp.h>

// General macros
#define MAX_CMD 4096
#define MAXCONN 1000
#define SERVER_IP "192.168.0.1"
#define SERVER_PORT 5000
#define NOSWITCHBOARD "NOSWITCHBOARD"
#define SWITCHBOARD "SWITCHBOARD"
#define ACKNOWLEDGE "ACKNOWLEDGE"
#define HEARTBEAT "HEARTBEATCK"
#define IAMTHERE4U "IAMTHERE4U"
#define SBEXITING "SBEXITING"

//#define DEBUG

// Quit program in case of error
// @ input : exit message
void exitFunction(char[]);

// Broadcast command given by bridge server to perticular switchboard.
// Tells switch board to perfomr given command and get reply from sb.
// also pass on that reply to bridge server.
// @ input : message to be sent and socked id.
void broadcastToSwitchBoardAndGetReply(char[],int *);

// Serve the request request by mobile app or anything from public.
// It replies to specific command by sending request to appropriate switch board 
// and taking reply from it.
// @ input : cliend socket id
void *serveRequest( void *);

void *connCheck();
