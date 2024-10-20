/*
1. create socker server
2. accept multiple connection.
   When a new connection established, use fork() to create child 
   process.Meanwhile, parent process will wait for new connections.
   child process is aim to execute "sl" and tranfer to client.
3. Use dup2() to redirect output of "sl" from terminal to client.
4. Use execlp() to run "sl"
5. After child process finish running "sl", there might be a chance to generate zombie process.
   To avoid zombie process, we create a SIGCHID and a handler function which uses waitpid to clear
   all child process.
*/

#include <stdio.h>  // perror()
#include <stdlib.h> // exit()
#include <fcntl.h>  // open()
#include <unistd.h> // dup2() exec()
#include <signal.h>   // signal()
#include <sys/wait.h> // waitpid()
#include <sys/socket.h> // socket()
#include <arpa/inet.h>  // sockaddr

/* Use for killing Zombie process */
void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
int server_fd;
/* Use for closing socket when catching SIGINT(Ctrl+C) signal*/
void sigint_handler(int signum) {
    close(server_fd);
    exit(0);
}



int main(int argc, char* argv[]){

    if(argc != 2) {
        fprintf(stderr, "Usage: ./lab5 <port>\n");
        exit(EXIT_FAILURE);
    }

    /* Whenever child process terminates, a SIGCHILD signal received form child, 
    it will run zombie_handler to wait for every child process and clean them */
    signal(SIGCHLD, zombie_handler); 
    signal(SIGINT, sigint_handler);
    
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0){
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    /* Force using socket address already in use */
    int yes = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    /* struct sockaddr_in info */
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address); 
    int port = atoi(argv[1]);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    /* start binding */
    if (listen(server_fd, 5) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("Server is listening to %d\n", port);

    while(1){
        /* accepting new connection */
        int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if(client_fd < 0){
            perror("Error accepting connection");
        }

        /* create child process to handle connection*/
        pid_t pid = fork();
        if (pid == 0){
            /* redirect output to client socket */

            dup2(client_fd, STDOUT_FILENO);
            close(client_fd);
            int child_pid = getpid();
            printf("Child ID: %d\n", child_pid);
            fflush(stdout);
            execlp("sl",  "sl", "-l", NULL);
            perror("Error execlp");
            exit(EXIT_FAILURE); 
        }else if(pid > 0){
            /* for parent process */
            printf("Child process created with PID: %d\n", pid);
            close(client_fd);
        }else{
            perror("fork failed");
        }
    }

    return 0;
}