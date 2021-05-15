/***************************************************************************//**
*  \file       driver.c
*
*  \details    Simple Linux device driver (IOCTL)
*
*  \author     EmbeTronicX
*
*  \Tested with Linux raspberrypi 5.10.27-v7l-embetronicx-custom+
*
*******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 
#include <linux/uaccess.h>              
#include <linux/ioctl.h>
//#include <string.h>
 
 
#define WR_VALUE _IOW('a','a',int32_t*)///////////////
#define RD_VALUE _IOR('a','b',int32_t*)///////////////
#define SIZE_BUF 256
#define MESSAGE_SIZE 50
 
int32_t value = 0;
 
dev_t dev = 0;
static struct class *dev_class;
static struct cdev frame_cdev;

/*
** Struct Prototype
*/
struct Frame{
int time;
int pid;
char message[MESSAGE_SIZE];
};

unsigned char frame_tail = 0;      //"указатель" хвоста буфера
unsigned char frame_head = 0;   //"указатель" головы буфера
unsigned char frame_count = 0;  //счетчик символов

/*Creating frame buffer*/
struct Frame cycleFrameBuf[SIZE_BUF];

/*
** Function Prototypes
*/
static int      __init frame_driver_init(void);
static void     __exit frame_driver_exit(void);
static int      frame_open(struct inode *inode, struct file *file);
static int      frame_release(struct inode *inode, struct file *file);
static ssize_t  frame_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  frame_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     frame_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

/*
** File operation sturcture
*/
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = frame_read,
        .write          = frame_write,
        .open           = frame_open,
        .unlocked_ioctl = frame_ioctl,
        .release        = frame_release,
};

/*
** Another functions
*/
//"очищаем" буфер
void FlushFrameBuf(void)
{
  frame_tail = 0;
  frame_head = 0;
  frame_count = 0;
}

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

//положить символ в буфер
void PutFrameInBuffer(struct Frame frame)
{
  if (frame_count < SIZE_BUF){
    cycleFrameBuf[frame_tail] = frame;
    frame_count++;
    frame_tail++;
    if (frame_tail == SIZE_BUF){
      frame_tail = 0;
    }
  }
}

//взять символ из буфера
struct Frame GetFrameFromBuffer(void)
{
  struct Frame frame;
  if (frame_count > 0){
    frame = cycleFrameBuf[frame_head];
    frame_count--;
    frame_head++;
    if (frame_head == SIZE_BUF){
      frame_head = 0;
    }
  }
  return frame;
}

/*
** This function will be called when we open the Device file
*/
static int frame_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        /*Creating frame buffer*/
        struct Frame cycleFrameBuf[SIZE_BUF];

        unsigned char frame_tail = 0;      //"указатель" хвоста буфера
        unsigned char frame_head = 0;   //"указатель" головы буфера
        unsigned char frame_count = 0;  //счетчик символов
        return 0;
}

/*
** This function will be called when we close the Device file
*/
static int frame_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t frame_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        pr_info("Read Function\n");

        int byte_read = 0;

        struct Frame frame = GetFrameFromBuffer();

        char buffer = FrameToCharBuf(frame);

        //int i;
        //for (i=0; i<len; i++)
        //{
        //    put_user(buffer[i], buffer+i);
        //    byte_read++;
        //}
        copy_to_user(buf, &buffer, sizeof(buffer));

        byte_read = sizeof(buffer);

        return byte_read;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t frame_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        pr_info("Write function\n");

        char buffer[len];
        //int i;
        //for (i=0; i<len; i++)
        //{
        //    buffer[i] = (*buf[i]);
        //}
        copy_from_user(buffer, buf, len);

        struct Frame frame = CharBufToFrame(buffer);

        PutFrameInBuffer(frame);

        return len;
}

/*
** This function will be called when we write IOCTL on the Device file
*/
//copy_to_user((int32_t*) arg, &value, sizeof(value))
static long frame_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    // //struct Frame frame = cycleFrameBuf[arg];
    // //switch(cmd) {
    //     case 0://Пользователь получает колическво кадров
    //         copy_to_user((int32_t*) arg, &frame_count, sizeof(frame_count));
    //         break;

    //     case 1://Пользователь получает сообщение из i - ого кадра, здесь аргумент - номер кадра
    //         //char[] msg = frame.message;
    //         //int i;
    //         //ssize_t size = sizeof(msg);
    //         //for (i=0; i<size; i++)
    //         //{
    //         //    put_user(msg[i], msg+i);
    //         //}
    //         copy_to_user(argp, &frame.message, MESSAGE_SIZE);
    //         break;

    //     default:
    //         pr_info("Default\n");
    //         break;
    //     }
    if (cmd == -1){//Пользователь получает колическво кадров
        copy_to_user((int32_t*) arg, &frame_count, sizeof(frame_count));
    }
    
    if(cmd >= 0){//Пользователь получает сообщение из i - ого кадра, здесь команда - номер кадра, она не должна быть отрицательной
        struct Frame frame = cycleFrameBuf[cmd];
        copy_to_user((int32_t*) arg, &frame.message, MESSAGE_SIZE);
    }
    else{
        pr_info("cmd must be more, than -1, or equal\n");
    }
    
    return 0;
    //}
    //}
    //}
}
 
/*
** Module Init function
*/
static int __init frame_driver_init(void)
{
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
                pr_err("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&frame_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&frame_cdev,dev,1)) < 0){
            pr_err("Cannot add the device to the system\n");
            goto r_class;
        }
 
        /*Creating struct class*/
        if((dev_class = class_create(THIS_MODULE,"frame_class")) == NULL){
            pr_err("Cannot create the struct class\n");
            goto r_class;
        }
 
        /*Creating device*/
        if((device_create(dev_class,NULL,dev,NULL,"frame_device")) == NULL){
            pr_err("Cannot create the Device 1\n");
            goto r_device;
        }
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 
r_device:
        class_destroy(dev_class);
r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

/*
** Module exit function
*/
static void __exit frame_driver_exit(void)
{
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&frame_cdev);
        unregister_chrdev_region(dev, 1);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(frame_driver_init);
module_exit(frame_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SlavaBogomolov");
MODULE_DESCRIPTION("Frame Buffer Driver");
MODULE_VERSION("1");