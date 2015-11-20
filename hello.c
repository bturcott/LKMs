/*
 *@file hello.c
 *@author Brad Turcott
 *@date 11/20/2015
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A simple linux driver for the Atlas.");
MODULE_VERSION("0.1");

static char *name ="world";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log")

static int __init helloatlas_init(void){
	printk(KERN_INFO "EBB: Hello %s from the Atlas LKM!\n", name);
	return 0;
}

static void __exit helloatlas_exit(void){
	printk(KERN_INFO "EBB: Goodbye %s from the Atlas LKM!\n", name);
}

module_init(helloatlas_init);
module_exit(helloatlas_exit);
