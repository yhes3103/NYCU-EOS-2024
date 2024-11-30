# include <stdio.h>
# include <unistd.h> // for fork() exec() read() close() lseek() dup() getpid() ...
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <signal.h>
# include <sys/shm.h> // for shared memory
# include <signal.h>

typedef struct data{
    int guess;
    char result[8];
} DATA;

int shmid;
DATA* shm;
int ans;
pid_t pid;

void SIGUSR1_handler(int signal_number){
    int guess = shm-> guess;

    if (guess > ans){
        strcpy(shm->result, "smaller");
        printf("[Game] Guess: %d, %s\n", shm->guess, shm->result);
    }else if (guess < ans){
        strcpy(shm->result, "bigger");
        printf("[Game] Guess: %d, %s\n", shm->guess, shm->result);
    }else{
        strcpy(shm->result, "bingo");
        printf("[Game] Guess: %d, %s\n", shm->guess, shm->result);
        kill(pid, SIGUSR2); 
    }
    
}

void SIGUSR2_handler(int signum) { 
    
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }
    exit(0); 
}

int main(int argc, char* argv[]){
    
    signal(SIGUSR2, SIGUSR1_handler);
    if (argc != 3){
        printf("Usage: ./game <shm key> <guess>\n");
        exit(EXIT_FAILURE);
    }
    pid = getpid();
    printf("[Game] Game PID: %d\n", pid);

    int shm_key = atoi(argv[1]);
    ans = atoi(argv[2]);

    /* Create the segment */
    if ((shmid = shmget(shm_key, sizeof(DATA), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* Now we attach the segment to our data space */
    if ((shm = (DATA *)shmat(shmid, NULL, 0)) == (DATA *) -1) {
        perror("shmat");
        exit(1);
    }
    strcpy(shm->result, "smaller");

    struct sigaction sa, sa2;
    memset(&sa, 0, sizeof(sa)); 
    sa.sa_handler = &SIGUSR1_handler; 
    sigaction(SIGUSR1, &sa, NULL);

    memset(&sa2, 0, sizeof(sa2));
    sa2.sa_handler = &SIGUSR2_handler;
    sigaction(SIGUSR2, &sa2, NULL);

    while(1){
        pause();
    }
}
