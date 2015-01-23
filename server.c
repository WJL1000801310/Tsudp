/********************************************************************/
/*                                                                  */
/*  FILE NAME             :  server.c                               */
/*  PRINCIPAL AUTHOR      :  wujianlu                               */
/*  SUBSYSTEM NAME        :  DB                                     */
/*  MODULE NAME           :  server                                 */
/*  LANGUAGE              :  C                                      */
/*  TARGET ENVIRONMENT    :  Linux                                  */
/*  DATE OF FIRST RELEASE :  2015/01/20                             */
/*  DESCRIPTION           :  The server of translate 1G file        */
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

#define SERVER_PORT 2500
#define BUFFER_SIZE 4096
/* package head */
typedef struct
{
	int id;
	int buf_size;
}PackInfo;

/* package send */
struct Pack
{
	PackInfo head;
	char buf[BUFFER_SIZE];
} data;
PackInfo pack_info;
PackInfo file_inf;

int main()
{
	int filesize;
	int id = 1;
        char file_name[] = "test.img";
	char buffer[BUFFER_SIZE];
	char md5_sum[MD5_LEN + 1];
        time_t t_start,t_end;
	struct sockaddr_in servaddr,clieaddr;
        socklen_t length = sizeof(clieaddr);
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		perror("Create Socket Failed:");
		exit(1);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERVER_PORT);
	if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
	{
		perror("Server Bind Failed:");
		exit(1);
	}					
        FILE *fp = fopen(file_name, "w");
        if(NULL == fp)
	{
		printf("File:\t%s Can Not Open To Write\n", file_name); 
		exit(1);
	}
	/*receive file information*/
	if(recvfrom(sockfd, (char*)&file_inf, sizeof(file_inf), 0, 
			           (struct sockaddr*)&clieaddr,&length) < 0)
	{
		perror("receive file information Failed:");
		exit(1);
	}
	printf("---------filesize:%d;maxID:%d----------\n",file_inf.buf_size,file_inf.id);
        printf("-------Begin receiving!-------\n");
	/* receive file from client and write */
        t_start=time(NULL);
	while(id <= file_inf.id)
	{
			
		if(recvfrom(sockfd, (char*)&data, sizeof(data), 0, 
				(struct sockaddr*)&clieaddr,&length) < 0)
		{
			perror("receive file  Failed:");
			exit(1);
		}
		else
		{
			if(data.head.id == id)
			{
				pack_info.id = data.head.id;
				pack_info.buf_size = data.head.buf_size;
				++id;
				/* send confirm information */
				if(sendto(sockfd, (char*)&pack_info, sizeof(pack_info), 0, 
					(struct sockaddr*)&clieaddr, length) < 0)
				{
					printf("Send confirm information failed!");
				}
				if(fwrite(data.buf, sizeof(char), data.head.buf_size, fp) < data.head.buf_size)
				{
					printf("File:\t%s Write Failed\n", file_name);
					break;
				}
			}
			else if(data.head.id < id)  /* resend package */
			{
				pack_info.id = data.head.id;
				pack_info.buf_size = data.head.buf_size;
                                /* send confirm information */
				if(sendto(sockfd, (char*)&pack_info, sizeof(pack_info), 0, 
					(struct sockaddr*)&clieaddr, length) < 0)
				{
					printf("Send confirm information failed!");
				}
			}
		}
	}
        t_end=time(NULL);
    	printf("----------use time:%.0fs-----------\n",difftime(t_end,t_start));
	/*receive filesize*/
	filesize=ftell(fp);		
	printf("--------receive filesize=%d----------\n",filesize);
	fclose(fp);
	bzero(md5_sum,MD5_LEN+1);//calculate mds
        if(!CalcFileMD5(file_name, md5_sum))
       	{
       		puts("Error occured!");
  	}
  	printf("---------the server MD5 is :%s -----------\n", md5_sum);
	bzero(buffer, BUFFER_SIZE);
	if(recvfrom(sockfd, buffer, BUFFER_SIZE,0,(struct sockaddr*)&clieaddr, &length) < 0)//receive client file md5
	{
		perror("Receive md5 Data Failed:");
		exit(1);
	}
	else
	{
		printf("--------the client md5 is :%s--------\n", md5_sum);
	}		
	if(strcmp(buffer,md5_sum) == 0)//compare md5
       	{
	        printf("---------File:%s Transfer Successful!--------\n", file_name);
       	}
 	else
	{
		printf("----------File:%s Transfer Wrong!---------\n", file_name);
	}						
	close(sockfd);
	return 0;
}
