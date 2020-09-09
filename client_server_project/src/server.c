#include "common.c"
#include <sys/stat.h>

void sigint_handler(int sig);
void handle_connection(int conn_fd, int listen_fd);
void send_file_info(int conn_fd, const char *dir, int is_simple);
void get_file(int conn_fd, char *ch, int is_force);
void create_dir(const char *path);
void put_file(int conn_fd, char *ch);
void run_command(int conn_fd, char *ch);

int main(int argc, char *argv[]){
    // setuid(0);
    // printf("uid %d\n", getuid());
    // printf("euid %d\n", geteuid());

    if (argc != 2) {
        printf("wrong usage: ./server port\n");
        exit(-1);
    }
    int listen_sock=socket(AF_INET,SOCK_STREAM,0);
    if (listen_sock==-1) {
        printf("%s\n",strerror(errno));
        exit(-1);
    }
    int reuseaddr=1;
    if (setsockopt(listen_sock,SOL_SOCKET,SO_REUSEADDR,&reuseaddr,sizeof(reuseaddr))==-1) {
        printf("setsockopt: %s\n",strerror(errno));
        exit(-1);
    }

    // server address
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    // sa.sin_addr.s_addr = htonl(INADDR_ANY);
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = htons(atoi(argv[1]));
    if (bind(listen_sock,(struct sockaddr*)&sa,sizeof(sa))==-1) {
        printf("Bind: %s\n",strerror(errno));
        exit(-1);
    }
    if (listen(listen_sock,SOMAXCONN)==-1) {
        printf("Listen: %s\n",strerror(errno));
        exit(-1);
    }
    // client address
    struct sockaddr_in ca;
    socklen_t ca_len;
    char ipaddr[64];
    // server quit command handler
    if (signal(SIGINT, sigint_handler) == SIG_ERR){
        perror("signale error");
        exit(-1);
    }
    printf("Server online\n");
    // listen to the port and react to connection
    for (;;) {
        int session_fd=accept(listen_sock, (struct sockaddr *)&ca, &ca_len);
        if (session_fd==-1) {
            if (errno==EINTR) continue;
            printf("failed to accept connection (errno=%d)\n",errno);
            exit(-1);
        }

        pid_t pid=fork();
        if (pid == -1) {
            printf("failed to create child process (errno=%d)\n", errno);
            exit(-1);
        }
        else if (pid == 0) {
            // get client host and port
            const char *client_ip = inet_ntop(AF_INET, &(ca.sin_addr), ipaddr, 64);
            int client_port = ntohs(ca.sin_port);
			printf ("** New connection from: ip=%s port=%d \n", client_ip, client_port);
            handle_connection(session_fd, listen_sock);
            close(session_fd);
			printf ("** Lost connection from: ip=%s port=%d \n", client_ip, client_port);
            exit(0);
        }
        else {
            close(session_fd);
        }
    }

    return 0;
}

/*
    sigint_handler - handling child processes while quitting the server
*/
void sigint_handler(int sig){
    if (getpid() == 0) {
        exit(0);
    }
    zombie_removal();
    printf("\nServer offline\n");
    exit(0);
}


void handle_connection(int conn_fd, int listen_fd){
    char buff[COMMAND_LENGTH + 1];
    while(1) {
        bzero(&buff, COMMAND_LENGTH+1);
        int k = read(conn_fd, buff, COMMAND_LENGTH);
        if (k <= 0) {
            perror("[Error] read from socket");
            exit(-1);
        }
        char *ch = strtok(buff, " ");
        // printf("command:%s\n", buff);
        // printf("command:%s\n", ch);
        if (strcmp(ch, SIMPLE_LIST) == 0){
            printf("%s\n", SIMPLE_LIST);
            // to-do
            ch = strtok(NULL, " ,");
            send_file_info(conn_fd, ch, 1);
        }
        else if (strcmp(ch, LONG_LIST) == 0) {
            printf("%s\n", LONG_LIST);
            // to-do
            ch = strtok(NULL, " ,");
            send_file_info(conn_fd, ch, 0);
        }
        else if (strcmp(ch, COMMAND_QUIT) == 0) {
            printf("%s\n", COMMAND_QUIT);
            break;
        }
        else if (strcmp(ch, COMMAND_SYS) == 0) {
            printf("%s\n", COMMAND_SYS);
            write(conn_fd, "linux x86_64 GNU/linux i78650U\n", LINE_LENGTH);
        }
        else if (strcmp(ch, COMMAND_PUT) == 0) {
            printf("%s\n", COMMAND_PUT);
            ch = strtok(NULL, " ,");
            get_file(conn_fd, ch, 0);
        }
        else if (strcmp(ch, COMMAND_FPUT) == 0) {
            printf("%s\n", COMMAND_FPUT);
            ch = strtok(NULL, " ,");
            get_file(conn_fd, ch, 1);
        }
        else if (strcmp(ch, COMMAND_GET) == 0) {
            printf("%s\n", COMMAND_GET);
            ch = strtok(NULL, " ,");
            put_file(conn_fd, ch);
        }
        else if (strcmp(ch, COMMAND_RUN) == 0) {
            printf("%s\n", COMMAND_RUN);
            ch = strtok(NULL, " ,");
            run_command(conn_fd, ch);
        }
    }
}

void send_file_info(int conn_fd, const char *dir_path, int is_simple) {
    char buffer[LINE_LENGTH + 1];
    char command[COMMAND_LENGTH];

    if (dir_path != NULL) {
        if (is_simple) {
            strcpy(command, "ls ");
        }
        else {
            strcpy(command, "ls -l ");
        }
        strcat(command, dir_path);
        FILE *fp=popen(command, "r");
        while(NULL != fgets(buffer, LINE_LENGTH, fp)){
            // printf("%s", buffer);
            write(conn_fd, buffer, LINE_LENGTH);
        }
    }
    else {
        if (is_simple){
            DIR *dir = opendir("./");
            struct dirent *ent;
            if (dir != NULL) {
                // accessing files in the directory using method readdir
                while ((ent = readdir (dir)) != NULL) {
                    if (ent->d_type == DT_DIR){
                        char buf[LINE_LENGTH];
                        sprintf(buf, "%s\n", ent->d_name);
                        write(conn_fd, buf, LINE_LENGTH);
                    }
                }
                // read over, close the directory
                closedir (dir);
            }
        }
        else {
            FILE *fp=popen("ls -l", "r");
            while(NULL != fgets(buffer, LINE_LENGTH, fp)){
                if (buffer[0] == 'd'){
                    // printf("%s", buffer);
                    write(conn_fd, buffer, LINE_LENGTH);
                }
            }
            pclose(fp);
        }
    }
    // 将结束符写入socket
    write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*5);

}


void create_dir(const char *path){
    rmdir(path);
    mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}



void get_file(int conn_fd, char *ch, int is_force) {
    char *dir = ch;
    ch = strtok(NULL, " ,");
    char *files = ch;

    if (!dir || !files) {
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "PUT error: dir name or files not provided\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
    }


    DIR *dir_ptr = opendir(dir);
    printf("PUT dir: %s\n", dir);
    printf("PUT files: %s\n", files);
    printf("PUT is_force: %d\n", is_force);
    printf("PUT dir: %p\n", dir_ptr);
    // file exists, not force
    if (dir_ptr && is_force != 1) {
        printf("PUT Error\n");
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "PUT error: dir exists and -f is not set\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
        return;
    }
    closedir(dir_ptr);

    printf("READY TO GET FILES\n");
    write(conn_fd, START_SENDING_INFO, LINE_LENGTH);
    create_dir(dir);
    chdir(dir);
    printf("Current Dir name:%s\n", getcwd(NULL, 0));
    char msg[MESSAGE_LENGTH + 1];
    while (files != NULL) {
        bzero(msg, MESSAGE_LENGTH);
        read(conn_fd, msg, MESSAGE_LENGTH);
        if (strcmp(msg, FILE_NOT_EXIST) == 0) {
            printf("File:%s not exists\n", files);
        }
        else {
            int file_len = atoi(msg);
            char *buffer = (char *)malloc(file_len + 1);
            read(conn_fd, buffer, file_len);
            printf("%s\n", buffer);

            printf("Write to file_path: %s\n", files);
            FILE *fp = fopen(files, "wb");
            if (fp != NULL) {
                printf("%p\n", fp);
                fwrite(buffer, 1, file_len, fp);
                printf("Write to file_path: %s %db\n", files, file_len);
            }
            free(buffer);
            fclose(fp);
        }
        files = strtok(NULL, " ,");
    }
    chdir("..");
}


void put_file(int conn_fd, char *ch) {
    char *dir = ch;
    ch = strtok(NULL, " ,");
    char *files = ch;

    // printf("Current Dir name:%s\n", getcwd(NULL, 0));
    // printf("%s %s %s\n", COMMAND_GET, dir, files);

    if (!dir || !files) {
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "GET error: dir name or files not provided\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
        return;
    }

    // check if the directory exists
    DIR *dir_ptr = opendir(dir);
    if (!dir_ptr) {
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "GET error: dir doesn't exist\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
        closedir(dir_ptr);
        return;
    }
    closedir(dir_ptr);


    // change current working directory to dir
    chdir(dir);

    FILE *f = fopen(files, "rb");
    // file not exist
    if (!f) {
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "GET error: file doesn't exist\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
        chdir("..");
        return;
    }


    printf("start sending:%s\n", ch);
    write(conn_fd, START_SENDING_INFO, LINE_LENGTH);
    fseek(f, 0, SEEK_END);

    // get file length and tell the server
    unsigned long file_len = ftell(f);
    char msg[MESSAGE_LENGTH+1];
    // *((int *)msg) = htonl(file_len);
    snprintf(msg, MESSAGE_LENGTH, "%d", (file_len));
    // tell client the file length
    write(conn_fd, msg, MESSAGE_LENGTH);

    // start to transfer
    rewind(f);
    char *buffer = (char *)malloc(file_len + 1);
    fread(buffer, file_len, 1, f);
    fclose(f);

    // write file content to socket
    write(conn_fd, buffer, file_len);
    printf("Sent %sb %s\n", msg, ch);
    printf("FILE CONTENT:\n%s\n", buffer);
    free(buffer);
    chdir("../");

}


void run_command(int conn_fd, char *ch) {
    char *dir = ch;
    if (!dir) {
        write(conn_fd, ERROR_SENDING, LINE_LENGTH);
        write(conn_fd, "RUN error: file name not provided\n", LINE_LENGTH);
        write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
        return;
    }
    char buffer[LINE_LENGTH + 1];
    char args[COMMAND_LENGTH + 1] = "";
    char *to_file = NULL;

    while ((ch = strtok(NULL, " ,")) != NULL) {
        if (strcmp(ch, "-f") == 0) {
            ch = strtok(NULL, " ,");
            to_file = ch;
            break;
        }
        else {
            strcat(args, ch);
            strcat(args, " ");
        }
    }


    char command[LINE_LENGTH] = "";

    FILE *save_to_file;
    if (to_file) {
        save_to_file = fopen(to_file, "w");
    }

    printf("name:%s args:%s to_file:%s\n", dir, args, to_file);

    // executable
    if(access(dir, X_OK) != -1)
    {
        printf("executable\n");
        sprintf(command, "%s %s ", dir, args);
        // return execute information
        FILE *fp = popen(command, "r");
        write(conn_fd, START_SENDING_INFO, LINE_LENGTH);
        while(NULL != fgets(buffer, LINE_LENGTH, fp)){
            printf("%s", buffer);
            write(conn_fd, buffer, LINE_LENGTH);
            if (to_file) {
                // fprintf(save_to_file, "%s", buffer);
                // fwrite(buffer, 1, LINE_LENGTH, save_to_file);
                fputs(buffer, save_to_file);
            }
            bzero(buffer, LINE_LENGTH);
        }
        write(conn_fd, FILE_SENDING_ENDED, LINE_LENGTH);
        pclose(fp);
    }
    // not executable
    else
    {
        printf("unexecutable\n");
        char *pfile = strtok(dir, ".");
	pfile = strtok(NULL, ".");
	printf("format: %s\n", pfile);
        if (pfile != NULL)
        {
            char sys_com[COMMAND_LENGTH] = "";
            // generate complie command
            if (strcmp(pfile, "c") == 0)
            {
                sprintf(sys_com, "gcc %s.c -o tmp -w", dir);
            }
            else if (strcmp(pfile, "cpp") == 0)
            {
                sprintf(sys_com, "g++ %s.cpp -o tmp -w", dir);
            }
            else {
                write(conn_fd, ERROR_SENDING, LINE_LENGTH);
                write(conn_fd, "RUN error: file name not compilable\n", LINE_LENGTH);
                write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
                if (to_file) {
                    fclose(save_to_file);
                }
                return;
            }
            // compile
            printf("COMMAND: %s\n", sys_com);
                            
            write(conn_fd, START_SENDING_INFO, LINE_LENGTH);

	    FILE *cp = popen(sys_com, "r");
            write(conn_fd, "COMPILING FILE\n", LINE_LENGTH);
            while(NULL != fgets(buffer, LINE_LENGTH, cp)){
                printf("%s", buffer);
                write(conn_fd, buffer, LINE_LENGTH);
                if (to_file) {
                    fputs(buffer, save_to_file);
                }
                bzero(buffer, LINE_LENGTH);
            }

            sprintf(command, "./tmp %s", args);
            printf("COMMAND: %s\n", command);
            // return execute information
            FILE *fp = popen(command, "r");
            while(NULL != fgets(buffer, LINE_LENGTH, fp)){
                printf("%s", buffer);
                write(conn_fd, buffer, LINE_LENGTH);
                if (to_file) {
                    // fprintf(save_to_file, "%s", buffer);
                    // fwrite(buffer, 1, LINE_LENGTH, save_to_file);
                    fputs(buffer, save_to_file);
                }
                bzero(buffer, LINE_LENGTH);
            }
            write(conn_fd, FILE_SENDING_ENDED, LINE_LENGTH);
            system("rm tmp");
            pclose(fp);
        }
        else {
            write(conn_fd, ERROR_SENDING, LINE_LENGTH);
            write(conn_fd, "RUN error: file name not provided\n", LINE_LENGTH);
            write(conn_fd, FILE_SENDING_ENDED, sizeof(char)*1);
            if (to_file) {
                fclose(save_to_file);
            }
            return;
        }
    }

    if (to_file) {
        fclose(save_to_file);
    }
}
