#include <stdio.h>
#include <fcntl.h>
#include <unistd.h> // for sleep funciton
#include <string.h>
#include <stdlib.h>

#define DEVICE "/dev/my_device"

int main(int argc,char *argv[]) {

    if(argc != 2) {
        fprintf(stderr, "Usage: ./writer <name>\n");
        exit(EXIT_FAILURE);
    }
    
    // open device 
    int fd = open(DEVICE, O_WRONLY);
    if (fd == -1) {
        perror("Failed to open the device");
        return EXIT_FAILURE;
    }

    int length = strlen(argv[1]);
    printf("Write %ld bytes to the device: %s\n", length, argv[1]);

    sleep(3);
    for (int i = 0 ; i < length ; i++){
        ssize_t bytes_written = write(fd, &argv[1][i], 1);
        if (bytes_written == -1) {
            perror("Failed to write to the device");
            close(fd);
        return EXIT_FAILURE;
        }
        sleep(1);
    }

    close(fd);
    return EXIT_SUCCESS;
}
