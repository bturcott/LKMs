#include <linux/module.h>
#include <linux/platform_device.h> //Platform driver library

#define DRIVER_NAME "fpga_spi"
#define RESOURCE1_START_ADDRESS 0xC0000000
#define RESOURCE1_END_ADDRESS 0xC000003F
#define DEVICE_IRQNM 0x0

//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A component SPI device on the platform bus.");
MODULE_VERSION("1.0");

static struct resource spi_resources[] = {
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

static struct platform_device spi_device = {
	.name = DRIVER_NAME,
	.id = -1,
	.num_resources = 1,
	.resource = spi_resources,
};

//Basic functions for insmod and rmmod userspace calls
static int __init spi_init(void){
	pr_info("\n Welcome to the spi platform device...\n");
	platform_device_register(&spi_device);
	return 0;
}

static int __exit spi_exit(void){
	pr_info("\n Unloading spi platform device...\n");
	platform_device_unregister(&spi_device);
	return 0;
}

//Mandatory function calls - must be included in every KLM
module_init(spi_init);
module_exit(spi_exit);
