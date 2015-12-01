#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h> //Platform driver library

#define DRIVER_NAME "platform_spi"
#define RESOURCE1_START_ADDRESS 0x50000000
#define RESOURCE1_END_ADDRESS 0x50001FFF
#define DEVICE_IRQNM 0x0


//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A component SPI device on the platform bus.");
MODULE_VERSION("1.0");

static struct resource hello_resources[] = {
	{
		.start = RESOURCE1_START_ADDRESS,
		.end = RESOURCE1_END_ADDRESS,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = DEVICE_IRQNM,
		.end = DEVICE_IRQNM,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device hello_device = {
	.name = DRIVER_NAME,
	.id = -1,
	.num_resources = 1,
	.resource = hello_resources,
};

//Basic functions for insmod and rmmod userspace calls
static int __init hello_init(void){
	printk(KERN_ALERT "\n Welcome to the hello world platform device...\n");
	platform_device_register(&hello_device);
	return 0;
}

static int __exit hello_exit(void){
	printk(KERN_ALERT "\n Unloading hello world platform device...\n");
	platform_device_unregister(&hello_device);
}

//Mandatory function calls - must be included in every KLM
module_init(hello_init);
module_exit(hello_exit);
