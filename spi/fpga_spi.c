#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <asm/uaccess.h>          // Required for the copy to user function
#define  DEVICE_NAME "spifpga"    ///< The device will appear at /dev/ebbchar using this value
#define  CLASS_NAME  "spi"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Brad Turcott");
MODULE_DESCRIPTION("An FPGA SPI driver");
MODULE_VERSION("1.0");

//Declaration for device, class and major number
static int majorNumber;
static struct class* spifpgaClass = NULL;
static struct device* spifpgaDevice = NULL;

//Function prototypes for file operations
static int spi_open(struct inode *, struct file *);
static int spi_release(struct inode *, struct file *);
//static int spi_write();
//static int spi_read();
//static int spi_ioctl();

 
//SPI file operations
static struct file_operations fops = 
{
	.open = spi_open,
	.release = spi_release,
	//.read = spi_read,
	//.write = spi_write,
	//.ioctl = spi_ioctl,
	
};

static int spi_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Spifpga has been opened.\n");
   return 0;
}


static int spi_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "Spifpga has been closed.\n");
   return 0;
}

static int __init spifpga_init(void){
   printk(KERN_INFO "Initializing the Spifpga LKM\n");
 
   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "spifpga failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Registered correctly with major number %d\n", majorNumber);
 
   // Register the device class
   spifpgaClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(spifpgaClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(spifpgaClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "Device class registered correctly\n");
 
   // Register the device driver
   spifpgaDevice = device_create(spifpgaClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(spifpgaDevice)){               // Clean up if there is an error
      class_destroy(spifpgaClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(spifpgaDevice);
   }
   printk(KERN_INFO "Device class created correctly\n"); // Made it! device was initialized
   return 0;
}

static void __exit spifpga_exit(void){
   device_destroy(spifpgaClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(spifpgaClass);                          // unregister the device class
   class_destroy(spifpgaClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "Spifpga removed\n");
}

module_init(spifpga_init);
module_exit(spifpga_exit);


