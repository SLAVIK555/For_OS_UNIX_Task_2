#include <linux/module.h>
#include <linux/kernel.h>
MODULE_LICENSE("GPL");
int init_module(void){
printk(KERN_INFO "Hello world, it's init module\n");
return 0;
}
void cleanup_module(void){
printk(KERN_INFO "Goodbye world, it's cleanup module\n");
}
