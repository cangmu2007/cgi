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

#define UNIX_PATH "./cgi_link"	//域套接字连接符路径(与服务端的同名宏路径一致,尽量创建在CGI目录下)

typedef struct cgi_msg
{
	uint32_t packet_len;
	uint32_t type;
	char sender[32];
	char recver[32];
	uint32_t len;
	char context[0];
}CGI_MSG,*CM;
