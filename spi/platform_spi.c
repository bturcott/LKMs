#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/uaccess.h>

// Define information about this kernel module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott <bturcott@altera.com>");
MODULE_DESCRIPTION("Connects a SPI in the FPGA fabric to the HPS");
MODULE_VERSION("1.0");

// Prototypes for the platform bus
static int spi_probe(struct platform_device *pdev);
static int spi_remove(struct platform_device *pdev);

//Prototypes for the SPI functionality
static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset);
static ssize_t spi_write(struct file *file, const char *buffer, size_t len, loff_t *offset);

// An instance of this structure will be created for every custom_led IP in the system
struct custom_spi_dev {
    struct miscdevice miscdev;
    void __iomem *regs;
    u16 spi_value;
};


//Data structure that ties the driver to the platform bus
static struct platform_driver spi_driver = {
    .probe = spi_probe,
    .remove = spi_remove,
    .driver = {
        .name = "custom_spi",
        .owner = THIS_MODULE,
        //.of_match_table = custom_spi_dt_ids //Include if you want to pull from
    }
};

//Registers driver to the platform bus
static int __init spi_init(void){
	int ret_val = 0; //Return value used for the result of the platform_driver_register function
	pr_info("Initializing Custom SPI module\n"); //print command similar to printk()
	ret_val = platform_driver_register(&spi_driver); //Function call trying to register the spi_driver to the platform bus
	if(ret_val !=0) {
		pr_err("platform_driver_register returned %d\n", ret_val);
		return ret_val;
	}
	
	pr_info("Custom SPI module successfully initialized!\n");
	return 0;
}

//Unregisters driver from the platform bus
static void __exit spi_exit(void){

	platform_driver_unregister(&spi_driver);
	pr_info("Custom SPI module successfully removed\n");
}

// The file operations that can be performed on the custom_spi character file 
static const struct file_operations custom_spi_fops = {
    .owner = THIS_MODULE,
    .read = spi_read,
    .write = spi_write
};

// Called whenever the kernel finds a new device that our driver can handle
// (In our case, this should only get called for the one instantiation of the Custom SPI module)
static int spi_probe(struct platform_device *pdev)
{
    int ret_val = -EBUSY;
    struct custom_spi_dev *dev;
    struct resource *r = 0;
    
    pr_info("spi_probe enter\n");

    // Get the memory resources for the spi
    r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(r == NULL) {
        pr_err("IORESOURCE_MEM (register space) does not exist\n");
        goto bad_exit_return;
    }
    
    // Create structure to hold device-specific information (like the registers)
    dev = devm_kzalloc(&pdev->dev, sizeof(struct custom_spi_dev), GFP_KERNEL);

    // Both request and ioremap a memory region
    // This makes sure nobody else can grab this memory region
    // as well as moving it into our address space so we can actually use it
    dev->regs = devm_ioremap_resource(&pdev->dev, r);
    if(IS_ERR(dev->regs))
        goto bad_ioremap;

    // Initialize the misc device (this is used to create a character file in userspace)
    dev->miscdev.minor = MISC_DYNAMIC_MINOR;    // Dynamically choose a minor number
    dev->miscdev.name = "custom_spi";
    dev->miscdev.fops = &custom_spi_fops;

    ret_val = misc_register(&dev->miscdev);
    if(ret_val != 0) {
        pr_info("Couldn't register misc device :(");
        goto bad_exit_return;
    }

    // Give a pointer to the instance-specific data to the generic platform_device structure
    // so we can access this data later on (for instance, in the read and write functions)
    platform_set_drvdata(pdev, (void*)dev);

    pr_info("leds_probe exit\n");

    return 0;

bad_ioremap:
   ret_val = PTR_ERR(dev->regs); 
bad_exit_return:
    pr_info("spi_probe bad exit :(\n");
    return ret_val;
}

// This function gets called whenever a read operation occurs on one of the character files
static ssize_t spi_read(struct file *file, char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /* 
    * Get the custom_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our custom_leds_dev structure that has the current led value).
    * 
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct custom_spi_dev *dev = container_of(file->private_data, struct custom_spi_dev, miscdev);

    // Give the user the current led value
    success = copy_to_user(buffer, &dev->spi_value, sizeof(dev->spi_value));

    // If we failed to copy the value to userspace, display an error message
    if(success != 0) {
        pr_info("Failed to return current led value to userspace\n");
        return -EFAULT; // Bad address error value. It's likely that "buffer" doesn't point to a good address
    }

    return 0; // "0" indicates End of File, aka, it tells the user process to stop reading
}

// This function gets called whenever a write operation occurs on one of the character files
static ssize_t spi_write(struct file *file, const char *buffer, size_t len, loff_t *offset)
{
    int success = 0;

    /* 
    * Get the custom_leds_dev structure out of the miscdevice structure.
    *
    * Remember, the Misc subsystem has a default "open" function that will set
    * "file"s private data to the appropriate miscdevice structure. We then use the
    * container_of macro to get the structure that miscdevice is stored inside of (which
    * is our custom_leds_dev structure that has the current led value).
    * 
    * For more info on how container_of works, check out:
    * http://linuxwell.com/2012/11/10/magical-container_of-macro/
    */
    struct custom_spi_dev *dev = container_of(file->private_data, struct custom_spi_dev, miscdev);

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

// Gets called whenever a device this driver handles is removed.
// This will also get called for each device being handled when 
// our driver gets removed from the system (using the rmmod command).
static int spi_remove(struct platform_device *pdev)
{
    // Grab the instance-specific information out of the platform device
    struct custom_spi_dev *dev = (struct custom_spi_dev*)platform_get_drvdata(pdev);

    pr_info("spi_remove enter\n");

    // Unregister the character file (remove it from /dev)
    misc_deregister(&dev->miscdev);

    pr_info("spi_remove exit\n");

    return 0;
}

// Tell the kernel which functions are the initialization and exit functions
module_init(spi_init);
module_exit(spi_exit);


