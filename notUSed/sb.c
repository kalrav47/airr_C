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




struct val{
	int  fd_;
	int pin_;
	int ld_;
};

struct val *values_read[10],*values_write[10];   


pthread_t pt1,pt2,pt3,pt4,pt_gpio_read[10],pt_gpio_write[10];
pthread_mutex_t lock;

time_t start[9]={0,},stop[9]={0,};

int fd, c;
//GPIO_USER_DATA gpio_data;


int flag[10]={0,};
int us[9]={0,};

void *fun1();
void *fun2();
void *fun3();
void *fun4();
void *gpio_read(struct val *);
void *gpio_write(struct val *);




FILE *fpp,*f_usage,*f_stat;
char id[11];
int sockfd = 0, n = 0,i;
char recvBuff[4096];
struct sockaddr_in serv_addr; 

char ip[15];

int ERR=0;


int gpio_pins_read[9]={18,6,13,19,26,12,16,20,21};
int gpio_pins_write[9]={23,3,4,17,8,22,10,9,5};

void incase_of_sig(){

	f_usage=fopen("usage.txt","w");
	fprintf(f_usage,"%d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8]);
        fclose(f_usage);
//	f_stat=fopen("stat.txt","w");
//	fprintf(f_stat,"%d %d %d %d %d %d %d %d %d",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8]);
//	fclose(f_stat);
	exit(0);

}


int main(int argc,char *argv[])
{	
	signal(SIGSEGV,incase_of_sig);
	signal(SIGINT,incase_of_sig);
	signal(SIGPIPE,incase_of_sig);
	
	/*f_stat=fopen("stat.txt","r");
	fscanf(f_stat,"%d %d %d %d %d %d %d %d %d",&flag[0],&flag[1],&flag[2],&flag[3],&flag[4],&flag[5],&flag[6],&flag[7],&flag[8]);
	fclose(f_stat);*/
	
	pthread_create(&pt1,NULL,fun1,NULL);
	//pthread_create(&pt2,NULL,fun2,NULL);	
	//pthread_create(&pt3,NULL,fun3,NULL);
	//pthread_create(&pt4,NULL,fun4,NULL);
	pthread_exit(NULL);
	return 0;
}


void *fun1()
{

	int ld=0;
	strcpy(id,"9876543210");

	memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 
	
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000); 

    if(inet_pton(AF_INET,"127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }\

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       incase_of_sig();
    } 


	char data[25];	

	write(sockfd,"SWITCHBOARD",12);
	read(sockfd, recvBuff,50);
	write(sockfd,"9876543210",12);
	read(sockfd, recvBuff,50);

	while(1) {
	strcpy(recvBuff,"");	
    read(sockfd, recvBuff,50);
	
	  if(strcmp(recvBuff,"") != 0)
		{
			
			printf("aa strlen avyu %d %s \n",strlen(recvBuff),recvBuff,id);
			if(strstr(recvBuff,id) != NULL && strlen(recvBuff)==11 )
				{
					if(recvBuff[10]=='R')
					{
						  sprintf(data,"%s%d%d%d%d%d%d%d%d%d",id,flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8]);
	 	                                  write(sockfd,data,strlen(data)+1);
					}
					else if(recvBuff[10]=='U')
					{
						pthread_mutex_lock(&lock);
						f_usage=fopen("usage.txt","r");
						fscanf(f_usage,"%d %d %d %d %d %d %d %d %d",&us[0],&us[1],&us[2],&us[3],&us[4],&us[5],&us[6],&us[7],&us[8]);
					
						sprintf(data,"%s %d %d %d %d %d %d %d %d %d",id,us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8]);
						write(sockfd,data,strlen(data)+1);
						fclose(f_usage);	
						pthread_mutex_unlock(&lock);
					}
					else if(recvBuff[10]=='E')
					{
					 f_usage=fopen("usage.txt","w");
					const char *textt="0 0 0 0 0 0 0 0 0";
					fprintf(f_usage,"%s",textt);
					printf("last E\n");
					fclose(f_usage);
					} 
					else
					{printf("\n kalrav169 \n");
						pthread_mutex_lock(&lock);
						ld=recvBuff[10]-48;
						printf("\n kalrav172 \n");
						if(flag[ld]==1)
						{
							printf("\n kalrav175 \n");
								write(sockfd,"ACKNOWLEDGE",12);
							 flag[ld]=0;

						}
						else if(flag[ld]==0)
						{	
							printf("\n kalrav1181 \n");
							 flag[ld]=1;
							write(sockfd,"ACKNOWLEDGE",12);
						}
						else
						{
							printf("\n kalrav188 \n");
							write(sockfd,"RECVFAILEDD",12);
						}
						printf("\n kalrav191 \n");
						pthread_mutex_unlock(&lock);		
						printf("flag[%d] =  %d\n",ld,flag[ld]);
					}           
				} 
				else
				{
					write(sockfd,"NOJOBTODO",13);}
	fflush(stdout);

        }
     
}
    if(n < 0)
    {
        printf("\n Read error \n");
    } 

 

}


void *fun2()
{
	

        int gpio_pins_read[9]={18,6,13,19,26,12,16,20,21};
        int gpio_pins_write[9]={23,3,4,17,8,22,10,9,5};   

	
	/*for(i=0;i<9;i++)
	{
		pthread_create(&pt_gpio_read[i],NULL,gpio_read,values_read[i]);
		pthread_create(&pt_gpio_write[i],NULL,gpio_write,values_write[i]);
	}
	
	pthread_exit(NULL);*/
	
	close(fd);
}


/*void *fun3()
{
int i=0;
	while(1)
		{
			f_stat=fopen("stat.txt","w");
		        fprintf(f_stat,"%d %d %d %d %d %d %d %d %d",flag[0],flag[1],flag[2],flag[3],flag[4],flag[5],flag[6],flag[7],flag[8]);
        		fclose(f_stat);
			sleep(1);

			pthread_mutex_lock(&lock);
			f_usage=fopen("usage.txt","r+");
                                                        fscanf(f_usage,"%d %d %d %d %d %d %d %d %d",&us[0],&us[1],&us[2],&us[3],&us[4],&us[5],&us[6],&us[7],&us[8]);
                                                //      printf("aa previous us - %d\n",us[ld]);
                                                for(i=0;i<9;i++)
							if(flag[i]==1){
							stop[i]=time(NULL);
							us[i]=stop[i]-start[i];
							}
							start[i]=time(NULL);
                                                //      printf("start :::: %d stop :::: %d us::::%d\n",start[ld],stop[ld],us[ld]);
                                                        fprintf(f_usage,"%d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8]);
                                                        fclose(f_usage);
			pthread_mutex_unlock(&lock);
			sleep(3);

		}
}*/
	




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





/*void *gpio_read(struct val *temp)
{

	while(1)
	{
			
                pthread_mutex_lock(&lock);

                usleep(10000);

                if(digitalRead(temp->pin_))
                {
                        if(flag[temp->ld_]==1) flag[temp->ld_]=0;
                        else if(flag[temp->ld_]==0)  flag[temp->ld_]=1;
                         usleep(10000);
                }
                pthread_mutex_unlock(&lock);
               sleep(1);
	}
}*/



/*void *gpio_write(struct val *temp)
{


	while(1)
	{

			if(flag[temp->ld_]==1)
        	        {
                        	pthread_mutex_lock(&lock);
				digitalWrite(temp->pin_, HIGH);
				start[temp->ld_]=time(NULL);
                       		 pthread_mutex_unlock(&lock);
			 	 usleep(1000);
               		 }
               		 else
               		 {

                       		 pthread_mutex_lock(&lock);
               		 
				digitalWrite(temp->pin_,LOW);
				stop[temp->ld_]=time(NULL);

				f_usage=fopen("usage.txt","r+");
                                                        fscanf(f_usage,"%d %d %d %d %d %d %d %d %d",&us[0],&us[1],&us[2],&us[3],&us[4],&us[5],&us[6],&us[7],&us[8]);
                         
                                                if(stop[temp->ld_]>start[temp->ld_])us[temp->ld_]+=(stop[temp->ld_]-start[temp->ld_]);
                                               
                                                        fprintf(f_usage,"%d %d %d %d %d %d %d %d %d",us[0],us[1],us[2],us[3],us[4],us[5],us[6],us[7],us[8]);
                                                        fclose(f_usage);


                        	pthread_mutex_unlock(&lock);
				usleep(1000);    
              		  }
			
	}
}*/


