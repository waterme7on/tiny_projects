#include "timer.c"
#include "common.c"

// connect to host
int connect_host(char* ip, int port);
void list_command(int control_socket, char *ch);
void sys_command(int control_socket);
void put_command(int control_socket, char *ch, char *input_buffer);
void get_command(int control_socket, char *ch);
void run_command(int control_socket, char *ch);
void command_tip() { printf("Commands available: \n* list [-l] dir\n* get dir file\n* put dir file1 file2 file3 ... \n* run filename [argvs] [-f filename]  \n* sys\n* quit\n");}

int main(int argc, char *argv[]) {
    if (argc != 3){
        printf("Wrong usage: ./client ip port\n");
        exit(-1);
    }

    char *host_ip = argv[1];
    int host_port = atoi(argv[2]);
    int control_socket = connect_host(host_ip, host_port);
    if (control_socket == -1) {
        printf("failed to connect to host %s:%d", host_ip, host_port);
        exit(-1);
    }
	printf("Connected.\n");
    command_tip();

    timer t;
    char input_buffer[LINE_LENGTH+1];
    char buf[LINE_LENGTH+1];

    printf(">");
    while (fgets(input_buffer, LINE_LENGTH, stdin) != NULL) {
        input_buffer[strcspn(input_buffer, "\n")] = '\0';
        if (input_buffer[0] == '\0'){
            printf(">");
            continue;
        }

	pid_t pid = fork();

	if (pid != 0) {
            continue;
	}

        char *ch = strtok(input_buffer, " ");
        // quit command
        if (strcmp(ch, "quit") == 0) {
            printf("quitting ... \n");
            write(control_socket, COMMAND_QUIT, COMMAND_LENGTH);
            break;
        }
        else if (strcmp(ch, "list") == 0) {
            list_command(control_socket, ch);
        }
        else if (strcmp(ch, "put") == 0) {
            put_command(control_socket, ch, input_buffer);
        }
        else if (strcmp(ch, "get") == 0) {
            get_command(control_socket, ch);
        }
        else if (strcmp(ch, "sys") == 0) {
            sys_command(control_socket);
        }
        else if (strcmp(ch, "run") == 0) {
            run_command(control_socket, ch);
        }
        else {
            printf("[Error] wrong command.\n");
            command_tip();
        }
        fflush(stdin);
        bzero(input_buffer, LINE_LENGTH);
        printf(">");
    }
    close(control_socket);
    // remove zombie process
    zombie_removal();
    return 0;
}

int connect_host(char* ip, int port){
	int sock;
	struct sockaddr_in sa;
	int ret;
	sock = socket (AF_INET, SOCK_STREAM, 0);
	bzero (&sa, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	inet_pton (AF_INET, ip, &sa.sin_addr);
	ret = connect (sock,(const struct sockaddr *) &sa,sizeof (sa));
	if (ret != 0) {
		printf ("Connect Failed :(\n");
		exit (0);
	}
	return sock;
}


void list_command(int control_socket, char  *ch){
    timer t;
    start(t);
    char list_buf[COMMAND_LENGTH+1] = "";
    char buf[LINE_LENGTH+1];

    bzero(list_buf, COMMAND_LENGTH);

    // long list
    ch = strtok(NULL, " ");
    if (ch && strcmp(ch, "-l") == 0){
        strcpy(list_buf, LONG_LIST);
    }
    // simple list
    else if (ch){
        strcpy(list_buf, SIMPLE_LIST);
        strcat(list_buf, " ");
        strcat(list_buf, ch);
    }
    else {
        strcpy(list_buf, SIMPLE_LIST);
    }
    if ((ch = strtok(NULL, " ,")) != NULL){
        strcat(list_buf, " ");
        strcat(list_buf, ch);
    }
    write(control_socket, list_buf, COMMAND_LENGTH);

    // printf("--------\n");
    // printf("[COMMAND: %s]\n", list_buf);
    // printf("--------\n");

    while (1) {
        bzero(buf, LINE_LENGTH);
        int k = read(control_socket, buf, LINE_LENGTH);
        if (k == 0 || strcmp(buf, FILE_SENDING_ENDED) == 0) {
            break;
        }
        printf("%s",buf);

    }
    stop(t);
    print_time(t);
    printf("\n");
}


void sys_command(int control_socket) {
    char buf[LINE_LENGTH+1];
    timer t;
    start(t);
    write(control_socket, COMMAND_SYS, COMMAND_LENGTH);
    bzero(buf, LINE_LENGTH);
    read(control_socket, buf, LINE_LENGTH);
    printf("system info: %s\n", buf);
    stop(t);
    print_time(t);
}



void put_command(int control_socket, char *ch, char *input_buffer){
    timer t;
    start(t);
    // firstly get dir name
    ch = strtok(NULL, " ,");
    // no dir name: wrong usage
    if (ch == NULL) {
        command_tip();
        return;
    }
    char *dir = ch;

    // secondly get files
    char files[COMMAND_LENGTH] = "";

    ch = strtok(NULL, " ,");
    // no file name: wrong usage
    if (ch == NULL) {
        command_tip();
        return;
    }

    strcat(files, ch);
    strcat(files, " ");


    int isForce = 0;
    while ( (ch = strtok(NULL, " ,")) != NULL){
        // printf("put_command file: %s\n", ch);
        if (strcmp(ch, "-f") == 0) {
            isForce = 1;
            continue;
        }
        // files after -f is considered as error
        if (isForce) {
            command_tip();
            break;
        }
        strcat(files, ch);
        strcat(files, " ");
    }

    char command[COMMAND_LENGTH + 1];
    if (isForce) {
        sprintf(command, "%s %s %s", COMMAND_FPUT, dir, files);
    }
    else {
        sprintf(command, "%s %s %s", COMMAND_PUT, dir, files);
    }
    write(control_socket, command, COMMAND_LENGTH);

    // communicate with server
    char buf[LINE_LENGTH + 1];
    bzero(buf, LINE_LENGTH);
    read(control_socket, buf, LINE_LENGTH);
    // put file error
    if (strcmp(buf, ERROR_SENDING) == 0) {
        while (1) {
            bzero(buf, LINE_LENGTH);
            int k = read(control_socket, buf, LINE_LENGTH);
            if (k == 0 || strcmp(buf, FILE_SENDING_ENDED) == 0) {
                break;
            }
            printf("%s",buf);
        }
    }

    if (strcmp(buf, START_SENDING_INFO) == 0) {
        printf("start putting files\n");
        printf("Files: %s\n", files);
        ch = strtok(files, " ,");
        while (ch != NULL) {
            FILE *f = fopen(ch, "rb");
            // file not exist
            if (!f) {
                printf("[Error] File: %s not exist\n", ch);
                write(control_socket, FILE_NOT_EXIST, MESSAGE_LENGTH);
                ch = strtok(NULL, " ,");
                continue;
            }
            printf("start sending:%s\n", ch);
            fseek(f, 0, SEEK_END);

            // get file length and tell the server
            unsigned long file_len = ftell(f);
            char msg[MESSAGE_LENGTH+1];
        	// *((int *)msg) = htonl(file_len);
            snprintf(msg, MESSAGE_LENGTH, "%d", (file_len));
            // tell server the file length
            write(control_socket, msg, MESSAGE_LENGTH);

            // start to transfer
            rewind(f);
            char *buffer = (char *)malloc(file_len + 1);
            fread(buffer, file_len, 1, f);
            fclose(f);

            // write file content to socket
            write(control_socket, buffer, file_len);
            printf("Sent %sb %s\n", msg, ch);
            printf("FILE CONTENT:\n%s\n", buffer);
            free(buffer);

            ch = strtok(NULL, " ,");
        }
    }
    stop(t);
    print_time(t);
}


void get_command(int control_socket, char *ch) {
    timer t;
    start(t);
    // firstly get dir name
    ch = strtok(NULL, " ,");
    // no dir name: wrong usage
    if (ch == NULL) {
        command_tip();
        return;
    }
    char *dir = ch;

    // secondly get filename
    ch = strtok(NULL, " ,");
    // no file name: wrong usage
    if (ch == NULL) {
        command_tip();
        return;
    }
    char *file = ch;

    // write command to socket
    char command[COMMAND_LENGTH];
    sprintf(command, "%s %s %s", COMMAND_GET, dir, file);
    // printf("%s\n", command);
    write(control_socket, command, COMMAND_LENGTH);

    // communicate with server
    char buf[LINE_LENGTH + 1];
    bzero(buf, LINE_LENGTH);
    read(control_socket, buf, LINE_LENGTH);

    // get file error
    if (strcmp(buf, ERROR_SENDING) == 0) {
        while (1) {
            bzero(buf, LINE_LENGTH);
            int k = read(control_socket, buf, LINE_LENGTH);
            if (k == 0 || strcmp(buf, FILE_SENDING_ENDED) == 0) {
                break;
            }
            printf("%s",buf);
        }
    }

    if (strcmp(buf, START_SENDING_INFO) == 0) {
        // read file length
        char msg[MESSAGE_LENGTH + 1];
        read(control_socket, msg, MESSAGE_LENGTH);
        int file_len = atoi(msg);
        // read file
        char *buffer = (char *)malloc(file_len + 1);
        read(control_socket, buffer, file_len);

        char *p_to_screen = strtok(buffer, "\n");

        int p_line_cnt = 0;
        while (p_to_screen) {
            printf("%s\n", p_to_screen);
            p_to_screen = strtok(NULL, "\n");
            p_line_cnt += 1;
            if (p_line_cnt == 3) {
                printf("continue? y/n:");
                char c = getchar();
                if (c == 'N' || c == 'n') {
                    break;
                }
                p_line_cnt = 0;
            }
        }
    }
    stop(t);
    print_time(t);
}

void run_command(int control_socket, char *ch) {
    timer t;
    start(t);
    printf("%s\n", ch);

    char command[COMMAND_LENGTH + 1] = "";
    ch = strtok(NULL, " ,");
    char *file = ch;
    if (!file) {
        command_tip();
        return;
    }
    strcat(command, COMMAND_RUN);
    strcat(command, " ");
    strcat(command, file);
    strcat(command, " ");

    int is_to_file = 0;
    char *to_file;
    while ((ch = strtok(NULL, " ,")) != NULL) {
        if (strcmp(ch, "-f") == 0) {
            ch = strtok(NULL, " ,");
            to_file = ch;
            is_to_file = 1;
            if (!ch) {
                command_tip();
                return;
            }
        }
        else {
            if (is_to_file) {
                command_tip();
                return;
            }
            strcat(command, ch);
            strcat(command, " ");
        }
    }
    if (is_to_file){
        strcat(command, "-f ");
        strcat(command, to_file);
    }

    printf("command: %s\n", command);

    write(control_socket, command, COMMAND_LENGTH);
    // communicate with server
    char buf[LINE_LENGTH + 1];
    bzero(buf, LINE_LENGTH);
    read(control_socket, buf, LINE_LENGTH);

    // get file error
    // start receiving result
    printf("----------------\nResult: \n");
    while (1) {
        bzero(buf, LINE_LENGTH);
        int k = read(control_socket, buf, LINE_LENGTH);
        if (k == 0 || strcmp(buf, FILE_SENDING_ENDED) == 0) {
            break;
        }
        printf("%s",buf);
    }
    stop(t);
    printf("----------------\n");
    print_time(t);
}
