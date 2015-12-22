#include <linux/module.h> //Basic library for kernel modules
#include <linux/platform_device.h> //Platform driver library
#include <linux/fs.h> //File system library: needed for file ops
//#include <linux/miscdevice.h> //Misc driver library
#include <linux/uaccess.h> //Required for copy to user function
#include <linux/io.h> //needed for iowrite32 functionality

#define DRIVER_NAME "fpga_spi"
#define CLASS_NAME "spi"

static int majorNumber;
static struct class* fpgaspiClass = NULL;
static struct device* fpgaspiDevice = NULL;

struct fpga_spi 
{
	struct cdev		c_dev;
	void __iomem		*mmio_base;
	u32 			spi_value;
};

static int spi_open(struct inode *inodep, struct file *file){
	pr_info("%s has been opened",DRIVER_NAME);
}

static int spi_close(struct inode *inodep, struct file *file){
	pr_info("%s has been closed",DRIVER_NAME);
}

/*
static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
	struct fpga_spi *spi = container_of(file->private_data, struct fpga_spi, miscdev);
	int	missing = 0;
	
	pr_info("%s file: read()\n", DRIVER_NAME);
	pr_info("Buffer is: %d\n", sizeof(buffer));
	pr_info("Spi value is: %p\n",spi->mmio_base);
	pr_info("Size of is: %d\n", sizeof(spi->spi_value));
	
	missing = copy_to_user(buffer, spi->mmio_base, sizeof(spi->spi_value));
	
	if(missing != 0) {
        	pr_info("Failed to return current led value to userspace\n");
        	return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    	}

	return 0;
}



static ssize_t spi_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset)
{
	struct fpga_spi *spi = container_of(file->private_data, struct fpga_spi, miscdev);
	//struct fpga_spi *spi;
	int	missing;
	//spi = file->private_data;
	//spi->spi_value = devm_kmalloc(32, GFP_KERNEL); 
	
	pr_info("%s file: write()\n", DRIVER_NAME);
	pr_info("Buffer is: %d\n", sizeof(buffer));
	pr_info("Spi value is: %p\n",&spi->spi_value);
	pr_info("Size of is: %d\n", sizeof(spi->spi_value));
	missing = copy_from_user(spi->mmio_base, buffer, sizeof(spi->spi_value));
	
	//pr_info("Copy from user complete.\n");
	//iowrite32(spi->spi_value, spi->mmio_base);
	//pr_info("iowrite32 complete.\n");
	return len;
}
*/
//Data structure to define file operations
static const struct file_operations fops = {
	.owner = THIS_MODULE,
	//.read = spi_read,
	//.write = spi_write,
	.open = spi_open,
	.release = spi_close,
};


static int spi_probe(struct platform_device *pdev)
{
	struct fpga_spi *spi;
	struct resource *r = 0;
	int ret = -EBUSY;

	pr_info("Probe function has been called ");
	
	/*Platform_get_resource gets information from the device resource 
	 *either device tree or device module. Includes start/end address etc. 
	 *returns a pointer to struct resource*/
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	
	//Dynamically allocate a major number
	majorNumber = registerchrdev(0, DRIVER_NAME, &fops);
	if (majorNumber<0){
		pr_err("%s failed to register a major number\n",DRIVER_NAME);
		return majorNumber;
	}
	
	pr_info("%s registered correctly with major number %d\n",DRIVER_NAME,majorNumber);
	
	//Register the device class
	fpgaspiClass = class_create(THIS_MODULE,CLASS_NAME);
	if(IS_ERR(fpgaspiClass)){
		unregister_chrdev(majorNumber, DRIVER_NAME);
		pr_err("Failed to register device class\n");
		return PTR_ERR(fpgaspiClass);
	}
	pr_info("%s device class registered correctly\n",DRIVER_NAME);
	
	//Register the device driver
	fpgaspiDevice = device_create(fpgaspiClass, NULL, MKDEV(majorNumber, 0), NULL, DRIVER_NAME);
	if(IS_ERR(fpgaspiDevice)){
		class_destroy(fpgaspiClass);
		unregister_chrdev(majorNumber,DRIVER_NAME);
		pr_err("Failed to create the device\n");
		return PTR_ERR(fpgaspiDevice);
	}
	
	cdev_init(&spi->c_dev,&fops);
	if (cdev_add(&spi->c_dev, majorNumber,1) == -1){
		device_destroy(fpgaspiClass(c1, majorNumber);
		class_destroy(fpgaspiClass);
		unregister_chrdev(majorNumber,DRIVER_NAME);
		return -1;
	} 
	
	fpga_spi->mmio_base = ioremap(r->
	return 0;
}

static int spi_remove(struct platform_device *pdev)
{
	struct fpga_spi *spi;
	pr_info("\n Remove function has been called");
	spi = platform_get_drvdata(pdev);
	if (spi == NULL)
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
