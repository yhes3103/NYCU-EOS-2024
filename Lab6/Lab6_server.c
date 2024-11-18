# include <stdio.h>
# include <unistd.h> // for fork() exec() read() close() lseek() dup() ...
# include <stdlib.h>
# include <sys/socket.h> // for socket
#include <netinet/in.h>  // sockaddr_in, htons, INADDR_ANY
#include <arpa/inet.h>   // htons, inet_ntoa
# include <sys/sem.h> // for semaphore
# include <string.h>
# include <errno.h>
# include <signal.h>
#include <sys/shm.h>


# define SEM_MODE 666
# define SEM_KEY 1122334455
# define BUFFERSIZE 256

# define SHM_KEY 11223344

int server_fd;
int* balance;
int shmid;

/* Close socket and remove semaphore when catching SIGINT signal ( Signal Interrupt such as Ctrl + C ) */
void handle_sigint(int signum) { 

    close(server_fd);

    int s = semget(SEM_KEY, 1, 0);
    if (s >= 0) {
        semctl(s, 0, IPC_RMID, 0);
    }
    
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
            perror("shmctl failed");
            exit(EXIT_FAILURE);
        }
    exit(0); 
}

/* P () - returns 0 if OK; -1 if there was a problem */
/* for acquire (-1) */
int P(int s){

    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0;   /* access the 1st (and only) sem in the array */
    sop.sem_op = -1;   /* wait, acquire */
    sop.sem_flg = 0;   /* no special options needed */

    if (semop(s, &sop, 1) < 0) { // semaphore operation to acquire
        fprintf(stderr, "P(): semop failed: %s\n", strerror(errno));
        return -1;
    } else {
        return 0;
    }
}

/* V() - returns 0 if OK; -1 if there was a problem */
int V(int s) {

    struct sembuf sop; /* the operation parameters */
    sop.sem_num = 0;   /* the 1st (and only) sem in the array */
    sop.sem_op = 1;    /* signal */
    sop.sem_flg = 0;   /* no special options needed */

    if (semop(s, &sop, 1) < 0) {
        fprintf(stderr, "V(): semop failed: %s\n", strerror(errno));
        return -1;
    } else {
        return 0;
    }
}


void deposit(int amount, int times){
    int s = semget(SEM_KEY, 1, 0);

    for (int i = 0 ; i < times ; i++){
        /* Entering Critical Section */
        P(s);
        *balance += amount;
        printf("After deposit: %d\n", *balance);
        V(s);
        /* Leaving Critical Section */
    }
}

void withdraw(int amount, int times){
    int s = semget(SEM_KEY, 1, 0);

    for (int i = 0 ; i < times ; i++){
        /* Entering Critical Section */
        P(s);
        *balance -= amount;
        printf("After Withdraw: %d\n", *balance);
        V(s);
        /* Leaving Critical Section */
    }
}

void child_func(int client_fd){
    char recv_buf[BUFFERSIZE] = {0};
    char temp[15];
    int amount, times;

    while(1){
        memset(recv_buf, 0 , BUFFERSIZE);
        /* receive message, if no message then block */
        // printf("%s", recv_buf);

        if (recv(client_fd, recv_buf, BUFFERSIZE, 0) > 0) {
           sscanf(recv_buf, "%s %d %d", temp, &amount, &times);

        if (strncmp(temp, "deposit", 7) == 0){
            deposit(amount, times);
        }else if (strncmp(temp, "withdraw", 8) == 0){
            withdraw(amount, times);
        }else{
            /* Invalid Command then continue*/
            printf("Invalid command.\n");
            break;
        }
        }
        
    }
    exit(0);
}


int main(int argc, char* argv[]){
    int s; // for semaphore id
    signal(SIGINT, handle_sigint);


    if (argc != 2){
        printf("Usage: ./lab6_server <port>\n");
        exit(EXIT_FAILURE);
    }
    
    /* Create shared memory to store variable balance */
    if ((shmid = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* Attach to parent process(Main process) */
    if ((balance = shmat(shmid, NULL, 0)) == (int *) -1) {
        perror("shmat");
        exit(1);
    }

    *balance = 0;

    /* Create a binary semaphore */
    s = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | SEM_MODE);
    if (s < 0){
        perror("Semaphore create failed");
        exit(EXIT_FAILURE);
    }

    /* Set semaphore initial value */
    int val = 1;
    if (semctl(s, 0, SETVAL, val) < 0){
        perror("Semaphore set value to 1 failed");
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

    // AF_INET = IPv4
    // SOCK_STREAM = TCP
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // start binding
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Server binding failed");
        exit(EXIT_FAILURE);
    }

    // start listening with maximum 5 client connecting
    if (listen(server_fd, 5) < 0) {
        perror("Server listening failed");
        exit(EXIT_FAILURE);
    }
    printf("Server is now listening...\n");

    while(1){

        int client_fd =accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_fd < 0){
            perror("Accepting client failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();
        if (pid < 0){
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }else if(pid == 0){
            close(server_fd);
            child_func(client_fd);
            close(client_fd);
        }else{
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;

    
}