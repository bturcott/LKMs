#include <linux/module.h> //Basic library for kernel modules
#include <linux/platform_device.h> //Platform driver library
#include <linux/fs.h> //File system library: needed for file ops
#include <linux/miscdevice.h> //Misc driver library
#include <linux/uaccess.h> //Required for copy to user function
#include <linux/io.h> //needed for iowrite32 functionality

#define DRIVER_NAME "fpga_spi"
#define CLASS_NAME "spi"
#define MEM_LENGHT 32

struct fpga_spi 
{
	struct miscdevice	miscdev;
	void __iomem		*mmio_base;
	u32 			spi_value;
};


static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
	struct fpga_spi *spi = container_of(file->private_data, struct fpga_spi, miscdev);
	int	missing = 0;
	
	
	pr_info("<%s> file: read()\n", DRIVER_NAME);
	
	missing = copy_to_user(buffer, &spi->spi_value, sizeof(spi->spi_value));
	
	if(missing != 0) {
        	pr_info("Failed to return current led value to userspace\n");
        	return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    	}
	
	pr_info("<%s> file: read()\n", DRIVER_NAME);
	return 0;
}



static ssize_t spi_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
	struct fpga_spi *spi = container_of(file->private_data, struct fpga_spi, miscdev);
	int	missing;
	
	pr_info("<%s> file: write()\n", DRIVER_NAME);
	
	missing = copy_from_user(&spi->spi_value, buffer, sizeof(spi->spi_value));
	
	pr_info("Copy from user complete.\n");
	//iowrite32(spi->spi_value, spi->mmio_base);
	//pr_info("iowrite32 complete.\n");
	return len;
}

//Data structure to define file operations
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = spi_read,
	.write = spi_write,
};


static int spi_probe(struct platform_device *pdev)
{
	struct fpga_spi *spi;
	struct resource *r;
	int ret = -EBUSY;

	pr_info("Probe function has been called ");
	//Allocates memory for spi device
	spi = devm_kzalloc(&pdev->dev, sizeof(*spi), GFP_KERNEL);
	if (spi == NULL)
		return -ENOMEM;
	
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	spi->mmio_base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(spi->mmio_base))
		return PTR_ERR(spi->mmio_base);

	spi->miscdev.minor = MISC_DYNAMIC_MINOR;
	spi->miscdev.name = DRIVER_NAME;
	spi->miscdev.fops = &fops;

	ret = misc_register(&spi->miscdev);
	if(ret != 0)
	{
		pr_info("Unable to register misc device.\n");
	}

	platform_set_drvdata(pdev, (void*)spi);
	
	pr_info("<%s> has been registered correctly.\n", DRIVER_NAME);
	/*
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
	*/
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

static const struct of_device_id fpga_spi_match[]=
{
	{.compatible = "altr,fpga_spi"},
	{/* End of table */}
};

MODULE_DEVICE_TABLE(of, fpga_spi_match);

//Defines the structure for the platform driver
static struct platform_driver spi_driver = {
	.probe = spi_probe,
	.remove = spi_remove,
	.driver = {
		.name = DRIVER_NAME,  
		.owner = THIS_MODULE,
		.of_match_table = fpga_spi_match,
	},
};

module_platform_driver(spi_driver);

//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("A component SPI device on the platform bus.");
MODULE_VERSION("1.0");
