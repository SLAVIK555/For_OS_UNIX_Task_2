#include <linux/kernel.h> /* Для printk() и т.д. */
#include <linux/module.h> /* Эта частичка древней магии, которая оживляет модули */
#include <linux/init.h> /* Определения макросов */
#include <linux/fs.h>
#include <asm/uaccess.h> /* put_user */
//#include <linux/ioctl.h>
//#include <sys/ioctl.h>  /* ioctl */


// Ниже мы задаём информацию о модуле, которую можно будет увидеть с помощью Modinfo
MODULE_LICENSE( "GPL" );
MODULE_SUPPORTED_DEVICE( "frame_cycle_buffer_driver" ); /* /dev/testdevice */

#define SUCCESS 0
#define DEVICE_NAME "frame_cycle_buffer_driver" /* Имя нашего устройства */
#define SIZE_BUF 256
#define MESSAGE_SIZE 50

struct Frame{
int time;
int pid;
char message[MESSAGE_SIZE];
};

// Поддерживаемые нашим устройством операции
static int device_open( struct inode *, struct file * );
static int device_release( struct inode *, struct file * );
static ssize_t device_read( struct file *, char *, size_t, loff_t * );
static ssize_t device_write( struct file *, const char *, size_t, loff_t * );

static ssize_t frame_read( struct file *, int *, int *, char *[], int * );
static ssize_t frame_write( struct file *, int , int , char [], int );

// Глобальные переменные, объявлены как static, воизбежание конфликтов имен.
static int major_number; /* Старший номер устройства нашего драйвера */
static int is_device_open = 0; /* Используется ли девайс ? */
//static char text[ 5 ] = "test\n"; /* Текст, который мы будет отдавать при обращении к нашему устройству */
//static char* text_ptr = text; /* Указатель на текущую позицию в тексте */

//Создадим кольцевой буфер для фреймов
struct Frame cycleFrameBuf[SIZE_BUF];
unsigned char frame_tail = 0;      //"указатель" хвоста буфера
unsigned char frame_head = 0;   //"указатель" головы буфера
unsigned char frame_count = 0;  //счетчик символов
//кольцевой (циклический) буфер
//unsigned char cycleBuf[SIZE_BUF];
//unsigned char tail = 0;      //"указатель" хвоста буфера
//unsigned char head = 0;   //"указатель" головы буфера
//unsigned char count = 0;  //счетчик символов

// Прописываем обработчики операций на устройством
static struct file_operations fops =
 {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release
  //.ioctl = device_ioctl
 };

//"очищаем" буфер
void FlushFrameBuf(void)
{
  frame_tail = 0;
  frame_head = 0;
  frame_count = 0;
}
//void FlushBuf(void)
//{
//  tail = 0;
//  head = 0;
//  count = 0;
//}

//положить символ в буфер
void PutFrame(struct Frame frame)
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
//void PutChar(unsigned char sym)
//{
//  if (count < SIZE_BUF){   //если в буфере еще есть место
//      cycleBuf[tail] = sym;    //помещаем в него символ
//      count++;                    //инкрементируем счетчик символов
//      tail++;                           //и индекс хвоста буфера
//      if (tail == SIZE_BUF) tail = 0;
//    }
//}

//взять символ из буфера
struct Frame GetFrame(void)
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
//unsigned char GetChar(void)
//{
//   unsigned char sym = 0;
//   if (count > 0){                            //если буфер не пустой
//      sym = cycleBuf[head];              //считываем символ из буфера
//      count--;                                   //уменьшаем счетчик символов
//      head++;                                  //инкрементируем индекс головы буфера
//      if (head == SIZE_BUF) head = 0;
//   }
//   return sym;
//}

// Функция загрузки модуля. Входная точка. Можем считать что это наш main()
static int __init test_init( void )
{
 printk( KERN_ALERT "TEST driver loaded!\n" );

 // Регистрируем устройсво и получаем старший номер устройства
 major_number = register_chrdev( 0, DEVICE_NAME, &fops );

 if ( major_number < 0 )
 {
  printk( "Registering the character device failed with %d\n", major_number );
  return major_number;
 }

 // Сообщаем присвоенный нам старший номер устройства
 printk( "Test module is loaded!\n" );

 printk( "Please, create a dev file with 'mknod /dev/test c %d 0'.\n", major_number );

 return SUCCESS;
}

// Функция выгрузки модуля
static void __exit test_exit( void )
{
 // Освобождаем устройство
 unregister_chrdev( major_number, DEVICE_NAME );

 printk( KERN_ALERT "Test module is unloaded!\n" );
}

// Указываем наши функции загрузки и выгрузки
module_init( test_init );
module_exit( test_exit );

static int device_open( struct inode *inode, struct file *file )
{
 //text_ptr = text;

 if ( is_device_open )
  return -EBUSY;

 is_device_open++;

 return SUCCESS;
}

static int device_release( struct inode *inode, struct file *file )
{
 is_device_open--;
 return SUCCESS;
}

// Функция для создания и записи фрейма
static ssize_t frame_write(struct file *filp, int transmitted_time, int transmitted_pid, char transmitted_mes[], int number_of_elements)
{
  if(number_of_elements > MESSAGE_SIZE){
    return -1;
  }
  else{
    int frame_write = 0;

    struct Frame frame;

    frame.time = transmitted_time;
    frame.pid = transmitted_pid;

    int i;
    for (i = 0; i < number_of_elements; i++){
      frame.message[i] = transmitted_mes[i];
    }

    PutFrame(frame);
    frame_write++;

    return frame_write;
  }
}

//static ssize_t device_write( struct file *filp, const char *buff, size_t len, loff_t * off )
//{
//  int byte_write = 0;

//  //if ( *text_ptr == 0 )//text_ptr = text[], тот массив. из которого мы читаем
//  //  return 0;

//  while ( len  )
//  {
//    PutChar(buff[byte_write]);
//    //i++;
//    //put_user( *( text_ptr++ ), buffer++ );
//    len--;
//    byte_write++;
//  }

//  return byte_write;
//}

// Эта функция получит ссылки на значения, в которые она запишет данные из фрейма
static ssize_t frame_read(struct file *filp, int *readed_time, int *readed_pid, char *readed_mes[], int *readed_num_of_el)
{
  int frame_read = 0;

  struct Frame frame;
  frame = GetFrame();

  (*readed_time) = frame.time;
  (*readed_pid) = frame.pid;

  ssize_t num_of_el = sizeof(frame.message)/sizeof(frame.message[0]);
  (*readed_num_of_el) = num_of_el;

  ssize_t i;
  for (i = 0; i < num_of_el; i++){
    (*readed_mes[i]) = frame.message[i];
  }

  return frame_read;
}
//static ssize_t device_read( struct file *filp, char *buffer, size_t length, loff_t * offset ){
//  int byte_read = 0;
//
//  //if ( *text_ptr == 0 )//text_ptr = text[], тот массив. из которого мы читаем
//  //  return 0;

//  while ( length  )
//  {
//    GetChar();
//    //put_user( *( text_ptr++ ), buffer++ );
//    length--;
//    byte_read++;
//  }

//  return byte_read;
//}
