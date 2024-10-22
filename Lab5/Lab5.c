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

int server_fd;

/* Use for killing Zombie process */
void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/* Close socket when catching SIGINT signal ( Signal Interrupt such as Ctrl + C ) */
void server_handler(int signum) { 
    close(server_fd);
}

int main(int argc, char* argv[]){

    /* Whenever child process terminates, a SIGCHILD signal received form child, 
    it will run zombie_handler to wait for every child process and clean them */
    signal(SIGCHLD, zombie_handler);
    signal(SIGINT, server_handler);

    if (argc != 2){
        printf("Usage: ./lab5 <port>\n");
        exit(EXIT_FAILURE);
        /*
            exit(return value)
            if return value is 0 means success
            nonzero means error
            in general, EXIT_FAILURE equals to 1
        */
    }


    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* setting sockaddr_in  */
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int port = atoi(argv[1]);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    /* force using socket address already in use */
    int yes = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    // start binding
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Server binding failed");
        exit(EXIT_FAILURE);
    }

    // start listening
    if (listen(server_fd, 3) < 0) {
        perror("Server listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is now listening...\n");

    while(1){
        /* accepting new connection */
        int client_fd =accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0){
            perror("Accepting client failed");

        }
        
        /* create child process to handle connection*/
        pid_t pid = fork();

        /*
        The key point is:
        when fork() creates a child process, the child process "inherits" all resources from the parent process, 
        including the file descriptor client_fd. This allows the child process to communicate with the client independently.
        
        in child process, after redirect the output to client_fd, child process no longer needs client_fd, so it uses close(client_fd)
        
        */

        if (pid == -1){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }else if (pid == 0){
            /* Here are the child codes */
            if (dup2(client_fd, STDOUT_FILENO) < 0) {
                perror("Dup2 failed");
                return 1;
            }
            close(server_fd);

            int child_pid = getpid();
            printf("Train ID: %d\n", child_pid);

            if(execlp("sl", "sl", "-l", NULL) <0 ){
                perror("Execlp failed");
                exit(EXIT_FAILURE);
            }
        }else{
            /* Happens when pid > 0 */
            /* Here are the parent codes */
            printf("Train ID: %d\n", pid); // child pid
            close(client_fd);
        }
    }

    return 0;

}