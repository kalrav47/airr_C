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
    int id,sockfd = 0,length,switch_num,cmdLen;
    char recvBuff[20],data[25],id_string[11],cmd[10];
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

    // tell server that this is switchboard not the ordinary client
    // so that server will keep on listening
    write(sockfd, MY_TYPE, 12);

    // wait for acknowledge.
    read(sockfd, recvBuff, 13);

    // give server our id so that it can remeber us and gives us
    // command which is request to this switchboard.
    write(sockfd, id_string, 12);

    // wait for acknowledge.
    read(sockfd, recvBuff, 13);

    while (1)
    {
	// clear our buffers
        strcpy(recvBuff, "");
        strcpy(data,"");

        // read command
        read(sockfd, recvBuff, 20);
        length = strlen(recvBuff);

        // make sure command has our id in it and command is not an empty
        if (strcmp(recvBuff, "") != 0 && strstr(recvBuff, id_string) != NULL && length<COMMAD_MIN_LENGTH)
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
                write(sockfd, data,16);
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

                    write(sockfd, data, 20);
                }

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
                        write(sockfd, INVALIDACTION, 14);
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

void incaseOfSignal()
{
    // turn of the status led and exit
    gpioWrite(15,0);
    exit(0);
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
	// Told to on the led. write to appropriate sys file and save the current time
        sprintf(writefile,"echo 1 > /sys/class/gpio/gpio%d/value",gpio_pins_write[pin]);
        system(writefile);
        begin[pin] = time(NULL);
    }
    else
    {
	// Told to off the led. write to appropriate sys file and get the current time
	// differentiate current time with start time and update the usage file.
        sprintf(writefile,"echo 0 > /sys/class/gpio/gpio%d/value",gpio_pins_write[pin]);
        system(writefile);
        end[pin] = time(NULL);

        f_usage = fopen(USAGE_FILE, "r");
        fscanf(f_usage, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &us[0],&us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8],&us[9],&us[10], &us[11], &us[12], &us[13], &us[14]);
        fclose(f_usage);

        if(begin[pin] != 0)
        {
            us[pin] = us[pin] + (end[pin] - begin[pin]);
            begin[pin]=0;
        }

        f_usage = fopen(USAGE_FILE, "w");
        fprintf(f_usage,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8],us[9],us[10],us[11],us[12],us[13],us[14]);
        fclose(f_usage);
    }

    // write to stat file
    FILE *f_stat=fopen(STAT_FILE,"w");
    fprintf(f_stat,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8],flag[9], flag[10], flag[11], flag[12], flag[13], flag[14]);
    fclose(f_stat);
}

void loadPreValues()
{
    int i=0;

    // read the stat and turn on/off leds accordingly
    FILE *f_stat=fopen(STAT_FILE,"r");
    fscanf(f_stat,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",&flag[0],&flag[1],&flag[2],&flag[3],&flag[4],&flag[5],&flag[6],&flag[7],&flag[8],&flag[9],&flag[10],&flag[11],&flag[12],&flag[13],&flag[14]);
    fclose(f_stat);

    for (i = 0; i < 15; i++)
    {
        if(flag[i]==1)
        {
            gpioWrite(i,1);
        }
        else
        {
            gpioWrite(i,0);
        }
    }
}

void *gpioRead()
{
    int j=0,i;

    while(1)
    {
        for (j = 0; j < 15; j++)
        {
            char readfile[128]="";
            // read file and if there is touch then toggle the switch i.e if it is on then turn it off.
            sprintf(readfile,"/sys/class/gpio/gpio%d/value",gpio_pins_read[j]);
            FILE *file = fopen(readfile,"r");
            fscanf(file,"%d",&i);
            if(i==1)
            {
                if(flag[j]==1)
                {
                    gpioWrite(j,0);
                }
                else
                {
                    gpioWrite(j,1);
                }
                sleep(3);
            }
            fclose(file);
        }
        sleep (0.5);
    }
}

int main(int argc, char *argv[])
{
    pthread_t gpioReadThread;

    // resgister for the signals
    signal(SIGSEGV, incaseOfSignal);
    signal(SIGINT, incaseOfSignal);
    signal(SIGPIPE, incaseOfSignal);

    // start reading touch switches
    pthread_create(&gpioReadThread, NULL, gpioRead, NULL);

    // check if it was armed already
    if( access( ARM_FILE, F_OK ) != -1 )
    {
        isArmed = true;
    }

    // load the pre values
    loadPreValues();

    // turn on the status led as we are running now.
    gpioWrite(15,1);

    // starts the socket and start serving commands
    startSocketAndServeCommands();

    // wait for thread to complete
    pthread_join(gpioReadThread,NULL);

    return 0;
}
