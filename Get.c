#include "head.h"

int main(int argc , char *argv[])
{
	char* data = getenv("QUERY_STRING");
	int i,ret=-1;
	printf("\n");

	char* ptr = strstr(data, "context=");
	int len=strlen(data)-(ptr-data)-7;

	CM cm=(CM)malloc(sizeof(CGI_MSG)+len);
	if(cm==NULL)
	{
		printf("ERROR!");
		exit(-1);
	}
	memset(cm,0,sizeof(CGI_MSG)+len);
	cm->packet_len=sizeof(CGI_MSG)+len;
	cm->len=len-1;

	if(sscanf(data, "%*[^=]=%d%*[^=]=%[^&]%*[^=]=%[^&]%*[^=]=%s", &cm->type, cm->sender, cm->recver, cm->context) != 4)
	{
		printf("ERROR!");
		exit(-1);
	}

	//设备判断
	char* type=getenv("HTTP_USER_AGENT");
	if(0==strcmp(type,"Android"))
	{
		cm->dev_type=ANDROID;
	}
	else if(0==strcmp(type,"iOS"))
	{
		cm->dev_type=IOS;
	}
	else if(0==strcmp(type,"Windows Phone"))
	{
		cm->dev_type=WP;
	}
	else
	{
		cm->dev_type=IGNORE;
	}

	//1、将结构体send发送至服务端
	int fd=-1;
	struct sockaddr_un srv_addr;

	if((fd = socket(PF_UNIX, SOCK_STREAM, 0))<0)
	{
		printf("ERROR!");
		exit(-1);
	}

	memset(&srv_addr,0,sizeof(struct sockaddr_un));
	srv_addr.sun_family = PF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_PATH);

	if(connect(fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un))<0)
	{
		printf("ERROR!");
		exit(-1);
	}

	if((ret=send(fd,(char*)cm,sizeof(CGI_MSG)+len,MSG_NOSIGNAL))<0)
	{
		printf("ERROR!");
		exit(-1);
	}
	free(cm);

	struct timeval timeout;
	timeout.tv_sec=270;   
	timeout.tv_usec=0;
	if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout.tv_sec,sizeof(struct timeval))<0)
	{
		printf("ERROR!");
		exit(-1);
	}

	//2、阻塞接收数据
	int size=-1;
	ret=recv(fd,(char*)&size,sizeof(size),0);
	if(-1==ret)
	{
		printf("ERROR!");
		exit(-1);
	}
	if(0==ret)
	{
		printf("ERROR!");
		exit(0);
	}
	int leftsize=sizeof(char) * size;
	char* ndata=(char*)malloc(leftsize);
	memset(ndata,0,leftsize);

	uint recv_count=0;
	while(recv_count<leftsize)
	{
		ret=recv(fd,ndata+recv_count,leftsize-recv_count,0);
		if(ret==-1)
		{
			printf("ERROR!");
			exit(-1);
		}else if(ret==0)
		{
			break;
		}else
			recv_count+=ret;
	}
	if(recv_count>0)
	{
		fwrite(ndata, recv_count, 1, stdout);
	}else
		printf("ERROR!");

	free(ndata);
	close(fd);
	return ret;
}
