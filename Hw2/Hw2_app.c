#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <arpa/inet.h>
#include <signal.h>   // signal()
#include <sys/wait.h> // waitpid()

#define BUFFERSIZE 256

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

void handle_client(int client_fd) {

    char recv_buf[30] = {0};
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
                    send_msg(client_fd, "Please wait a few minutes...");
                    sleep(3);
                    cost = 60*myorder->amount1 + 80*myorder->amount2;
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
                }else if(myorder->restaurant_choice == 1){
                    send_msg(client_fd, "Please wait a few minutes...");
                    sleep(5);
                    cost = 40*myorder->amount1 + 70*myorder->amount2;
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
                }else if(myorder->restaurant_choice == 2){
                    send_msg(client_fd, "Please wait a few minutes...");
                    sleep(8);
                    cost = 120*myorder->amount1 + 50*myorder->amount2;
                    sprintf(paid, "Delivery has arrived and you need to pay %d$", cost);
                    send_msg(client_fd, paid);
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
    signal(SIGCHLD, zombie_handler);
    if (argc != 2) {
        printf("Usage: ./hw2 <port>\n");
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