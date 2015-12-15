#include <linux/module.h>
#include <linux/platform_device.h> //Platform driver library
#include <linux/fs.h> //File system library: needed for file ops
#include <linux/miscdevice.h> //Misc driver library
#include <linux/uaccess.h> //Required for copy to user function
#include <linux/io.h>


#define DRIVER_NAME "fpga_spi"
#define CLASS_NAME "spi"

static int majorNumber;
static struct class* fpga_spi_class = NULL;

//Function prototypes for write and read fops
//static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset);
//static ssize_t spi_write(struct file *file, const char *buffer, size_t len, loff_t *offset);

struct fpga_spi {
	struct device		*dev;
	void __iomem		*mmio_base;
	u32 			spi_value;
};

//Data structure to define file operations
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	//.read = spi_read,
	//.write = spi_write,
	//.open = spi_open,
	//.close = spi_close,
};


static int spi_probe(struct platform_device *pdev)
{
	struct fpga_spi *spi;
	struct resource *r;
	//int ret = 0;

	pr_info("\n Probe function has been called ");
	
	spi = devm_kzalloc(&pdev->dev, sizeof(*spi), GFP_KERNEL);
	if (spi == NULL)
		return -ENOMEM;
	
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi->mmio_base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(spi->mmio_base))
		return PTR_ERR(spi->mmio_base);

	platform_set_drvdata(pdev, spi);
	
	//Dynamically allocate a major number
	majorNumber = register_chrdev(0, DRIVER_NAME, &fops);
	if (majorNumber<0){
		pr_err("\n fpga_spi failed to register a major number \n");
	}
	pr_info("fpga_spi: registered correctly with major number %d\n", majorNumber);
	
	fpga_spi_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(fpga_spi_class)){
		unregister_chrdev(majorNumber, DRIVER_NAME);
		pr_err("Failed to register device class\n");
		return PTR_ERR(fpga_spi_class);
	}
	
	pr_info("fpga_spi: device class registered correctly\n");
	
	spi->dev = device_create(fpga_spi_class, NULL, MKDEV(majorNumber, 0), NULL, DRIVER_NAME);
	
	return 0;
}

static int spi_remove(struct platform_device *pdev)
{
	struct fpga_spi *hw;
	pr_info("\n Remove function has been called");
	hw = platform_get_drvdata(pdev);
	if (hw == NULL)
		return -ENODEV;
		
	return 0;
}

//Defines the structure for the platform driver
static struct platform_driver spi_driver = {
	.probe = spi_probe,
	.remove = spi_remove,
	.driver = {
		.name = DRIVER_NAME,  
		.owner = THIS_MODULE,
	},
};

module_platform_driver(spi_driver);

//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A component SPI device on the platform bus.");
MODULE_VERSION("1.0");
