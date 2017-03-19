#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<signal.h>
#include<pthread.h>
#include<time.h>
#include <stdbool.h>

#define PORT 5000
//#define ID_FILE "/root/id.txt"
#define ID_FILE "id"
#define STAT_FILE "stat.txt"
#define USAGE_FILE "usage.txt"
#define IP "127.0.0.1"
//#define IP "192.168.1.105"
#define MY_TYPE "SWITCHBOARD"
#define ACKNOWLEDGE "ACKNOWLEDGE"
//#define ACKNOWLEDGE "************"
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

pthread_mutex_t lock;
int flag[15] = { 0, };
time_t begin[15] = { 0, } ;
time_t end[15]= { 0, };
static bool isArmed = false;

void start_socket()
{
    int id,sockfd = 0,length,switch_num,cmdLen;
    char recvBuff[20],data[25],id_string[11],cmd[10];
    struct sockaddr_in serv_addr;
    const char s[2] = "-";
    char *toggle;
    int noAction = 0;
    
    char cutstart;
    int cutend;
    int action = 2;

    FILE *file = fopen(ID_FILE, "r");
    fscanf(file, "%d", &id);
    sprintf(id_string,"%d",id);
    printf("Id file %d\n",id);

    memset(recvBuff, '0', sizeof(recvBuff));
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        incase_of_sig();
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        incase_of_sig();
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        incase_of_sig();
    }

    write(sockfd, MY_TYPE, 12);
    read(sockfd, recvBuff, 13);
    write(sockfd, id_string, 12);
    read(sockfd, recvBuff, 13);
    printf("sent id %s\n",id_string);
    while (1)
    {
        strcpy(recvBuff, "");
        strcpy(data,"");
        read(sockfd, recvBuff, 20);
	printf("got command\n");
        length = strlen(recvBuff);
        //cmd = recvBuff+10;
        
        // cut the command
	
	cutstart=10;
	cutend = length-cutstart;
	
	strncpy(cmd,recvBuff+cutstart,cutend);
	cmd[cutend]=0;
	strcpy(recvBuff+cutstart,recvBuff+cutstart+cutend);
        
        printf("Received - command = %s\n",cmd);
        if (strcmp(recvBuff, "") != 0 && strstr(recvBuff, id_string) != NULL)
        {
            if (strcmp(cmd,STATUS) == 0)
            {
                sprintf(data, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",flag[0], flag[1], flag[2], flag[3], flag[4], flag[5], flag[6], flag[7], flag[8],flag[9], flag[10], flag[11], flag[12], flag[13], flag[14]);
                write(sockfd, data, strlen(data) + 1);
                printf("replied\n");
            }
            else if(strcmp(cmd,ARM) == 0)
            {
                if(isArmed)
                {
                    write(sockfd, ALREADYARM, 12);
                }
                else
                {
                    isArmed= true;
                    system(ARM_CMD);
       
                    write(sockfd, ACKNOWLEDGE, 12);
                }
            }
            else if(strcmp(cmd,DISARM)==0)
            {
                if(!isArmed)
                {
                    write(sockfd, ALREADYDISARM, 12);
                }
                else
                {
                    isArmed= false;
                    system(DISARM_CMD);
                    write(sockfd, ACKNOWLEDGE, 12);
                }
            }
            else
            {
                if(!isArmed)
                {
                    pthread_mutex_lock(&lock);
		    cmdLen = strlen(cmd);
		    if(strstr(cmd,s) != NULL && cmd[cmdLen-1] != '-')
		    {
		            toggle = strtok(cmd, s);
		            switch_num = atoi(toggle);
		            
		            toggle = strtok(NULL, s);
		            action = atoi(toggle);
		    
		    }
		    else
		    {
		    	noAction =1;
		    }
		    
                    if(noAction==1)
                    {
                    	write(sockfd, NOACTIONASKED, 14);
                    	noAction=0;
                    }                   
                    else if(switch_num >=15 || strcmp(cmd,"")==0)
                    {
                        write(sockfd, NOSUCHSWITCH, 13);
                    }             
                    else if(action != 1 && action != 0)
                    {
                        write(sockfd, INVALIDACTION, 14);
                    }
                    else if (action == 1 || action == 0 )
                    {
                        gpio_write(switch_num,action);
                        
                        if(action == 1)
                        {
                        	write(sockfd, TURNEDON, 9);
                        }	
                        else
                        {
                        	write(sockfd, TURNEDOFF, 10);
                        }

                    }
                    else
                    {
                        write(sockfd, RECVFAILEDD, 12);
                    }

                    pthread_mutex_unlock(&lock);
                }
                else
                {
                    write(sockfd, SYSARMED, 12);
                }
            }
        }
        else
        {
            write(sockfd, RECVFAILEDD, 12);
        }
    }
}

void incase_of_sig()
{
    gpio_write(15,0);
    exit(0);
}

void gpio_write(int pin,int value)
{
    int us[15]={ 0, };
    FILE *f_usage;
    int gpio_pins_write[16]={35,49,131,133,135,137,139,195,196,50,107,109,111,48,47,132};
    char writefile[128]="";
    
    if(value != 1 && value != 0)
    {
        return;
    }

    
    flag[pin]=value;

    FILE *f_stat=fopen(STAT_FILE,"w");
    fprintf(f_stat,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8],flag[9], flag[10], flag[11], flag[12], flag[13], flag[14]);
    fclose(f_stat);

    if(value==1)
    {
    	sprintf(writefile,"echo 1 > /sys/class/gpio/gpio%d/value",gpio_pins_write[pin]);
    	system(writefile);	
        begin[pin] = time(NULL);
    }
    else
    {     
        sprintf(writefile,"echo 0 > /sys/class/gpio/gpio%d/value",gpio_pins_write[pin]);
    	system(writefile);
        end[pin] = time(NULL);

        f_usage = fopen(USAGE_FILE, "r");
        fscanf(f_usage, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &us[0],&us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8],&us[9],&us[10], &us[11], &us[12], &us[13], &us[14]);
        fclose(f_usage);

        if(begin[pin] != 0)
        {
            us[pin] = us[pin] + (end[pin] - begin[pin]);
        }

        f_usage = fopen(USAGE_FILE, "w");
        fprintf(f_usage,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8],us[9],us[10],us[11],us[12],us[13],us[14]);
        fclose(f_usage);
    }
}

void load_preValues()
{
    int i=0;
    FILE *f_stat=fopen(STAT_FILE,"r");
    fscanf(f_stat,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&flag[0],&flag[1],&flag[2],&flag[3],&flag[4],&flag[5],&flag[6],&flag[7],&flag[8],&flag[9],&flag[10],&flag[11],&flag[12],&flag[13],&flag[14]);
    fclose(f_stat);

    for (i = 0; i < 15; i++)
    {
        if(flag[i]==1)
        {
            gpio_write(i,1);
        }
    }
}

void *gpio_read()
{
    int gpio_pins_read[15] = {100 ,102 ,106, 108, 110 ,114, 116, 118, 120 ,123, 34 ,99 ,101,103 ,129 };
    int j=0,i;

    while(1)
    {
        for (j = 0; j < 15; j++)
        {
    	    char readfile[128]="";
    	    /*sprintf(readfile,"/sys/class/gpio/gpio%d/value",gpio_pins_read[j]);
            FILE *file = fopen(readfile,"r");
            fscanf(file,"%d",&i);
            if(i==1)
            {
                if(flag[j]==1)
                {
                    gpio_write(j,0);
                }
                else
                {
                    gpio_write(j,1);
                }
            }
            fclose(file);
            usleep(100);*/
        }
        sleep(2);
    }
}

int main(int argc, char *argv[])
{

    pthread_t pt1;

    signal(SIGSEGV, incase_of_sig);
    signal(SIGINT, incase_of_sig);
    signal(SIGPIPE, incase_of_sig);

    pthread_create(&pt1, NULL, gpio_read, NULL);
 
    if( access( ARM_FILE, F_OK ) != -1 )
    {
        isArmed = true;
    }

    load_preValues();
    gpio_write(15,1);
    start_socket();

    pthread_join(pt1,NULL);
    return 0;
}
