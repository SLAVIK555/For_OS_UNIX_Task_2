/* 
 * Standard in kernel modules 
 */

/* 
 * ТРЕБУЕТСЯ ЯДРО ВЕРСИИ 2.x.x 
 */
#include <linux/kernel.h>    /* Все-таки мы работаем с ядром! */
#include <linux/module.h>    /* Необходимо для любого модуля */
#include <linux/workqueue.h> /* очереди задач */
#include <linux/sched.h>     /* Взаимодействие с планировщиком */
#include <linux/interrupt.h> /* определение irqreturn_t */
#include <asm/io.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 
#include <linux/uaccess.h>              
#include <linux/ioctl.h>

#define MY_WORK_QUEUE_NAME "WQsched.c"

static long kbi_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
int global_keyboard_incrimetn = 0;

dev_t dev = 0;
static struct cdev kbi_cdev;

/*
** File operation sturcture
*/
static struct file_operations fops =
{
    .unlocked_ioctl = kbi_ioctl,
};


static struct workqueue_struct *my_workqueue;

/* 
 * Эта функция вызывается ядром, поэтому в ней будут безопасны все действия
 * которые допустимы в модулях ядра.
 */
// static void got_char(void *scancode)
// {
//   printk("Scan Code %x %s.\n",
//         (int)*((char *)scancode) & 0x7F,
//         *((char *)scancode) & 0x80 ? "Released" : "Pressed");
// }

/* 
 * Обработчик прерываний от клавиатуры. Он считывает информацию с клавиатуры
 * и передает ее менее критичной по времени исполнения части,
 * которая будет запущена сразу же, как только ядро сочтет это возможным.
 */
irq_handler_t irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
  /* 
   * Эти переменные объявлены статическими, чтобы имелась возможность
   * доступа к ним (посредством указателей) из "нижней половины".
   */
  unsigned char scancode;
  unsigned char status;


  //int value;

  /*
  * Прочитать состояние клавиатуры
  */
  status = inb(0x64);
  scancode = inb(0x60);

  printk("Scan Code %x %s.\n",
        (int)*((char *)scancode) & 0x7F,
        *((char *)scancode) & 0x80 ? "Released" : "Pressed");

  return IRQ_HANDLED;
}

/*
 * Наш ioctl для считывания глобального счетчика клавиатурных прерываний
 */
static long kbi_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
  if(cmd == 0){//Пользователь подает команду на считывание количества прерываний
        copy_to_user((int32_t*) arg, &global_keyboard_incrimetn, sizeof(global_keyboard_incrimetn));
  }

  return 0;
}

/* 
 * Инициализация модуля - регистрация обработчика прерывания
 */
int init_module()
{
  /*Allocating Major number*/
  if((alloc_chrdev_region(&dev, 0, 1, "etx_Dev")) <0){
      pr_err("Cannot allocate major number\n");
      return -1;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
  /*Creating cdev structure*/
  cdev_init(&kbi_cdev,&fops);
 
  /*Adding character device to the system*/
  if((cdev_add(&kbi_cdev,dev,1)) < 0){
      pr_err("Cannot add the device to the system\n");
      unregister_chrdev_region(dev,1);
      return -1;
  }


  my_workqueue = create_workqueue(MY_WORK_QUEUE_NAME);

  /* 
   * Поскольку стандартный обработчик прерываний от клавиатуры не может
   * сосуществовать с таким как наш, то придется запретить его
   * (освободить IRQ) прежде, чем что либо сделать.
   * Но поскольку мы не знаем где он находится в ядре, то мы лишены
   * возможности переустановить его - поэтому компьютер придется перезагрузить
   * после опробования этого примера.
   */
        free_irq(1, NULL);

  /* 
   * Подставить свой обработчик (irq_handler) на IRQ 1.   
   * SA_SHIRQ означает, что мы допускаем возможность совместного
   * обслуживания этого IRQ другими обработчиками.
   */
  return request_irq(1,    /* Номер IRQ */
         irq_handler,      /* наш обработчик */
         IRQF_SHARED,      //SA_SHIRQ,
         "test_keyboard_irq_handler",
         (void *)(irq_handler));
}

/* 
 * Завершение работы
 */
void cleanup_module()
{
  /* 
   * Эта функция добавлена лишь для полноты изложения. 
   * Она вообще бессмысленна, поскольку я не вижу способа
   * восстановить стандартный обработчик прерываний от клавиатуры 
   * поэтому необходимо выполнить перезагрузку системы.
   */
  free_irq(1, NULL);
}

/* 
 * некоторые функции, относящиеся к work_queue 
 * доступны только если модуль лицензирован под GPL
 */
MODULE_LICENSE("GPL");