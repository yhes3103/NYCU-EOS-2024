# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>  
# include <arpa/inet.h>
# include <signal.h>   // signal()
# include <sys/wait.h> // waitpid()
# include <sys/shm.h>
# include <sys/sem.h> // for semaphore
# include <errno.h>
# include <sys/time.h> 

# define BUFFERSIZE 256
# define SHM_KEY 1234
# define SEM_KEY 12345


/* to store 2 delivery men's waiting time */
typedef struct waitstatus{
    int waiting_time[2];
} WAITSTATUS;

int shmid;
int s; // for semaphore id
WAITSTATUS* shm;

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

void timer_handler(int signum) { 
    if (shm->waiting_time[0] > 0){
        shm->waiting_time[0]--;
    }
    if (shm -> waiting_time[1] > 0){
        shm->waiting_time[1]--; 
    } 
    printf("%d, %d\n", shm->waiting_time[0], shm->waiting_time[1]);
} 

void SIGINT_handler(int sugnum){

    /* Remove shared memory */
    if (shmctl(shmid, IPC_RMID, NULL) < 0) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }

    /* Remove semaphore */
    int s = semget(SEM_KEY, 1, 0);
    if (s >= 0) {
        semctl(s, 0, IPC_RMID, 0);
    }
    exit(0);
}

void zombie_handler(int signum) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}


void send_msg(int client_fd, char str[]){
    char send_buf[BUFFERSIZE] = {0};
    memset(send_buf, '\0', BUFFERSIZE);
    strcpy(send_buf, str);
    strcat(send_buf, "\n");
    send(client_fd, send_buf, 256, 0);
}

struct shop_menu{
    char shop_name[15];
    char food_name1[15];
    char food_name2[15];
    int price1;
    int price2;
    int distance;
};

struct shop_menu menus[3] = {
    {"Dessert shop", "cookie", "cake", 60, 80, 3},
    {"Beverage shop", "tea", "boba", 40, 70, 5},
    {"Diner", "fried-rice", "Egg-drop-soup", 120, 50, 8}
};

struct order{
    int amount1;
    int amount2;
    int restaurant_choice;
};


void update_order(struct order *order, int new_amount1, int new_amount2) {
    if (order != NULL) {
        order->amount1 = new_amount1;
        order->amount2 = new_amount2;
    }
}

void confirm_order(WAITSTATUS* shm, int n, int client_fd, char recv_buf[]) {

    int waiting_time;
    if (shm->waiting_time[0] == 0 && shm->waiting_time[1] == 0) {
        shm->waiting_time[0] += n;
        waiting_time = shm->waiting_time[0];
    } else if (shm->waiting_time[0] == 0 || shm->waiting_time[1] == 0) {
        if (shm->waiting_time[0] == 0) {
            shm->waiting_time[0] += n;
            waiting_time = shm->waiting_time[0];
        } else {
            shm->waiting_time[1] += n;
            waiting_time = shm->waiting_time[1];
        }
    } else {
        if (shm->waiting_time[0] <= shm->waiting_time[1]) {
            shm->waiting_time[0] += n;
            waiting_time = shm->waiting_time[0];
        } else {
            shm->waiting_time[1] += n;
            waiting_time = shm->waiting_time[1];
        }
    }

    if (waiting_time >= 30) {
        send_msg(client_fd, "Your delivery will take a long time, do you want to wait?");
        memset(recv_buf, '\0', BUFFERSIZE);
        recv(client_fd, recv_buf, BUFFERSIZE, 0);

        if (strncmp(recv_buf, "Yes", 3) == 0) {
            send_msg(client_fd, "Please wait a few minutes...");
        } else if (strncmp(recv_buf, "No", 2) == 0) {
            exit(0);
        }
    } else {
        send_msg(client_fd, "Please wait a few minutes...");
    }
}


void handle_client(int client_fd) {

    char recv_buf[256] = {0};
    char temp[256] = {0};

    struct order *myorder = (struct order *)malloc(sizeof(struct order));
    if (!myorder) {
        perror("Failed to allocate memory for order");
        exit(EXIT_FAILURE);
    }

    myorder->amount1 = 0;
    myorder->amount2 = 0;
    myorder->restaurant_choice = -1;

    char order_item[15] = {0};
    int amount;
    
    while(1){
        memset(recv_buf, '\0', BUFFERSIZE);
        recv(client_fd, recv_buf, BUFFERSIZE, 0);

        if (strncmp(recv_buf, "shop list", 9) == 0) {

            // send shop list
            send_msg(client_fd, "Dessert shop:3km\n- cookie:$60|cake:$80\nBeverage shop:5km\n- tea:$40|boba:$70\nDiner:8km\n- fried-rice:$120|Egg-drop-soup:$50");
        } 
        else if (strncmp(recv_buf, "order", 5) == 0) {

            //separate each input
            sscanf(recv_buf, "%s %s %d", temp, order_item, &amount);

            // first time order to choose which restaurant
            if (myorder->restaurant_choice == -1) {


                for (int i = 0; i < 3; ++i) {
                    if (strncmp(order_item, menus[i].food_name1, sizeof(menus[i].food_name1)) == 0 ||
                        strncmp(order_item, menus[i].food_name2, sizeof(menus[i].food_name2)) == 0) {
                        myorder->restaurant_choice = i;
                        break;
                    }
                }

                //  check which food and send ordered list to client for the first time
                if (myorder->restaurant_choice != -1) {
                    if (strncmp(order_item, menus[myorder->restaurant_choice].food_name1, sizeof(menus[myorder->restaurant_choice].food_name1)) == 0) {
                        update_order(myorder, amount, 0);
                        sprintf(temp, "%s %d", order_item, myorder->amount1);
                    } else if (strncmp(order_item, menus[myorder->restaurant_choice].food_name2, sizeof(menus[myorder->restaurant_choice].food_name2)) == 0) {
                        update_order(myorder, 0, amount);
                        sprintf(temp, "%s %d", order_item, myorder->amount2);
                    }

                    send_msg(client_fd, temp);
                }
            } 
            // Second or more orders
            else {
                // check is the food within the same restaurant
                if ((strncmp(order_item, menus[myorder->restaurant_choice].food_name1, sizeof(menus[myorder->restaurant_choice].food_name1)) == 0) ||
                    (strncmp(order_item, menus[myorder->restaurant_choice].food_name2, sizeof(menus[myorder->restaurant_choice].food_name2)) == 0)) {

                    // check which food you've order with the same reataurant
                    if (strncmp(order_item, menus[myorder->restaurant_choice].food_name1, sizeof(menus[myorder->restaurant_choice].food_name1)) == 0) {
                        update_order(myorder, myorder->amount1 + amount, myorder->amount2);
                    } else if (strncmp(order_item, menus[myorder->restaurant_choice].food_name2, sizeof(menus[myorder->restaurant_choice].food_name2)) == 0) {
                        update_order(myorder, myorder->amount1, myorder->amount2 + amount);
                    }

                    // send order list and decide two kinds of food or a kind of food
                    if (myorder->amount1 != 0 && myorder->amount2 == 0) {
                        sprintf(temp, "%s %d", menus[myorder->restaurant_choice].food_name1, myorder->amount1);
                        send_msg(client_fd, temp);
                    } else if (myorder->amount1 == 0 && myorder->amount2 != 0) {
                        sprintf(temp, "%s %d", menus[myorder->restaurant_choice].food_name2, myorder->amount2);
                        send_msg(client_fd, temp);
                    } else {
                        char add[40];
                        sprintf(add, "%s %d|%s %d", menus[myorder->restaurant_choice].food_name1, myorder->amount1, menus[myorder->restaurant_choice].food_name2, myorder->amount2);
                        send_msg(client_fd, add);
                    }

                // if with the different restaurant
                } else {
                
                    // send the current order list
                    if (myorder->amount1 != 0 && myorder->amount2 == 0) {
                        sprintf(temp, "%s %d", menus[myorder->restaurant_choice].food_name1, myorder->amount1);
                        send_msg(client_fd, temp);
                    } else if (myorder->amount1 == 0 && myorder->amount2 != 0) {
                        sprintf(temp, "%s %d", menus[myorder->restaurant_choice].food_name2, myorder->amount2);
                        send_msg(client_fd, temp);
                    } else {
                        sprintf(temp, "%s %d|%s %d", menus[myorder->restaurant_choice].food_name1, myorder->amount1, menus[myorder->restaurant_choice].food_name2, myorder->amount2);
                        send_msg(client_fd, temp);
                    }
                }
            }
        }else if(strncmp(recv_buf, "confirm", 7) == 0){
   
            char paid[256] = {0};
            int cost = 0;
        
            if(myorder->amount1 == 0 && myorder->amount2 == 0){
                send_msg(client_fd, "Please order some meals"); 

            }else{
                if (myorder->restaurant_choice == 0){
                    cost = 60*myorder->amount1 + 80*myorder->amount2;
                    //send_msg(client_fd, "Please wait a few minutes...");

                    confirm_order(shm, 3, client_fd, recv_buf);

                    P(s);
                    sleep(3);
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
                    V(s);

                }else if(myorder->restaurant_choice == 1){
                    //send_msg(client_fd, "Please wait a few minutes...");
                    cost = 40*myorder->amount1 + 70*myorder->amount2;

                    confirm_order(shm, 5, client_fd, recv_buf);

                    P(s);
                    sleep(5);
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
                    V(s);

                }else if(myorder->restaurant_choice == 2){
                    //send_msg(client_fd, "Please wait a few minutes...");
                    cost = 120*myorder->amount1 + 50*myorder->amount2;

                    confirm_order(shm, 8, client_fd, recv_buf);

                    P(s);
                    sleep(8);
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
                    V(s);
                }    
                exit(0);
            }
        }else if(strncmp(recv_buf, "cancel", 6) == 0){
            exit(0);
        }
    }
    free(myorder);   
}

int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: ./hw3 <port>\n");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa; 
    struct itimerval timer; 

    /* Install timer_handler as the signal handler for SIGVTALRM */ 
    /*
    問題的根本原因在於程式中使用了定時器（setitimer），這會觸發 SIGALRM 信號，
    而這個信號可能會中斷正被阻塞的accept，而造成system interrupt，所以使用SA_RESTART

    處理signal時，SA_RESTART用來告訴kernel：如果信號中斷了某些系統呼叫，那麼在信號處理完成後，
    自動重新啟動被中斷的呼叫，而不是直接返回錯誤。
    如果不使用 SA_RESTART，程式可能因為 accept 被信號中斷而直接報錯，甚至導致程式崩潰。
    */
    memset(&sa, 0, sizeof(sa)); 
    sa.sa_handler = &timer_handler; 
    sa.sa_flags = SA_RESTART;  
    sigaction(SIGALRM, &sa, NULL); 

    /* Configure the timer to expire after 1 sec */ 
    timer.it_value.tv_sec = 1; 
    timer.it_value.tv_usec = 0; 

    /* Reset the timer back to 1 sec after expired */ 
    timer.it_interval.tv_sec = 1; 
    timer.it_interval.tv_usec = 0; 

    /* Start a real timer */ 
    setitimer(ITIMER_REAL, &timer, NULL); 

    /* Ctrl+C to remove semaphore and shared memory */    
    struct sigaction sa2;
    memset(&sa2, 0, sizeof(sa2)); 
    sa2.sa_handler = &SIGINT_handler; 
    // sa2.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa2, NULL); 

    /* zombie process handler */
    signal(SIGCHLD, zombie_handler);
     


    /* Create shared memory to store variable balance */
    if ((shmid = shmget(SHM_KEY, sizeof(WAITSTATUS), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    /* Attach to parent process(Main process) */
    if ((shm = (WAITSTATUS* )shmat(shmid, NULL, 0)) == (WAITSTATUS *) -1) {
        perror("shmat");
        exit(1);
    }

    shm->waiting_time[0] = 0;
    shm->waiting_time[1] = 0;

    /* Create a binary semaphore */
    s = semget(SEM_KEY, 1, IPC_CREAT | IPC_EXCL | 666);
    if (s < 0){
        perror("Semaphore create failed");
        exit(EXIT_FAILURE);
    }

    /* Set semaphore initial value = 2 refers to two delivery man */
    int val = 2;
    if (semctl(s, 0, SETVAL, val) < 0){
        perror("Semaphore set value to 1 failed");
        exit(EXIT_FAILURE);
    }




    int port = atoi(argv[1]);

    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    int server_fd;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // start binding
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // start listening
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        
        int client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        
        if (client_fd < 0) {
            perror("Client accept failed");
            exit(EXIT_FAILURE);
        }

        pid_t pid = fork();

        if(pid == 0){
            close(server_fd);
            
            handle_client(client_fd);
            
            close(client_fd);
        }else if (pid > 0) {
            close(client_fd);  
        } else {
            perror("Fork failed");
            close(client_fd);
        }

    }

    close(server_fd);
}
