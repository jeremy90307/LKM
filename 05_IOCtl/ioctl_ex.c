#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

#include "ioctl_ex.h"

#define DRIVER_NAME "ioctl_ex"
#define DRIVER_CLASS "  ioctl_ex_class"

/* variables for device and device class */
static dev_t my_dev_num;
static struct class *my_class;
static struct cdev my_dev;

static int driver_open(struct inode *driver_file, struct file *instance){
	printk("ioctl_ex - open was called!\n");
	return 0;
}

static int driver_close(struct inode *driver_file, struct file *instance){
	printk("ioctl_ex - close was called!\n");
	return 0;
}

static long int my_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    switch(cmd){
        case WR_VALUE:
            if(copy_from_user(&value, (int32_t *)arg, sizeof(value))){
                printk("Data Write : Error\n");
            }
            printk("Value = %d", value);
            break;
        case RD_VALUE:
            if(copy_to_user((int32_t *)arg, &value, sizeof(value))){
                printk("Data Read : Error\n");
            }
            break;
        default:
            printk("Default\n");
            break;
    }
    return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
    .unlocked_ioctl = my_ioctl
};

static int __init ModuleInit(void) 
{ 
    /* allocate a device number */
	if(alloc_chrdev_region(&my_dev_num, 0, 1, DRIVER_NAME) < 0){
		printk("device number cant be allocated!\n");
		return -1;
	}
	printk("device number Major: %d, Minor: %d was registered!\n", my_dev_num >> 20, my_dev_num && 0xfffff);

	/* create device class */
	if((my_class = class_create(DRIVER_CLASS)) == NULL){
		printk("device class cant be created!\n");
		goto ClassError;
	}

	/* create device file */
	if(device_create(my_class, NULL, my_dev_num, NULL, DRIVER_NAME) == NULL){
		printk("cant create device file!\n");
		goto FileError;
	}

	/* initalize device file */
	cdev_init(&my_dev, &fops);

	/* register device to kernel */
	if(cdev_add(&my_dev, my_dev_num, 1) == -1){
		printk("register of device to kernel failed!\n");
		goto AddError;
	}
    printk("Device Driver Insert ... Done!\n");
	return 0;

AddError:
	device_destroy(my_class, my_dev_num);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_dev_num, 1);
	return -1; 
} 

static void __exit ModuleExit(void) 
{ 
    cdev_del(&my_dev);
	device_destroy(my_class, my_dev_num);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_num, 1);
	printk("Device Driver Remove ... Done!\n"); 
} 

module_init(ModuleInit); 
module_exit(ModuleExit);  

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsai Zhong-Han");
MODULE_DESCRIPTION("Practice for ioctl");