#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>

#define UNIX_PATH "./cgi_link"	//���׽������ӷ�·��(�����˵�ͬ����·��һ��,����������CGIĿ¼��)

enum DEV_TYPE
{
	IGNORE=0x0,
	ANDROID,
	IOS,
	WP,
};

typedef struct cgi_msg
{
	uint32_t packet_len;
	uint32_t type;
	char sender[48];
	char recver[48];
	uint32_t dev_type;
	uint32_t len;
	char context[0];
}CGI_MSG,*CM;
