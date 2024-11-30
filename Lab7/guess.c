# include <stdio.h>
# include <unistd.h> // for fork() exec() read() close() lseek() dup() getpid() ...
# include <stdlib.h>
# include <string.h>
# include <errno.h>
# include <signal.h>
# include <sys/shm.h> // for shared memory
# include <signal.h>
# include <sys/time.h> 

typedef struct data{
    int guess;
    char result[8];
} DATA;

int shmid;
DATA* shm;
int game_pid;
int upper_bound;
int lower_bound=0;
int guess;



void timer_handler(int signum) {

    kill(game_pid, SIGUSR1); 
    usleep(50000);

    if (strncmp(shm->result, "smaller", 7) == 0) {

        printf("[Guess] Guess: %d\n", shm->guess);
        upper_bound = shm->guess ;  
        shm->guess = (upper_bound + lower_bound) / 2; 

    } else if (strncmp(shm->result, "bigger", 6) == 0) {

        printf("[Guess] Guess: %d\n", shm->guess);
        lower_bound = shm->guess ;  
        shm->guess = (upper_bound + lower_bound) / 2; 

    } else if (strncmp(shm->result, "bingo", 5) == 0) {
        
        printf("[Guess] Guess: %d\n", shm->guess);
        exit(0);  
    }
    
}


int main(int argc, char* argv[]){

    if (argc != 4){ 
        printf("Usage: guess <key> <upper_bound> <pid>\n");
        exit(EXIT_FAILURE);
    }
    
    int shm_key = atoi(argv[1]);
    upper_bound = atoi(argv[2]);
    game_pid = atoi(argv[3]);

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

    shm->guess=upper_bound;


    struct itimerval timer; 
    /* Configure the timer to expire after 1 sec */ 
    timer.it_value.tv_sec = 1; 
    timer.it_value.tv_usec = 0; 

    /* Reset the timer back to 1 sec after expired */ 
    timer.it_interval.tv_sec = 1; 
    timer.it_interval.tv_usec = 0; 
    setitimer(ITIMER_REAL, &timer, NULL); 

    /* Set the timer_handler and use SIGUSR1 to signal game process*/
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa)); 
    sa.sa_handler = &timer_handler; 
    sigaction(SIGALRM, &sa, NULL);

    while(1){
        pause();
    }
}