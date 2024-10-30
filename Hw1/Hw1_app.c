#include <unistd.h> // system call functions such as read, write, lseek, fork
#include <stdio.h>
#include <fcntl.h> // file control, use for open, O_RDONLY
#include <stdlib.h> // use for malloc, free, rand, exit
#include <string.h> // use for string functions
#include <termios.h> // change input mode


#define DEVICE_LED "/dev/led_device"
#define DEVICE_SEG "/dev/seg_device"
static struct termios stored_settings;

void set_keypress (void)
{
  struct termios new_settings;

  tcgetattr (0, &stored_settings);

  new_settings = stored_settings;

  /* Disable canonical mode, and set buffer size to 1 byte */
  new_settings.c_lflag &= (~ICANON);
  new_settings.c_cc[VTIME] = 0;
  new_settings.c_cc[VMIN] = 1;

  tcsetattr (0, TCSANOW, &new_settings);
  return;
}

void reset_keypress (void)
{
  tcsetattr (0, TCSANOW, &stored_settings);
  return;
}


void write_seg_device(int total_cost){
    int fd = open(DEVICE_SEG, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device");
        return; // Return instead of EXIT_FAILURE
    }
   
    char cost[10]; 
    snprintf(cost, sizeof(cost), "%d", total_cost); // to change integer to char

    for (int i = 0; cost[i] != '\0'; i++) {
        ssize_t bytes_written = write(fd, &cost[i], 1);
        if (bytes_written == -1) {
            perror("Failed to write to the device");
            close(fd);
            return; // Return instead of EXIT_FAILURE
        }
        usleep(500000);
    }
    char x = 'x';   /////////////////////////////////////////////////////////////
    write(fd, &x, 1); // stop displaying any digit
    close(fd);
}

void write_led_device(int distance){
    int fd = open(DEVICE_LED, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device");
        return; // Return instead of EXIT_FAILURE
    }

char data[10] = {'9', '8', '7', '6', '5', '4', '3', '2', '1', '0'};
 
for (int i = 9 - distance; i <= 9; i++) {
    ssize_t bytes_written = write(fd, &data[i], 1);
    // printf("Writing '%c' to LED device\n", data[i]);
    if (bytes_written == -1) {
        perror("Failed to write to the device");
        close(fd);
        return;
    }
    sleep(1);
}
    close(fd);
}

void order(char* shop_name, char* item1, char* item2, int price1, int price2, int* order1, int* order2){
    int order_choice;
    while(1){
        system("clear"); // Change to clear for Linux
        printf("<!--%s-->\n", shop_name);
        printf("Please choose from 1~4: \n");
        printf("1. %s: $%d\n", item1, price1);
        printf("2. %s: $%d\n", item2, price2);
        printf("3. Confirm\n");
        printf("4. Cancel\n");
        scanf("%d", &order_choice);
        system("clear");
        
        if (order_choice == 1 ){
            int amount; 
            system("clear");
            printf("How many?  \n");
            scanf("%d", &amount);
            *order1 += amount; // Simplified
        } else if (order_choice == 2) {
            int amount; 
            system("clear");
            printf("How many?  \n");
            scanf("%d", &amount);
            *order2 += amount; // Simplified
        } else if (order_choice == 3) {
            if (*order1 == 0 && *order2 == 0) {
                printf("Nothing to order\n");
                set_keypress();
                printf("<!-- 按任意鍵回主選單 -->\n");
                getchar(); // Use getchar to pause
                getchar();
                reset_keypress();
                system("clear");
                break;
            }
            printf("<!--Order sent-->\n");
            printf("%s: %d \n", item1, *order1);
            printf("%s: %d \n", item2, *order2);
            int total_cost = price1 * (*order1) + price2 * (*order2);
            printf("Total cost: %d\n", total_cost);
            if (strcmp(shop_name, "Dessert Shop") == 0) {
                write_led_device(3);
            } else if (strcmp(shop_name, "Beverage Shop") == 0) {
                write_led_device(5);
            } else if (strcmp(shop_name, "Diner") == 0) {
                write_led_device(8);
            }
            write_seg_device(total_cost);

            *order1 = 0; // set to 0 after delivery
            *order2 = 0;
            getchar(); // Use getchar to pause
            system("clear");
            break;
        } else if (order_choice == 4) {
            *order1 = 0;
            *order2 = 0;
            printf("Order canceled.\n");
            getchar(); // Use getchar to pause
            system("clear");
            break;
        } else {
            printf("Invalid choice, please choose again.\n");
            getchar(); // Use getchar to pause
            system("clear");
        }
    }
}

int main() {
    int menuchoice = 0, shopchoice = 0;
    int order_amount[3][2] = {0};

    while(1) {
        printf("<!-- 初始狀態 -->\n");
        printf("<!-- 主選單 -->\n");
        printf("1. shop list\n");
        printf("2. order\n");
        scanf("%d", &menuchoice);

        if (menuchoice == 1) {
            system("clear");
            printf("<!--shop list-->\n");
            printf("Dessert shop: 3km\n");
            printf("Beverage shop: 5km\n");
            printf("Diner: 8km\n");
            set_keypress();
            printf("<!-- 按任意鍵回主選單 -->\n");
            getchar(); // Use getchar to pause
            getchar();
            reset_keypress();
            system("clear");
        } else if (menuchoice == 2) {
            system("clear");
            printf("<!--order-->\n");
            printf("Please choose from 1~3: \n");
            printf("1. Dessert shop\n");
            printf("2. Beverage shop\n");
            printf("3. Diner\n");
            scanf("%d", &shopchoice);
            system("clear");

            if (shopchoice == 1) {
                order("Dessert Shop", "Cookie", "Cake", 60, 80, &order_amount[0][0], &order_amount[0][1]);
            } else if (shopchoice == 2) {
                order("Beverage Shop", "Tea", "Boba", 40, 70, &order_amount[1][0], &order_amount[1][1]);
            } else if (shopchoice == 3) {
                order("Diner", "Fried rice", "Egg-drop soup", 120, 50, &order_amount[2][0], &order_amount[2][1]);
            } else {
                printf("Invalid input, please choose again: ");
                getchar(); // Use getchar to pause
                system("clear");
            }
        }else{
            printf("Invalid input, please choose again: ");
            set_keypress();
            system("clear");
            printf("\n<!-- 按任意鍵回主選單 -->\n");
            getchar(); // Use getchar to pause
            getchar();
            reset_keypress();
            system("clear");
        }
    }
}
