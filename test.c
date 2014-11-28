#include "head.h"

CM get_func()
{
	char* data=getenv("QUERY_STRING");
	char* ptr = strstr(data, "context=");
	uint len=strlen(data)-(ptr-data)-7;

	CM cm=(CM)malloc(sizeof(CGI_MSG)+len);
	if(NULL==cm)
	{
		return NULL;
	}
	memset(cm,0,sizeof(CGI_MSG)+len);
	cm->packet_len=sizeof(CGI_MSG)+len;
	cm->len=len-1;

	if(sscanf(data, "%*[^=]=%d%*[^=]=%[^&]%*[^=]=%[^&]%*[^=]=%s", &cm->type, cm->sender, cm->recver, cm->context) != 4)
	{
		return NULL;
	}
	return cm;
}

CM post_func()
{
	char *lenmsg = getenv("CONTENT_LENGTH");
	int plen;
	printf("\n");

	if((lenmsg == NULL) || ((plen = atoi(lenmsg)) <= 0))
	{
		return NULL;
	}

	char* data = (char*)malloc(plen);
	fread(data,1,plen,stdin);
	data[plen] = 0;
	fflush(stdin);

	char* ptr = strstr(data, "context=");
	uint len=plen-(ptr-data)-8;

	CM cm=(CM)malloc(sizeof(CGI_MSG)+len);
	if(NULL==cm)
	{
		return NULL;
	}
	memset(cm,0,sizeof(CGI_MSG)+len);
	cm->packet_len=sizeof(CGI_MSG)+len;
	cm->len=len;

	if(sscanf(data, "%*[^=]=%d%*[^=]=%[^&]%*[^=]=%[^&]", &cm->type, cm->sender, cm->recver) != 3)
	{
		return NULL;
	}

	memcpy(cm->context,data+(plen-len),len);
	free(data);
	data=NULL;

	return cm;
}

int data_exchange(CM cm)
{
	//1、将结构体send发送至服务端
	int fd=-1,ret=-1;
	struct sockaddr_un srv_addr;

	if((fd = socket(PF_UNIX, SOCK_STREAM, 0))<0)
	{
		return -1;
	}

	memset(&srv_addr,0,sizeof(struct sockaddr_un));
	srv_addr.sun_family = PF_UNIX;
	strcpy(srv_addr.sun_path,UNIX_PATH);

	if(connect(fd, (struct sockaddr *)&srv_addr, sizeof(struct sockaddr_un))<0)
	{
		close(fd);
		return -1;
	}

	if((ret=send(fd,(char*)cm,cm->packet_len,MSG_NOSIGNAL))<0)
	{
		close(fd);
		return -1;
	}

	struct timeval timeout;
	timeout.tv_sec=270;   
	timeout.tv_usec=0;
	if(setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout.tv_sec,sizeof(struct timeval))<0)
	{
		close(fd);
		return -1;
	}

	//2、阻塞接收数据
	int size=-1;
	ret=recv(fd,(char*)&size,sizeof(size),0);
	if(-1==ret)
	{
		close(fd);
		return -1;
	}
	if(size<=0)
	{
		close(fd);
		return -1;
	}
	uint leftsize=sizeof(char) * size;
	char* ndata=(char*)malloc(leftsize);
	if(NULL==ndata)
	{
		close(fd);
		return -1;
	}
	memset(ndata,0,leftsize);

	uint recv_count=0;
	while(recv_count<leftsize)
	{
		ret=recv(fd,ndata+recv_count,leftsize-recv_count,0);
		if(ret==-1)
		{
			free(ndata);
			ndata=NULL;
			close(fd);
			return -1;
		}else if(ret==0)
		{
			break;
		}else
			recv_count+=ret;
	}
	if(recv_count>0)
	{
		fwrite(ndata, recv_count, 1, stdout);
	}
	else
	{
		printf("ERROR!");
	}

	free(ndata);
	ndata=NULL;
	close(fd);
	return 0;
}

int main(void)
{
	int ret=-1;
	CM cm=NULL;
	char* request_method=getenv("REQUEST_METHOD");

	if(0==strcmp(request_method,"GET"))
	{
		cm=get_func();
	}
	else if((0==strcmp(request_method,"POST"))||(0==strcmp(request_method,"PUT")))
	{
		cm=post_func();
	}

	printf("\n");

	if(NULL==cm)
	{
		printf("ERROR!");
		return -1;
	}

	if((ret=data_exchange(cm))<0)
	{
		printf("ERROR!");
	}
	free(cm);
	cm=NULL;

	return ret;
}