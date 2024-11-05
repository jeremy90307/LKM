#include <linux/init.h> /* Needed for the macros */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

/* buffer for data */
static char buffer[255];
static size_t  buffer_pointer = 0; // calculate the data length

/* variables for device and device class */
static dev_t my_dev_num;
static struct class *my_class;
static struct cdev my_dev;

#define DRIVER_NAME "dummydriver"
#define DRIVER_CLASS "MyModuleClass"

/* read data out of the buffer */
static ssize_t driver_read(struct file *file, char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* get amount of data to cp*/
	to_copy = min(count, buffer_pointer);

	/* copy data to user*/
	not_copied = copy_to_user(usr_buffer, buffer, to_copy);

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

/* write data to buffer */
static ssize_t driver_write(struct file *file, const char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(buffer));

	/* copy data to user*/
	not_copied = copy_from_user(buffer, usr_buffer, to_copy);
	buffer_pointer = to_copy;

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

static int driver_open(struct inode *driver_file, struct file *instance){
	printk("dev_num - open was called!\n");
	return 0;
}

static int driver_close(struct inode *driver_file, struct file *instance){
	printk("dev_num - close was called!\n");
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
	printk("read_write - device number Major: %d, Minor: %d was registered!\n", my_dev_num >> 20, my_dev_num && 0xfffff);

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

	return 0; 
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
	cdev_del(&my_dev);
	device_destroy(my_class, my_dev_num);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_num, 1);
	printk("Goodbye, world\n"); 
} 

module_init(hello_init); 
module_exit(hello_exit); 

MODULE_LICENSE("GPL");