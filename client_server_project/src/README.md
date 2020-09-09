
## 函数说明

server.c

1. sigint_handler(ing sig);
   @sig: SIGINT信号
   处理server停止的命令，在停止前回收各个僵尸进程

2. handle_connection(int conn_fd, int listen_fd);
   @conn_fd: 连接后创建的socket 
   @listen_fd: 服务器坚挺的socket
   当服务器监听到一个新的连接时，即创建子进程进入这个函数，以处理新建立的链接。
   函数识别接收到的命令，拆分和识别命令，并根据命令进入到相应的处理程序中。
   结果：新建了子进程响应连接


3. send_file_info(int conn_fd, const char *dir, int is_simple);
   @conn_fd: 同上
   @dir: 文件夹名
   @is_simple: 是否包含-f参数
   响应list命令
   从conn_fd得到各个文件的名字，在dir中进行寻找，如果dir不存在，则返回当前目录下存在的各个文件夹；如果dir存在，则返回对应文件夹的信息。
   结果：对文件进行读取

4. get_file
   @conn_fd: 同上
   @ch: 接收到的命令
   @is_force: 是否有-f参数
   响应客户的put命令
   根据接收到的命令进行拆分，得到文件夹以及各个文件；
   如果文件夹存在且没有-f参数则返回错误信息给客户端，否则则按照规则，将文件上传到指定的文件夹。
   结果：可能新建文件夹以及文件

5. put_file
   @conn_fd: 同上
   @ch: 同上
   响应客户的get命令
   根据ch拆分得到文件夹以及文件，如果文件夹或者文件不存在则报错，否则则将文件大小以及文件内容写入到socket
   结果：对文件进行读取

6. create_fir
   强制创建一个新的文件，如果文件名对应的文件已经存在则删除后重新创建

7. run_command
   拆分得到文件名、参数以及是否输出到文件
   如果文件为可执行文件，则按照参数执行；
   如果文件可编译（.c,.cpp）文件，则编译执行；
   否则返回结果；
   程序执行的结果最终均会返回到客户端
   结果：对文件进行编译，产生/运行了可执行文件

8. main
   监听socket，当有新连接时，建立子进程，子进程进入handle_connection逻辑，父进程继续监听是否有新连接




## client.c
1. connect_host
   @ ip
   @ port
   根据ip和port连接服务器
   结果：建立了新的socket连接

2. list_command
   @control_socket: 连接的socket
   将命令写入socket，并打印结果
   结果：得到文件信息

3. sys_command
   @control_socket
   将命令写入socket，并打印结果


4. put_command
   @control_socket
   @ch
   根据ch拆分出文件夹和文件，将文件上传至服务器，获得返回结果后打印

5. get_command
   @control_socket
   根据ch拆分出文件夹和文件，得到服务器返回的文件后显示，并每次只显示多行，需要用户确定是否继续显示。
   结果：获得服务器端传来的文件

6. run_command
   @control_socket
   @ch
   根据ch拆分文件和命令，传至服务器，接收服务器结果并打印

7. command_tip
   打印提示信息

8. main
   结果用户输入，并根据用户输入，如果输入为错误输入，则提示命令；否则新建进程执行对应的命令逻辑
















