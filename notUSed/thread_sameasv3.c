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

struct val {
    int fd_;
    int pin_;
    int ld_;
};

struct val *values_read[10], *values_write[10];

pthread_t pt1, pt2, pt3, pt4,pt_gpio_read[10], pt_gpio_write[10];
pthread_mutex_t lock;

time_t start[9] = { 0, }, stop[9] = {
0,};

int fd, c;
//GPIO_USER_DATA gpio_data;

int flag[10] = { 0, };
int us[9] = { 0, };

void *fun1();
void *fun2();
void *fun3();
void *fun4();

void *gpio_read(void *);
void gpio_write(int,int);

FILE *fpp, *f_usage, *f_stat;
char id[11];
int sockfd = 0, n = 0, i;
char recvBuff[4096];
struct sockaddr_in serv_addr;

char ip[15];

int ERR = 0;


void incase_of_sig()
{

    f_usage = fopen("usage.txt", "w");
    fprintf(f_usage, "%d %d %d %d %d %d %d %d %d", us[0], us[1], us[2], us[3], us[4], us[5], us[6], us[7], us[8]);
    fclose(f_usage);
    exit(0);

}

int main(int argc, char *argv[])
{
    //	signal(SIGSEGV, incase_of_sig);
    signal(SIGINT, incase_of_sig);
    signal(SIGPIPE, incase_of_sig);

    f_stat=fopen("stat.txt","r");
       fscanf(f_stat,"%d %d %d %d %d %d %d %d %d",&flag[0],&flag[1],&flag[2],&flag[3],&flag[4],&flag[5],&flag[6],&flag[7],&flag[8]);
       fclose(f_stat);

    pthread_create(&pt1, NULL, fun1, NULL);
    pthread_create(&pt2,NULL,fun2,NULL);  
    pthread_create(&pt3,NULL,fun3,NULL);
    //pthread_create(&pt4,NULL,fun4,NULL);
    pthread_exit(NULL);
    return 0;
}

void *fun1()
{

    int ld = 0;
    fpp = fopen("/home/kalrav/id.txt", "r");
    fscanf(fpp, "%s", id);

    memset(recvBuff, '0', sizeof(recvBuff));
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\n inet_pton error occured\n");
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        incase_of_sig();
    }

    char data[25];

    write(sockfd, "SWITCHBOARD", 12);
    read(sockfd, recvBuff, 50);
    write(sockfd, "1234567890", 12);
    read(sockfd, recvBuff, 50);

    while (1) {
        strcpy(recvBuff, "");
        read(sockfd, recvBuff, 50);
        printf("kalrav %s",rcbuff);
        if (strcmp(recvBuff, "") != 0) {

            printf("aa strlen avyu %d %s\n", strlen(recvBuff), id);
            if (strstr(recvBuff, id) != NULL && strlen(recvBuff) == 11) {
                if (recvBuff[10] == 'R') {
                    sprintf(data, "%s%d%d%d%d%d%d%d%d%d", id, flag[0], flag[1],
                            flag[2], flag[3], flag[4], flag[5], flag[6], flag[7], flag[8]);
                    write(sockfd, data, strlen(data) + 1);
                } else if (recvBuff[10] == 'U') {
                    pthread_mutex_lock(&lock);
                    f_usage = fopen("usage.txt", "r");
                    fscanf(f_usage, "%d %d %d %d %d %d %d %d %d", &us[0],
                           &us[1], &us[2], &us[3], &us[4], &us[5], &us[6], &us[7], &us[8]);

                    sprintf(data, "%s %d %d %d %d %d %d %d %d %d", id, us[0],
                            us[1], us[2], us[3], us[4], us[5], us[6], us[7], us[8]);
                    write(sockfd, data, strlen(data) + 1);
                    fclose(f_usage);
                    pthread_mutex_unlock(&lock);
                } else if (recvBuff[10] == 'E') {
                    f_usage = fopen("usage.txt", "w");
                    const char *textt = "0 0 0 0 0 0 0 0 0";
                    fprintf(f_usage, "%s", textt);
                    printf("last E\n");
                    fclose(f_usage);
                } else {
                    pthread_mutex_lock(&lock);
                    ld = recvBuff[10] - 48;

                    if (flag[ld] == 1) {
                        write(sockfd, "ACKNOWLEDGE", 12);
                        gpio_write(ld,0);

                    } else if (flag[ld] == 0) {

                        gpio_write(ld,1);
                        write(sockfd, "ACKNOWLEDGE", 12);
                    } else {
                        write(sockfd, "RECVFAILEDD", 12);
                    }
                    pthread_mutex_unlock(&lock);
                    printf("flag[%d] =  %d\n", ld, flag[ld]);
                }
            }

        }

    }
    if (n < 0) {
        printf("\n Read error \n");
    }

}

void *fun2()
{
int gpio_pins_read[9] = { 18, 6, 13, 19, 26, 12, 16, 20, 21 };

    int j=0;
int val = 0;
	while(1){
    for (j = 0; j < 9; j++) {
    
    	FILE *file = fopen("stat.txt","r");
    	fscanf(file,"%d",&i);
    	if(i==1242354)
    	{
    		if(flag[j]==1){gpio_write(j,0);}
    		else {gpio_write(j,1);}
    	}
    	fclose(file);
    	usleep(100);
      
    }sleep(1);

 }

    close(fd);
}

void *fun3()
{
int i=0;
	while(1)
		{
			
			f_stat=fopen("stat.txt","w");
		        fprintf(f_stat,"%d %d %d %d %d %d %d %d %d",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8]);
        		fclose(f_stat);	
			/*f_usage=fopen("usage.txt","r+");
                                                        fscanf(f_usage,"%d %d %d %d %d %d %d %d %d",&us[0],&us[1],&us[2],&us[3],&us[4],&us[5],&us[6],&us[7],&us[8]);
                                                
                                                for(i=0;i<9;i++)
							if(flag[i]==1){
							
							u[i]++;
							}
							
                                                        fprintf(f_usage,"%d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8]);
                                                        fclose(f_usage);*/
			
			sleep(1);

		}
}

/*void *fun4()
{
	char rcbuff[100]="";
	int sockfdd;
	while(1)
	{
	    if((sockfdd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	    {
	        printf("\n Error : Could not create socket \n");
        	incase_of_sig();
    	    } 
		
	struct timeval tv;	
	tv.tv_sec=2;
	if(setsockopt(sockfdd,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval))) {
		incase_of_sig(); }

    		memset(&serv_addr, '0', sizeof(serv_addr)); 

    		serv_addr.sin_family = AF_INET;
    		serv_addr.sin_port = htons(5000); 

  	  if(inet_pton(AF_INET,"192.168.1.2", &serv_addr.sin_addr)<=0)
    		{
       			 printf("\n inet_pton error occured\n");
        		incase_of_sig();
    		}

   	 if( connect(sockfdd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    		{
       			printf("\n Error : Connect Failed \n");
       			incase_of_sig();
   		 }
			pinMode(27,OUTPUT);
			digitalWrite(27,HIGH); 
	
		if(send(sockfdd,"HEARTBEATCK",12,MSG_NOSIGNAL) <= 0 )incase_of_sig();
		usleep(50);

		if(read(sockfdd,rcbuff,100)>0)
		{
			digitalWrite(27,HIGH);
		}
		else
		
		{
			digitalWrite(27,LOW);
			incase_of_sig();
  		}
		sleep(1);	
	}
}*/

void gpio_write(int pin,int value)
{
	int gpio_pins_write[9]={23,3,4,17,8,22,10,9,5};
	flag[pin]=value;
}

