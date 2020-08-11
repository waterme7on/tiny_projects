#define _GNU_SOURCE
#include <sys/types.h>
#include <signal.h>
#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>

// 用于控制downloadCount
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
// 用于控制读取
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;

int downloadCount = 0;

/*
	connect_host - 创建socket连接指定的address和port
	@address: ipv4 地址
	@port: 服务器所在端口号
	和server.c中的相同
*/
int connect_host(char* address, int port){
	int sock;
	struct sockaddr_in sa;
	int ret;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	bzero (&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	inet_pton (AF_INET, address, &sa.sin_addr);
	ret = connect (sock,(const struct sockaddr *) &sa,sizeof (sa));
	if (ret != 0) {
		printf ("Connect Failed :(\n");
		exit (0);
	}
	return sock;
}

// 用于接收服务器发送的数据的端口和socket
int dataPort = -1;
int data_list_sock = -1;


/*
	openDataChannel - 开启数据接收端口，获取socket
*/
void openDataChannel(){
	int list_sock;
	struct sockaddr_in sa;
	list_sock = socket (AF_INET, SOCK_STREAM, 0);
	bzero (&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = 0;
	bind (list_sock,(struct sockaddr *) &sa,sizeof(sa));
	listen (list_sock, 25);
	socklen_t len = sizeof(sa);
	if (getsockname(list_sock, (struct sockaddr *)&sa, &len) != -1)
    	dataPort = ntohs(sa.sin_port);
    data_list_sock = list_sock;
}

struct param {
	int sock;
};


/*
	pause_handler - 用于处理暂停
*/
void pause_handler(int sig){
	pthread_mutex_lock( &mutex2 );
	// printf("\t[INFO] TID: %d, PID: %d\n", gettid(),getpid());
	printf("\t[INFO] Paused. continue: y/n\n>");
	// kill(0, SIGSTOP);
	fflush(stdin);
	char buff[MAXLINELEN];
	while (1){
		fgets(buff, MAXLINELEN, stdin);
		fflush(stdin);
		if (strchr(buff, 'y') != NULL) {
			// 是则返回原来的函数继续
			printf("\t[INFO] Continued.\n>");
			// kill(0, SIGCONT);
			break;
		}
		else if (strchr(buff, 'n') != NULL){
			// 否则回收各个子进程，退出
			printf("\t[INFO] Download break\n");
			kill(0, SIGKILL);
			while(wait(NULL) > 0){};
			exit(0);
		}
		else {
			printf("\t[Download] Wrong input: %s", buff);
		}
	}
	fflush(stdin);
	pthread_mutex_unlock( &mutex2 );
	return;
}

/*
	download - 在对应端口下读取文件
*/
void *download(void* args){
	// 初始化变量
	struct param *p;
	p = (struct param *) args;
	char msg[4];
	// 首先接收server发送的信息
	read(p->sock, msg, 10);
	int size = ntohl(*((int *)msg));
	if ( size == FILE_NOT_FOUND){
		// server发送FILE_NOT_FOUND说明文件未找到
		printf("\t[NOT OK] No such file.\n");
	} else {
		// 否则说明文件找到，可以开始接收数据
		// 记录时间
		clock_t st_time = clock() / (CLOCKS_PER_SEC / 1000);
		char filename[MAX_FILE_LENGTH];
		// 否则则读取文件名
		read(p->sock, filename, MAX_FILE_LENGTH);
		printf("\t[Downloading] %20s %.2lf kbytes.\n", filename, size / 1024.0);
		// printf("\t[Downloading] TID: %d, PID: %d\n", gettid(),getpid());

		// 开启缓冲区用于读取文件
		char* buff = malloc(size);
		bzero(buff, size);
		int downSize = size;
		// 如果还有未读取数据则进行读取
		while ( downSize > 0){
			// printf("%d : %p -->", downSize, buff);
			// 从socket中读取数据
			pthread_mutex_lock( &mutex2 );
			int resp = read(p->sock, buff, 1000);
			printf("\t[Downloading]  read piece: %d, Ctrl+c to pause\n", resp);
			buff = buff + resp;
			downSize = downSize - resp;
			pthread_mutex_unlock( &mutex2 );
		}
		buff = buff - size - 1;

		// 将读取到的数据写入对应的文件名中
		FILE *fp = fopen(filename, "wb");
		fwrite(buff,1,size,fp);
		fclose(fp);
		// 输出计时信息
		clock_t en_time = clock() / (CLOCKS_PER_SEC / 1000);
		printf("\t[Downloaded] %20s, took %d ms.\n", filename, (int)(en_time-st_time));
	}
	// 将downCount减一
	// 这里用到了线程共享锁
	pthread_mutex_lock( &mutex1 );
	downloadCount--;
	pthread_mutex_unlock( &mutex1 );
	close(p->sock);
	pthread_exit(NULL);
}

/*
	listenDataChannel - 监听openDataChannel开启的port和socket
*/
void *listenDataChannel(void *args){
	signal(SIGINT, pause_handler);
	int conn_sock;
	struct sockaddr_in ca;
	socklen_t ca_len;
	while (1){
		bzero (&ca, sizeof(ca));
		ca_len = sizeof(ca); // important to initialize
		// 监听打开的数据接收socket
		conn_sock = accept (data_list_sock, (struct sockaddr *) &ca,&ca_len);
		struct param* args = malloc(sizeof(struct param));
		args->sock = conn_sock;
		// 创建线程
		pthread_t tid;
		// 进入新线程进行download
		int ret = pthread_create(&(tid), NULL, download, (void *)args);
		if ( ret < -1){
			printf("FATAL ERROR! Thread create failed.");
			exit(-1);
		}
	}
	pthread_exit(NULL);
}


/*
	hostname_to_ip - 域名转换为地址
*/
int hostname_to_ip(char * hostname , char* ip)
{
	struct hostent *he;
	struct in_addr **addr_list;
	int i;
	if ( (he = gethostbyname( hostname ) ) == NULL)
	{
		// get the host info
		herror("gethostbyname");
		return 1;
	}

	addr_list = (struct in_addr **) he->h_addr_list;

	for(i = 0; addr_list[i] != NULL; i++)
	{
		//Return the first one;
		strcpy(ip , inet_ntoa(*addr_list[i]) );
		return 0;
	}
	return 1;
}

int main(int argc, char *argv[]){
	if ( argc != 3){
		printf("Wrong usage!\n");
		exit(-1);
	}

	// 将hostname转换为port
	char *hostname = argv[1];
	char ip[100];
	hostname_to_ip(hostname , ip);

	// 开启接收数据所需要的socket和port
	openDataChannel();
	pthread_t tid;

	// 开启监听线程
	int ret = pthread_create(&(tid), NULL, listenDataChannel, NULL);
	if ( ret < 0){
		printf("FATAL ERROR! Thread create failed.");
		exit(-1);
	}

	// 主进程无视SIGINT信号
	// download进程需要接收这个信号来进行pause
	// signal(SIGINT, SIG_IGN);

	int port = atoi(argv[2]);
	// 连接到server
	printf("Trying to connect to host\n");
	int controlSocket = connect_host(hostname, port);

	printf("Connected. Commands available: \n* list\n* get <file1> <file2> .. <fileN>\n* quit\n");
	// printf("Main TID: %d, PID: %d\n", gettid(),getpid());
	// client输入命令
	char initial[4];
	*((int *)initial) = htonl(dataPort);
	write(controlSocket, initial, 4);

	char buff[MAXLINELEN];
	printf(">");
	// 输入命令，响应命令对应逻辑
	while (fgets(buff, MAXLINELEN, stdin) != NULL) {
		buff[strcspn(buff, "\n")] = '\0';
		if (buff[0] == '\0')
			continue;

		char *ch = strtok(buff, " ");
		// 同样的，根据输入命令
		if ( strcmp(ch,"quit") == 0){
			// 将quit发送给server，然后退出
			write(controlSocket, MESSAGE_QUIT, COMMAND_LENGTH);
			printf("Bye!\n");
			return 0;
		}
		else if ( strcmp(ch,"list") == 0){
			// 将list发送给server，然后接收server发回的list
			write(controlSocket, MESSAGE_LIST, COMMAND_LENGTH);


			char buf[MAX_FILE_LENGTH];
			// 接收每个文件名，并以FILE_SENDING_ENDED结尾
			// 可以对应server.c里的sendFileName看
			while (1){
				bzero(buf, MAX_FILE_LENGTH);
				// 依次读取后输出
				int k = read (controlSocket, buf, MAX_FILE_LENGTH);
				if ( k == 0 || strcmp(buf,FILE_SENDING_ENDED) == 0){
					// 检测到FILE_SENDING_ENDED就退出
					break;
				}
				printf("%s ", buf);
			}
			printf("\n");
		}
		else if ( strcmp(ch,"get") == 0){
			// For each file, send server: GETT, A, GETT, B, GETT, C, GETT, D (i.e for a file request, GETT is send, after filename is sent)
			// 首先解析命令，将get file1 file2 ... 变为
			// get file1 file2 -> get file1 get file2
			while ( (ch = strtok(NULL, " ,")) != NULL) {// 按照","和" "来解析get 命令
				// 由于是多线程，需要给共享变量downloadcount枷锁
				pthread_mutex_lock( &mutex1 );
				downloadCount++;
				pthread_mutex_unlock( &mutex1 );
				// 将信息写入
				write(controlSocket, MESSAGE_GET, COMMAND_LENGTH);
				char tmp[MAX_FILE_LENGTH];
				strcpy(tmp, ch);
				write(controlSocket, tmp, MAX_FILE_LENGTH);
			}
			printf("Starting to download %d files\n" , downloadCount);
			// 堵塞等待下载结束
			while ( downloadCount > 0);
		}
		else if ( strcmp(ch, "put") == 0){
		}
		else {
			printf("No such command as: %s\n", ch);
		}
		// 清空缓冲区
		fflush(stdin);
		bzero(buff, MAXLINELEN);
		printf(">");
	}
	wait(NULL);
	// 关闭连接
	close(controlSocket);
	exit (0);
}
