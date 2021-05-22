#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main()
{
        int fd;
        int32_t number;
        printf("*********************************\n");
 
        printf("\nOpening Driver\n");
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

        ioctl(fd, 0, (int32_t*) &number); 

        printf("%d\n", number);
 
        printf("Closing Driver\n");
        close(fd);
}