# include <stdio.h>
# include <unistd.h> // for fork() exec() read() close() lseek() dup() ...
# include <stdlib.h>
#include <netinet/in.h>  // sockaddr_in, htons, INADDR_ANY
#include <arpa/inet.h>   // htons, inet_ntoa
# include <sys/socket.h> // for socket
# include <sys/sem.h> // for semaphore
# include <string.h>
# include <errno.h>

int main(int argc, char* argv[]){
    char send_buf[50] = {0};

    if (argc != 6){
        printf("Usage: ./lab6_client <ip> <port> <deposit/withdraw> <amount> <times>\n");
        exit(EXIT_FAILURE);
    }

    int server_fd = 0;
    server_fd =  socket(AF_INET , SOCK_STREAM , 0);
    if(server_fd == -1){
        perror("Connect to server failed");
    }

    int port = atoi(argv[2]);
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(argv[1]);
    address.sin_port = htons(port);

    if(connect(server_fd,(struct sockaddr *)&address, addrlen) < 0){
        printf("Connection error\n");
    }

    sprintf(send_buf, "%s %s %s", argv[3], argv[4], argv[5]);
    send(server_fd, send_buf, 50, 0);

    return 0;

}