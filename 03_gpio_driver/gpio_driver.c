#include <linux/init.h> /* Needed for the macros */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* variables for device and device class */
static dev_t my_dev_num;
static struct class *my_class;
static struct cdev my_dev;

#define DRIVER_NAME "LED_gpio_driver"
#define DRIVER_CLASS "MyModuleClass" // After class_create, the device exports to /sys/class/

#define GPIO_LED 575  // 575 corresponds to GPIO 4
#define GPIO_BUTTON 588  // 588 corresponds to GPIO 17

/* read data out of the buffer */
static ssize_t driver_read(struct file *file, char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;
	char tmp[3] = " \n";

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(tmp));

	/* Read value of button */
	printk("gpio 17 value: %d\n", gpio_get_value(GPIO_BUTTON));
	tmp[0] = gpio_get_value(GPIO_BUTTON) + '0'; // store to tmp[0]

	/* copy data to user*/
	not_copied = copy_to_user(usr_buffer, &tmp, to_copy);

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

/* write data to buffer */
static ssize_t driver_write(struct file *file, const char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;
	char value;

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(value));

	/* copy data from user*/
	not_copied = copy_from_user(&value, usr_buffer, to_copy);
	switch(value){
		case '0':
			gpio_set_value(GPIO_LED, 0);
			break;
		case '1':
			gpio_set_value(GPIO_LED, 1);
			break;
		default:
			printk("Invalid value!\n");
			break;
	}

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

static int driver_open(struct inode *driver_file, struct file *instance){
	printk("GPIO_Driver - open was called!\n");
	return 0;
}

static int driver_close(struct inode *driver_file, struct file *instance){
	printk("GPIO_Driver - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.read = driver_read,
	.write = driver_write
};

static int __init hello_init(void) 
{ 
	printk("Hello world!!!\n"); 

	/* allocate a dev num*/
	if(alloc_chrdev_region(&my_dev_num, 0, 1, DRIVER_NAME) < 0){
		printk("device number cant be allocated!\n");
		return -1;
	}
	printk("Device number Major: %d, Minor: %d was registered!\n", my_dev_num >> 20, my_dev_num && 0xfffff);

	/* create device class*/
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

	/* set gpio 4 init */
	if(gpio_request(GPIO_LED, "rpi-gpio-4")){
		printk("gpio 4 can't be allocated!\n");
		goto AddError;
	}

	/* set gpio 4 direction */
	if(gpio_direction_output(GPIO_LED, 0)){
		printk("can't set gpio 4 to output!\n");
		goto Gpio4Error;
	}

		/* set gpio 17 init */
	if(gpio_request(GPIO_BUTTON, "rpi-gpio-17")){
		printk("gpio 17 can't be allocated!\n");
		goto AddError;
	}

	/* set gpio 17 direction */
	if(gpio_direction_input(GPIO_BUTTON)){
		printk("cant set gpio 17 to input!\n");
		goto Gpio17Error;
	}

	return 0; 
Gpio17Error:
	gpio_free(GPIO_BUTTON);
Gpio4Error:
	gpio_free(GPIO_LED);
AddError:
	device_destroy(my_class, my_dev_num);
FileError:
	class_destroy(my_class);
ClassError:
	unregister_chrdev_region(my_dev_num, 1);
	return -1; 
} 

static void __exit hello_exit(void) 
{ 
	gpio_set_value(GPIO_LED, 0);
	gpio_free(GPIO_BUTTON);
	gpio_free(GPIO_LED);
	cdev_del(&my_dev);
	device_destroy(my_class, my_dev_num);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_num, 1);
	printk("Goodbye, world\n"); 
} 

module_init(hello_init); 
module_exit(hello_exit); 

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsai Zhong-Han");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");