#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h> //Platform driver library

#define DRIVER_NAME "hello_world"

//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A simple hello world platform driver.");
MODULE_VERSION("1.0");

//Platform driver functions 
static int hello_probe(struct platform_device *pdev){
	printk(KERN_ALERT "\n Probe function was called!");
	return 0;
}

static int hello_remove(struct platform_device *pdev){
	printk(KERN_ALERT "\n Remove function was called!");
	return 0;
}

//Defines the structure for the platform driver
static struct platform_driver hello_driver = {
	.probe = hello_probe,
	.remove = hello_remove,
	.driver = {
		.name = DRIVER_NAME,  
		.owner = THIS_MODULE,
	},
};

//Basic functions for insmod and rmmod userspace calls
static int __init hello_init(void){
	printk(KERN_ALERT "\n Welcome to the hello world platform driver...\n");
	return platform_driver_register(&hello_driver);
}

static int __exit hello_exit(void){
	printk(KERN_ALERT "\n Unloading hello world platform driver...\n");
	platform_driver_unregister(&hello_driver);
}

//Mandatory function calls - must be included in every KLM
module_init(hello_init);
module_exit(hello_exit);
