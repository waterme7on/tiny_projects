#include "shared.h"
#include <sys/socket.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>

/*
	connect_host - 创建socket连接指定的address和port
	@address: ipv4 地址
	@port: 服务器所在端口号
*/
int connect_host(const char* address, int port){
	int sock;
	struct sockaddr_in sa;
	int ret;
	// 开启一个socket，socket_family=AF_INET，socket_type=SOCK_STREAM
	// 参数具体说明在这里：https://man7.org/linux/man-pages/man2/socket.2.html
	sock = socket (AF_INET, SOCK_STREAM, 0);
	// 释放sa的内存
	bzero (&sa, sizeof(sa));
	// 这里指定sa的端口号和协议族
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	// 将IPv4地址解析后存储到sa.sin_addr
	inet_pton (AF_INET, address, &sa.sin_addr);
	// 准备好后连接到对应的socket
	ret = connect (sock,(const struct sockaddr *) &sa,sizeof (sa));
	if (ret != 0) {
		printf ("Connect Failed :(\n");
		exit (0);
	}
	return sock;
}

/*
	sendFileNames - 向sock对应的socket发送当前server目录下的文件名
	@sock: socket 地址
*/
void sendFileNames(int sock){
	// dir为文件指针
	DIR *dir = opendir ("./");
	struct dirent *ent;
	if (dir != NULL) {
		// 通过readdir方法得到文件详细信息
		// 即ent，ent->d_name表示文件名
		while ((ent = readdir (dir)) != NULL) {
			if ( ent->d_type == DT_REG) {
				write(sock, ent->d_name, MAX_FILE_LENGTH);
			}
	  	}
		// 读取结束，关闭文件夹
	 	closedir (dir);
	}
	// 将结束符写入socket
	write(sock, FILE_SENDING_ENDED, sizeof(char)*1);
}


// 发送文件用结构体：host表示域名，非联网可以任意，port表示端口
struct send_params {
	const char* host;
	int port;
	char* filename;
};


/*
	sendFile - 发送文件用
	@args: 对应send_params结构体中的各项
*/
void* sendFile(void* args){
	struct send_params *p;
	p = (struct send_params *) args;
	// 此处host可以无需指定，因为没有涉及到网络传输，只是进程通信
	int sock = connect_host(p->host, p->port);
	printf("\t[T]Started Sending %s \n", p->filename);
	// 打开对应文件
	FILE *file = fopen(p->filename, "rb");
	if (!file){
		// 此处是针对文件不存在情况的处理
		char msg[10];
		*((int *)msg) = htonl(FILE_NOT_FOUND);
		write(sock, msg, 4);
		pthread_exit(NULL);
	}
	// 将文件指针重置
	fseek(file, 0, SEEK_END);
	// 得到文件长度
	unsigned long fileLen =ftell(file);
	// 传送文件长度
	char msg[10];
	*((int *)msg) = htonl(fileLen);
	write(sock, msg, 10);

	// 传送文件名
	write(sock, p->filename, MAX_FILE_LENGTH);
	// 重置文件指针
	rewind(file);
	// 请求大小等于文件长度的缓冲区
	char *buffer=(char *)malloc(fileLen+1);
	// 文件读入缓冲区中
	fread(buffer, fileLen, 1, file);
	// 关闭文件
	fclose(file);
	// 缓冲区写入socket中
	write(sock, buffer, fileLen + 1);
	// 关闭socket
	close(sock);
	printf("\t[T] Sent %db %s\n", (int)fileLen, p->filename);
	// 释放缓冲区
	free(buffer);
	// 线程终止
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){
	if ( argc != 2){
		printf("Wrong usage!\n");
		exit(-1);
	}

	// 初始化一些变量
	int list_sock;
	int conn_sock;
	struct sockaddr_in sa, ca;
	socklen_t ca_len;
	char ipaddrstr[IPSTRLEN];
	list_sock = socket (AF_INET, SOCK_STREAM, 0);
	int optval = 1;

	// 这一步使得list_sock可以在发生意外终止的情况下被重用
	setsockopt(list_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);

	// 初始化sockaddr
	bzero (&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	// 端口即为输入的第二个参数
	sa.sin_port = htons(atoi(argv[1]));

	// 将socket与sockaddr绑定
	bind (list_sock,(struct sockaddr *) &sa,sizeof(sa));
	// 开始监听这个端口
	listen (list_sock, 10);
	printf("Server is Online!\n");
	while (1)
	{
		bzero (&ca, sizeof(ca));
		ca_len = sizeof(ca); // important to initialize
		// 用accept来监听，一旦有连接
		// 则fork一个进程
		conn_sock = accept (list_sock, (struct sockaddr *) &ca,&ca_len);
		pid_t child = fork();
		if ( child == 0){
			// 子进程用于实现各个逻辑
			// 首先获取host和port
			const char* client_ip = inet_ntop(AF_INET, &(ca.sin_addr),ipaddrstr, IPSTRLEN);
			int client_port = ntohs(ca.sin_port);
			printf ("** New connection from: ip=%s port=%d \n", client_ip, client_port);
			char initial[4];
			//
			read (conn_sock, initial , 4);
			int downloadPort = ntohl(*((int *)initial));
			printf("** Download port: %d **\n", downloadPort);
			while (1){
				char buff[COMMAND_LENGTH + 1];
				bzero(&buff, COMMAND_LENGTH + 1);
				// 读取输入的命令
				int k = read (conn_sock, buff, COMMAND_LENGTH);
				if ( k <= 0){
					break;
				}
				// 按照输入命令处理逻辑
				if ( strcmp(buff,"LIST") == 0){
					// list则发送文件名
					printf("LIST OPERATION, SENDING FILE NAMES\n");
					sendFileNames(conn_sock);
					bzero(buff,COMMAND_LENGTH);
				}
				else if ( strcmp(buff,MESSAGE_QUIT) == 0){
					// quit则直接退出
					printf("Quiting..");
					break;
				}
				else if ( strcmp(buff,MESSAGE_GET) == 0){
					// get首先读取文件名
					char b[MAX_FILE_LENGTH];
					bzero(b, MAX_FILE_LENGTH);
					read (conn_sock, b, MAX_FILE_LENGTH);

					// 初始化发送向量
					struct send_params* args = malloc(sizeof(struct send_params));
					args->host = client_ip;
					args->port = downloadPort;

					char* mychar = malloc(sizeof(char)*MAX_FILE_LENGTH);
					strcpy(mychar, b);
					args->filename = mychar;

					// 创建线程处理sendFile
					pthread_t tid;
					int ret = pthread_create(&(tid), NULL, sendFile, (void *)args);
					if ( ret < 0){
						exit(-1);
					}
				}
				else if (strcmp(buff, "PUT") == 0) {
					// 读取文件长度
					char msg[10];
					read(conn_sock, msg, 10);
					int size =  ntohl(*((int *)msg));
					// 文件名
					char filename[MAX_FILE_LENGTH];
					bzero(filename, MAX_FILE_LENGTH);
					read (conn_sock, filename, MAX_FILE_LENGTH);

					printf("\t[GETTING] Get file %s size:%d\n", filename, size);


					char *buff = (char*)malloc(size);
					bzero(buff, size);

					int downSize = size;
					// 如果还有未读取数据则进行读取
					if (size < 65536) {
						read(conn_sock, buff, size+1);
						printf("\t[GETTING] read from socket\n");
					}
					else{
						while ( downSize > 0){
							// 从socket中读取数据
							int resp = read(conn_sock, buff, 65536);
							buff = buff + resp;
							downSize = downSize - resp;
						}
					}
					// buff = buff - size - 1;
					// 将读取到的数据写入对应的文件名中
					FILE *fp = fopen(filename, "wb");
					fwrite(buff,1,size,fp);
					fclose(fp);
					printf("\t[Got] File %s: %db\n", filename, size);
				}
			}
			// 关闭当前连接
			close (conn_sock);
			printf ("Server closed connection to client\n");
			// 退出子进程
			exit(1);
		}
	}
}
