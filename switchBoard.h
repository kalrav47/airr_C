//General header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
//#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<pthread.h>
#include<time.h>
#include <stdbool.h>
#include <linux/inotify.h>

#define PORT 5000
#define ROOT_DIR "/root"
#define ID_FILE "/root/id.txt"
//#define ID_FILE "id"
#define STAT_FILE "/root/stat.txt"
#define USAGE_FILE "/root/usage.txt"
//#define IP "127.0.0.1"
#define IP "192.168.0.1"
#define MY_TYPE "SWITCHBOARD"
#define ACKNOWLEDGE "ACKNOWLEDGE"
#define NOACTIONASKED "NOACTIONASKED"
#define INVALIDACTION "INVALIDACTION"
#define TURNEDON "TURNEDON"
#define TURNEDOFF "TURNEDOFF"
#define RECVFAILEDD "RECVFAILEDD"
#define STATUS "STATUS"
#define NOSUCHSWITCH "NOSUCHSWITCH"
#define NOPROPERVAL "NOPROPERVAL"
#define ARM "ARM"
#define DISARM "DISARM"
#define ALREADYARM "ALREADYARM"
#define ALREADYDISARM "ALREADYDISARM"
#define SYSARMED "SYSARMED"
#define ARM_FILE ".AMRED"
#define ARM_CMD "touch .AMRED"
#define DISARM_CMD "rm .AMRED"
#define USAGE "USAGE"
#define COMMAD_MIN_LENGTH 10
#define MAX_CMD 4096
#define HEARTBEAT "HEARTBEATCK"
#define IAMTHERE4U "IAMTHERE4U"
#define RESETUSAGE "RESETUSAGE"
#define RESETSTAT "RESETSTAT"
#define RESETUSAGEDONE "RESETUSAGEDONE"
#define RESETSTATDONE "RESETSTATDONE"
#define SYNC "sync"
#define SBEXITING "SBEXITING"

// for inotify
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN ( 1024 * ( EVENT_SIZE + 16 ) )

//#define DEBUG

// Starts the socket when program runs.
// After that it starts listen to clients request
// and preform requested operation and replies back
void startSocketAndServeCommands();

// In case of any signal, it turns off the status led and
// exit the program
void incaseOfSignal();

// Turns led on/off
// @input,
// arg 1 : gpio pin number (mapped to an array gpio_pins_write)
// arg 2 : value (on/off)
void gpioWrite(int,int);

// load the value i.e. flag when program runs.
void loadPreValues();

// starts reading to touch switches and turns led on/off
// according to touch event.
void *gpioRead();

// checks connection to server and kills if server is not there.
void *heartBeatCheck();


// Save the usage of each switch
void saveUsage();
