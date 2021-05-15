#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
 
//#define WR_VALUE _IOW('a','a',int32_t*)
//#define RD_VALUE _IOR('a','b',int32_t*)
#define MESSAGE_SIZE 50

struct Frame{
int time;
int pid;
char message[MESSAGE_SIZE];
};

struct Frame CharBufToFrame(char buf[])
{
    struct Frame frame; //re make struct
    ssize_t size = sizeof(frame);
    memcpy(&frame, buf, size);
    //(*len) = size;
    return frame;
}

char FrameToCharBuf(struct Frame frame)
{
    ssize_t size = sizeof(frame);
    char buf[size];
    memcpy(buf, &frame, size);
    //(*len) = size;
    return (*buf);
}
 
int main()
{
        int fd;
        //int32_t value, number;
        printf("*********************************\n");
 
        printf("\nOpening Driver\n");
        fd = open("/dev/etx_device", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }

        struct Frame frame;
        frame.time = 1;
        frame.pid = 1;
        //char msg1[MESSAGE_SIZE] = {'1','s','t'};
        frame.message[0] = '1';
        char buffer1 = FrameToCharBuf(frame);
        write(fd, buffer1, strlen(buffer1)+1);

        frame.time = 2;
        frame.pid = 2;
        //char msg2[MESSAGE_SIZE] = {'2','n','d'};
        frame.message[0] = '2';
        char buffer2 = FrameToCharBuf(frame);
        write(fd, buffer2, strlen(buffer2)+1);

        frame.time = 3;
        frame.pid = 3;
        //char msg3[MESSAGE_SIZE] = {'3','r','d'};
        frame.message[0] = '3';
        char buffer3 = FrameToCharBuf(frame);
        write(fd, buffer3, strlen(buffer3)+1);

        int32_t nom;
        ioctl(fd, -1, (int32_t*) &nom); 
        printf("%d\n", nom);

        char rbuffer[100];
        read(fd, rbuffer, 100);

        //int32_t nom;
        ioctl(fd, -1, (int32_t*) &nom); 
        printf("%d\n", nom);

        // printf("Enter the Value to send\n");
        // scanf("%d",&number);
        // printf("Writing Value to Driver\n");
        // ioctl(fd, WR_VALUE, (int32_t*) &number); 
 
        // printf("Reading Value from Driver\n");
        // ioctl(fd, RD_VALUE, (int32_t*) &value);
        // printf("Value is %d\n", value);
 
        printf("Closing Driver\n");
        close(fd);
}