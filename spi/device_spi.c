#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott <bturcott@altera.com>");
MODULE_DESCRIPTION("A SPI device for the platform bus");
MODULE_VERSION("1.0");

struct platform_device *pdev;
int inst_id = 1;

pdev = platform_device_alloc("custom_spi", inst_id);

struct	resource res = {
	.start = 0x0000000,
	.end = 0x10000000,
	.name = "spi"
	.flags = IORESOURCE_MEM,
	};

platform_device_add_resources(pdev, &res, 1);

static int __init add_spi(void){
	pr_info("SPI Device has been loaded");
	return 0;
}




module_init();
module_exit();
