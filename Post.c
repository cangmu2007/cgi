#include "head.h"

int main(int argc, char *argv[])
{
	char *lenmsg = getenv("CONTENT_LENGTH");
	int i,plen,ret=-1;
	printf("\n");

	if((lenmsg == NULL) || ((plen = atoi(lenmsg)) <= 0))
	{
		printf("ERROR!");
		return -1;
	}

	char* data = (char*)malloc(plen);
	fread(data,1,plen,stdin);
	data[plen] = 0;
	fflush(stdin);

	char* ptr = strstr(data, "context=");
	int len=plen-(ptr-data)-8;

	CM cm=(CM)malloc(sizeof(CGI_MSG)+len);
	if(cm==NULL)
	{
		printf("ERROR!");
		exit(-1);
	}
	memset(cm,0,sizeof(CGI_MSG)+len);
	cm->packet_len=sizeof(CGI_MSG)+len;
	cm->len=len;

	if(sscanf(data, "%*[^=]=%d%*[^=]=%[^&]%*[^=]=%[^&]", &cm->type, cm->sender, cm->recver) != 3)
	{
		printf("ERROR!");
		exit(-1);
	}

	memcpy(cm->context,data+(plen-len),len);
	free(data);
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
