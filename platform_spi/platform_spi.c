#include <linux/module.h>
#include <linux/platform_device.h> //Platform driver library
#include <linux/miscdevice.h> //Misc driver library - auto assigns major number 10
#include <linux/fs.h> //File system library 
#include <linux/clk.h> //needed for clk
#include <linux/uaccess.h>
#include <linux/io.h>

#define DRIVER_NAME "platform_spi"

//Function prototypes for write and read fops
static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t spi_write(struct file *file, const char *buffer, size_t len, loff_t *offset);


struct spi_dev{	
	struct miscdevice miscdev;
	struct clk *clk;
	void __iomem *regs; //__iomem is used by sparse to find possible coding faults
	u32 spi_value; //value to read/write into the avm
};
                                                                                                                                                 
/* File operations are defined in this section*/
/*------------------------------------------------------------------------------------*/
static const struct file_operations spi_fops = {
	.owner = THIS_MODULE,
	.read = spi_read,
	.write = spi_write
};
//Platform driver functions 
/*---------------------------------------------------------------------------*/
static int spi_probe(struct platform_device *pdev){
	
	struct spi_dev *dev;
	struct resource *r = 0;
	int ret = 0;
	
	pr_info("\n Probe function was called!");

    	// Get the memory resources for this LED device
    	//r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    	//if(r == NULL) {
        //	pr_err("IORESOURCE_MEM (register space) does not exist\n");
       	//	goto bad_exit_return;
   	//}

	dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
	if (dev = NULL)
		return -ENOMEM;
	
	pr_info("\n Memory was allocated \n");
	
	dev->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(dev->clk))
		return PTR_ERR(dev->clk);
	pr_info("\n Memory for clk was allocated \n");
	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	pr_info("\n Platform resources were obtained. \n");
	dev->regs = devm_ioremap_resource(&pdev->dev, r);
	pr_info("\n IORESOURCE was obtained and remapped");
    	if(IS_ERR(dev->regs))
        	goto bad_ioremap;
	
	dev->miscdev.minor = MISC_DYNAMIC_MINOR;
	dev->miscdev.name = "spi";
	dev->miscdev.fops = &spi_fops;	
	
	ret = misc_register(&dev->miscdev);
	if(ret !=0) {
		pr_info("\n Couldn't register misc device");
		goto bad_exit_return;
	}
	
	platform_set_drvdata(pdev, (void*)dev);
	pr_info("spi_probe exit\n");
	
	return 0;

bad_ioremap:
	ret = PTR_ERR(dev->regs);

bad_exit_return:
	pr_err("spi_probe bad exit\n");
	return ret;
}

static int spi_remove(struct platform_device *pdev){
	pr_info("\n Remove function was called!");
	
	struct spi_dev *dev = (struct spi_dev*)platform_get_drvdata(pdev);
	
	misc_deregister(&dev->miscdev);
	 
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
/*---------------------------------------------------------------------------*/
//Read Operation
static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;
    
    struct spi_dev *dev = container_of(file->private_data, struct spi_dev, miscdev);
    
    // Give the user the current led value
    success = copy_to_user(buffer, &dev->spi_value, sizeof(dev->spi_value));

    // If we failed to copy the value to userspace, display an error message
    if(success != 0) {
        pr_info("Failed to return current spi value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }

    return 0; // "0" indicates End of File, aka, it tells the user process to stop reading
}
//Write Operation
static ssize_t spi_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    struct spi_dev *dev = container_of(file->private_data, struct spi_dev, miscdev);

    // Get the new led value (this is just the first byte of the given data)
    success = copy_from_user(&dev->spi_value, buffer, sizeof(dev->spi_value));

    // If we failed to copy the value from userspace, display an error message
    if(success != 0) {
        pr_info("Failed to read led value from userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    } else {
        // We read the data correctly, so update the LEDs
        iowrite32(dev->spi_value, dev->regs);
    }

    return len; // Tell the user process that we wrote every byte they sent (even if we only wrote the first value, this will ensure they don't try to re-write their data)
}


//Basic functions for insmod and rmmod userspace calls
static int __init spi_init(void){
	pr_info("\n Welcome to the spi platform driver...\n");
	return platform_driver_register(&spi_driver);
}

static int __exit spi_exit(void){
	pr_info("\n Unloading spi platform driver...\n");
	platform_driver_unregister(&spi_driver);
}

//Mandatory function calls - must be included in every KLM
module_init(spi_init);
module_exit(spi_exit);

//Module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott <bturcott@altera.com>");
MODULE_DESCRIPTION("A component SPI driver on the platform bus.");
MODULE_VERSION("1.0");

