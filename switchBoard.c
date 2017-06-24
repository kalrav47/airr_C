// Compile for PC : gcc switchBoard.c -o switchBoard -lpthread
// Compile for CHIP : ~/CHIP-SDK/CHIP-buildroot/output/host/usr/bin/arm-linux-gnueabihf-gcc switchBoard.c -o switchBoard -lpthread -w

// Header file
#include "switchBoard.h"


// Global variables
int flag[15] = { 0, };
time_t begin[15] = { 0, };
time_t end[15]= { 0, };
static bool isArmed = false;

int gpio_pins_write[16]= {35,49,131,133,135,137,139,195,196,50,107,109,111,48,47,132};
int gpio_pins_read[15] = {100 ,102 ,106, 108, 110 ,114, 116, 118, 120 ,123, 34 ,99 ,101,103 ,129};

void startSocketAndServeCommands()
{
    int id,sockfd = 0,length,switch_num,cmdLen,j;
    char recvBuff[MAX_CMD],data[25],id_string[11],cmd[10];
    struct sockaddr_in serv_addr;
    const char s[2] = "-";
    char *toggle;
    int noAction = 0;
    int us[15]= { 0, };
    FILE *f_usage;

    char cutstart;
    int cutend;
    int action = 2;

    // read id of the switchboard
    FILE *file = fopen(ID_FILE, "r");
    fscanf(file, "%d", &id);
    sprintf(id_string,"%d",id);

    memset(recvBuff, '0', sizeof(recvBuff));

    // create socket to server
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        incaseOfSignal();
    }

    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        incaseOfSignal();
    }

    // conencts to server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\n Error : Connect Failed \n");
        incaseOfSignal();
    }
#ifdef DEBUG
	    printf("Writing type to server\n");
#endif
    // tell server that this is switchboard not the ordinary client
    // so that server will keep on listening
    write(sockfd, MY_TYPE, strlen(MY_TYPE)+1);
#ifdef DEBUG
	    printf("Writing done\n");
#endif
    // wait for acknowledge.
    read(sockfd, recvBuff, MAX_CMD);

#ifdef DEBUG
	    printf("Got ack.. '%s' from server\n",recvBuff);
#endif
    // give server our id so that it can remeber us and gives us
    // command which is request to this switchboard.
    write(sockfd, id_string, strlen(id_string)+1);
#ifdef DEBUG
	    printf("Writing id done\n");
#endif
    // wait for acknowledge.
    read(sockfd, recvBuff, MAX_CMD);
#ifdef DEBUG
	    printf("Got ack.. '%s' from server\n",recvBuff);
#endif
    while (1)
    {
    	// sync once before next command. Better to sync
    	system(SYNC);
	// clear our buffers
        strcpy(recvBuff, "");
        strcpy(data,"");

#ifdef DEBUG
        printf("Waiting for command...\n");
#endif
        // read command
        read(sockfd, recvBuff, MAX_CMD);
        length = strlen(recvBuff);

#ifdef DEBUG
        printf("Got command : %s\n",recvBuff);
#endif

        // make sure command has our id in it and command is not an empty
        if (strcmp(recvBuff, "") != 0 && strstr(recvBuff, id_string) != NULL)
        {
            // parse the command
            cutstart=10;
            cutend = length-cutstart;

            strncpy(cmd,recvBuff+cutstart,cutend);
            cmd[cutend]=0;
            strcpy(recvBuff+cutstart,recvBuff+cutstart+cutend);

            if (strcmp(cmd,STATUS) == 0)
            {
                sprintf(data, "%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",flag[0], flag[1], flag[2], flag[3], flag[4], flag[5], flag[6], flag[7], flag[8],flag[9], flag[10], flag[11], flag[12], flag[13], flag[14]);
                data[15]='\0';
                write(sockfd, data,strlen(data)+1);
            }
            else if(strcmp(cmd,RESETUSAGE) == 0)
            {
            	// reset usage. Simply set all zeros in file
            	sprintf(data,"echo \"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\" > %s",USAGE_FILE);
            	system(data);
            	system(SYNC);
            	write(sockfd, RESETUSAGEDONE, 15);

            }
            else if(strcmp(cmd,HEARTBEAT) == 0)
            {
            	write(sockfd,IAMTHERE4U, 11);
            }
            else if(strcmp(cmd,RESETSTAT) == 0)
            {
            	// just delete respective marker files. Else will be taken care by gpioRead function
            	for (j = 0; j < 15; j++)
        	{
        		sprintf(data,"rm /root/.%d",gpio_pins_write[j]);
       			system(data);
        	}
        	system(SYNC);
            	write(sockfd, RESETSTATDONE,14);
            }
            else if(strstr(cmd,USAGE) != NULL)
            {
                cmdLen = strlen(cmd);
                if(strstr(cmd,s) != NULL && cmd[cmdLen-1] != '-')
                {
                    toggle = strtok(cmd, s);

                    if(strcmp(toggle,USAGE)!=0)
                    {
                        switch_num = -1;
                    }
                    else
                    {
                        toggle = strtok(NULL, s);
                        if(toggle[0] >='0' && toggle[0] <='9')
                        {
                            switch_num = atoi(toggle);
                        }
                        else
                        {
                            switch_num = -1;
                        }
                    }

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
                else if(switch_num >=15 || strcmp(cmd,"")==0 || switch_num == -1)
                {
                    write(sockfd, NOSUCHSWITCH, 13);
                }
                else
                {
                    f_usage = fopen(USAGE_FILE, "r");
                    fscanf(f_usage, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &us[0],&us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8],&us[9],&us[10], &us[11], &us[12], &us[13], &us[14]);
                    fclose(f_usage);

                    sprintf(data,"%d",us[switch_num]);

                    write(sockfd, data, strlen(data)+1);
                }

            }
            else if(strcmp(cmd,ARM) == 0)
            {
                if(isArmed)
                {
                    write(sockfd, ALREADYARM, 11);
                }
                else
                {
                    isArmed= true;
                    system(ARM_CMD);
		    system(SYNC);
                    write(sockfd, ACKNOWLEDGE,12);
                }
            }
            else if(strcmp(cmd,DISARM)==0)
            {
                if(!isArmed)
                {
                    write(sockfd, ALREADYDISARM,14);
                }
                else
                {
                    isArmed= false;
                    system(DISARM_CMD);
                    system(SYNC);
                    write(sockfd, ACKNOWLEDGE, 12);
                }
            }
            else
            {
                if(!isArmed)
                {
                    cmdLen = strlen(cmd);
                    if(strstr(cmd,s) != NULL && cmd[cmdLen-1] != '-')
                    {
                        toggle = strtok(cmd, s);

                        if(toggle[0] >='0' && toggle[0] <='9')
                        {
                            switch_num = atoi(toggle);
                        }
                        else
                        {
                            switch_num = -1;
                        }

                        toggle = strtok(NULL, s);

                        if(toggle[0] >='0' && toggle[0] <='9')
                        {
                            action = atoi(toggle);
                        }
                        else
                        {
                            noAction =1;
                        }
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
                        write(sockfd, INVALIDACTION,14);
                    }
                    else if (action == 1 || action == 0 )
                    {
                        gpioWrite(switch_num,action);

                        if(switch_num == -1)
                        {
                            write(sockfd, NOSUCHSWITCH, 13);
                        }
                        else if(action == 1)
                        {
                            write(sockfd, TURNEDON, 9);
                        }
                        else
                        {
#ifdef DEBUG
        printf("returning to command : %s\n",TURNEDOFF);
#endif
                            write(sockfd, TURNEDOFF, 10);
                        }

                    }
                    else
                    {
                        write(sockfd, RECVFAILEDD, 12);
                    }
                }
                else
                {
                    write(sockfd, SYSARMED, 9);
                }
            }
        }
        else
        {
            write(sockfd, RECVFAILEDD, 11);
        }
    }
}

void incaseOfSignal()
{
    saveUsage();
    // turn of the status led and exit
    gpioWrite(15,0);
    exit(0);
}

void saveUsage()
{
	int j;
	int us[15]= { 0, };
	FILE *f_usage;
	
	for (j = 0; j < 15; j++)
        {
        	if(flag[j]==1)
        	{
        		    end[j] = time(NULL);
    	    		     
    	    		     f_usage = fopen(USAGE_FILE, "r");
			     fscanf(f_usage, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &us[0],&us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8],&us[9],&us[10], &us[11], &us[12], &us[13], &us[14]);
			     fclose(f_usage);

			    if(begin[j] != 0)
			    {
			       us[j] = us[j] + (end[j] - begin[j]);
			       begin[j]=0;
			    }

			    f_usage = fopen(USAGE_FILE, "w");
			    fprintf(f_usage,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8],us[9],us[10],us[11],us[12],us[13],us[14]);
			    fclose(f_usage);
        	}	
        }
}

void gpioWrite(int pin,int value)
{
    int us[15]= { 0, };
    FILE *f_usage;
    char writefile[128]="";

    // check that value is valid
    if(value != 1 && value != 0)
    {
        return;
    }

    // update the flag
    flag[pin]=value;

    if(value==1)
    {
	// Told to on the led. create appropriate marker file.
        sprintf(writefile,"touch /root/.%d",gpio_pins_write[pin]);
        system(writefile);
        system(SYNC);
    }
    else
    {
	// Told to off the led. delete appropriate tmp file.
        sprintf(writefile,"rm /root/.%d",gpio_pins_write[pin]);
        system(writefile);
        system(SYNC);
    }
}

void *heartBeatCheck()
{ 
    int internal_client_socketid = 0;
    char temp[MAX_CMD];
    struct sockaddr_in serv_addr;
    long arg;
    
    //wait for 5 seconds before we start
    sleep(5);
    
    while(1)
    {
    
    strcpy(temp,"");
    
    // set alarm of 5 seconds
    alarm (5);
    
    // create socket to internal server
    if((internal_client_socketid = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        incaseOfSignal();
    }
#ifdef DEBUG
        printf("HeartBeatCheck : socket created !\n");
#endif
    
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET,IP,&serv_addr.sin_addr)<=0)
    {
#ifdef DEBUG
        printf("HeartBeatCheck : while creating socket\n");
#endif
        incaseOfSignal();
    }
  
#ifdef DEBUG
        printf("HeartBeatCheck : connecting.. !\n");
#endif

    // connect to internal server
    if( connect(internal_client_socketid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
#ifdef DEBUG
        printf("HeartBeatCheck : while connecting...\n");
#endif
        incaseOfSignal();
    }
#ifdef DEBUG
        printf("HeartBeatCheck : sending... !\n");
#endif   
  
    if(send(internal_client_socketid,HEARTBEAT,15,MSG_DONTWAIT) == 11)
    {
#ifdef DEBUG
        printf("HeartBeatCheck : while sending to socket\n");
#endif
    	incaseOfSignal();
    }
    
    read(internal_client_socketid,temp,MAX_CMD);
    
    if(strcmp(temp,IAMTHERE4U) != 0)
    {
#ifdef DEBUG
        printf("HeartBeatCheck : while receving socket, reply = %s\n",temp);
#endif
    	incaseOfSignal();
    }
    
    close(internal_client_socketid);
    
    // disable alarm
    alarm (0);
    
#ifdef DEBUG
        printf("HeartBeatCheck : ping success !\n");
#endif
    sleep(5);
   }
}

void *gpioRead()
{
    int inotifyFd;
    int watchList;
    int length;
    int i,j;
    int us[15]= { 0, };
    FILE *f_usage;
    char fileName[10]="";
    char buffer[EVENT_BUF_LEN];
    
    // init inotify
    inotifyFd  = inotify_init();
    
    if(inotifyFd<0)
    {
    	incaseOfSignal();
    }

    watchList = inotify_add_watch( inotifyFd,ROOT_DIR, IN_CREATE | IN_DELETE );

#ifdef DEBUG
        printf("file watch thread started ...\n");
#endif

    while(1)
    {
    	i=0;
    	length = read(inotifyFd, buffer, EVENT_BUF_LEN);
    	
    	if(length < 0)
    	{
    	    incaseOfSignal();
    	}
    	
#ifdef DEBUG
        printf("File operation happened\n");
#endif
    	while(i<length)
    	{
    	    struct inotify_event *event = (struct inotify_event *) &buffer[i];
    	    strcpy(fileName,event->name);
    	    int len = strlen(fileName);
#ifdef DEBUG
        printf("File operation on %s detected\n");
#endif
    	    if (fileName[0]=='.')
    	    {
    	    	for(j=0;j<16;j++)
    	    	{
    	    	     char readfile[10]="";
    	    	     sprintf(readfile,".%d",gpio_pins_write[j]);
    	    	     
    	    	     if(strcmp(readfile,fileName) == 0)
    	    	     {
    	    	     	if (event->mask & IN_CREATE)
    	    		{
#ifdef DEBUG
        printf("File creation detected\n");
#endif
    	    		     flag[j]=1;
    	    		     begin[j] = time(NULL);
    	    		}
    	    
    	    		if (event->mask & IN_DELETE)
    	    		{
#ifdef DEBUG
        printf("File deletion detected.. updating usage\n");
#endif
    	    		     flag[j]=0;
    	    		     end[j] = time(NULL);
    	    		     
    	    		     f_usage = fopen(USAGE_FILE, "r");
			     fscanf(f_usage, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &us[0],&us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8],&us[9],&us[10], &us[11], &us[12], &us[13], &us[14]);
			     fclose(f_usage);

			    if(begin[j] != 0)
			    {
			       us[j] = us[j] + (end[j] - begin[j]);
			       begin[j]=0;
			    }

			    f_usage = fopen(USAGE_FILE, "w");
			    fprintf(f_usage,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8],us[9],us[10],us[11],us[12],us[13],us[14]);
			    fclose(f_usage);
#ifdef DEBUG
        printf("usage updation done\n");
#endif
    	    		}
    	    	     }
    	        }
    	    }
    	    
    	    i += EVENT_SIZE + event->len;
    	}
    }
}

int main(int argc, char *argv[])
{
    pthread_t gpioReadThread,heartBeatChecker;

    // resgister for the signals
    signal(SIGSEGV, incaseOfSignal);
    signal(SIGINT, incaseOfSignal);
    signal(SIGPIPE, incaseOfSignal);
    signal(SIGALRM, incaseOfSignal);

    // start reading touch switches
    pthread_create(&gpioReadThread, NULL, gpioRead, NULL);
    
    // start heartbeat checker
    pthread_create(&heartBeatChecker, NULL, heartBeatCheck, NULL);

    // check if it was armed already
    if( access( ARM_FILE, F_OK ) != -1 )
    {
        isArmed = true;
    }
    
    // turn on the status led as we are running now.
    gpioWrite(15,1);

    // starts the socket and start serving commands
    startSocketAndServeCommands();

    // wait for thread to complete
    pthread_join(gpioReadThread,NULL);

    return 0;
}
