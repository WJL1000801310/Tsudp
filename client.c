/********************************************************************/
/*                                                                  */
/*  FILE NAME             :  client.c                               */
/*  PRINCIPAL AUTHOR      :  wujianlu                                */
/*  SUBSYSTEM NAME        :  DB                                     */
/*  MODULE NAME           :  client                                 */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  Linux                                  */
/*  DATE OF FIRST RELEASE :  2015/01/20                             */
/*  DESCRIPTION           :  The client of of translate 1G file     */
/********************************************************************/
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<netdb.h>
#include<stdarg.h>
#include<string.h>
#include<time.h>
#include "MD5.h"

#define SERV_PORT 2500
#define BUFFSIZE 4096
#define MAXLINE 512

/*packeage head */
typedef struct 
{
	int id;
	int buf_size;
}PackInfo;

/* package receive */
struct Pack
{
	PackInfo head;
	char buf[BUFFSIZE];
} data;

PackInfo file_info;
int main()
{
	char file_name[MAXLINE+1];
	char buffer[BUFFSIZE];
	char md5_sum[MD5_LEN + 1];
	int filesize = 0;		
	int send_id = 0;
	int recv_id = 0;
        time_t t_start,t_end;
	struct sockaddr_in servaddr;
        socklen_t length = sizeof(servaddr);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(SERV_PORT);	
	bzero(file_name, MAXLINE+1);
	printf("Please Input File Name : ");
	scanf("%s", file_name);
	
       	if(!CalcFileMD5(file_name, md5_sum))//calculte md5
      	{
       		puts("Error occured!");
      	}
       	printf("-------the md5 of the file %s is:\n----%s----\n",file_name,md5_sum);
	FILE *fp = fopen(file_name, "r");
	fseek(fp,0,SEEK_END);		/*point to the end of the file*/
	filesize=ftell(fp);		/*get the filesize*/
	rewind(fp); 			/*point to the head of the file*/

	file_info.buf_size = filesize;
	file_info.id = filesize/BUFFSIZE;//calculte maxID
	printf("-------filesize:%d , maxID:%d-------\n",file_info.buf_size,file_info.id);
	if(sendto(sockfd, (char*)&file_info, sizeof(file_info), 0 , (struct sockaddr*)&servaddr,length) < 0)
	{
		perror("Send File information Failed.");
		exit(1);
	}
	if(NULL == fp)
	{
		printf("File:%s Not Found.\n", file_name);
	}
	else
	{
                
		int len = 0;
                printf("-------Begin transmission!-------\n");
		/* time start */		
    		t_start=time(NULL);
		while(1)
		{
			PackInfo pack_info;
			if(recv_id == send_id)
			{
				++send_id;
				if((len = fread(data.buf, sizeof(char), BUFFSIZE, fp)) > 0)
				{
					data.head.id = send_id;  /* Used id to tag order */
					data.head.buf_size = len;  /* Record the data length */
					if(sendto(sockfd, (char*)&data, sizeof(data), 0, (struct sockaddr*)&servaddr, length) < 0)
					{
						perror("Send File Failed:");
						break;
					}
					recvfrom(sockfd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&servaddr, &length);
					recv_id = pack_info.id;	
				}
				else
				{
					break;
				}
			}
			else
			{
				if(sendto(sockfd, (char*)&data,sizeof(data),0,(struct sockaddr*)&servaddr, length) < 0)
				{
					perror("Send File Failed:");
					break;
				}
				recvfrom(sockfd, (char*)&pack_info, sizeof(pack_info), 0, (struct sockaddr*)&servaddr, &length);
				recv_id = pack_info.id;	
			}
		}
		/* time end */
    		t_end=time(NULL);
    		printf("----------use time:%.0fs-----------\n",difftime(t_end,t_start));
		fclose(fp);
		/* send client file md5 */
		bzero(buffer, BUFFSIZE);
		strncpy(buffer, md5_sum, strlen(md5_sum)>BUFFSIZE?BUFFSIZE:strlen(md5_sum));	
		if(sendto(sockfd, buffer, BUFFSIZE,0,(struct sockaddr*)&servaddr,length) < 0)
		{
			perror("-------------Send MD5 Failed:--------------");
			exit(1);
		}
			
	}

	close(sockfd);
	return 0;
}
