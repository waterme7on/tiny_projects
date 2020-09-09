#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


#define COMMAND_LENGTH 50
#define SIMPLE_LIST "LIST"
#define LONG_LIST "LLIST"
#define COMMAND_QUIT "QUIT"
#define COMMAND_SYS "SYS"
#define COMMAND_PUT "PUT"
#define COMMAND_FPUT "FPUT"
#define COMMAND_GET "GET"
#define COMMAND_RUN "RUN"


#define ERROR_SENDING "*****"
#define START_SENDING_INFO "READY_TO_GET"

#define LINE_LENGTH 1024
#define FILE_NAME_LENGTH 50
#define FILE_SENDING_ENDED "."
#define FILE_NOT_EXIST "FNEXIST"
#define MESSAGE_LENGTH 10