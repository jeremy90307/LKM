#include <linux/init.h> /* Needed for the macros */ 
#include <linux/module.h> /* Needed by all modules */ 
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

/* variables for device and device class */
static dev_t my_dev_num;
static struct class *my_class;
static struct cdev my_dev;

#define DRIVER_NAME "lcd"
#define DRIVER_CLASS "MyModuleClass" // After class_create, the device exports to /sys/class/

#define ENABLE_PIN 573  // corresponds to GPIO 2
#define REGISTER_SELECT_PIN 574  // corresponds to GPIO 3
#define LCD_DATA_PIN_0 575  // corresponds to GPIO 4
#define LCD_DATA_PIN_1 588  // corresponds to GPIO 17
#define LCD_DATA_PIN_2 598	// corresponds to GPIO 27
#define LCD_DATA_PIN_3 593	// corresponds to GPIO 22
#define LCD_DATA_PIN_4 581	// corresponds to GPIO 10
#define LCD_DATA_PIN_5 580	// corresponds to GPIO 9
#define LCD_DATA_PIN_6 582	// corresponds to GPIO 11
#define LCD_DATA_PIN_7 576	// corresponds to GPIO 5

/*lcd char buffer */
static char lcd_buffer[17];

unsigned int gpios[] = {
	ENABLE_PIN,
	REGISTER_SELECT_PIN,
	LCD_DATA_PIN_0,
	LCD_DATA_PIN_1,
	LCD_DATA_PIN_2,
	LCD_DATA_PIN_3,
	LCD_DATA_PIN_4,
	LCD_DATA_PIN_5,
	LCD_DATA_PIN_6,
	LCD_DATA_PIN_7
};
#define ENABLE gpios[0]
#define REGISTER_SELECT gpios[1]

/* generate a pulse on the enable signal */
void lcd_enable(void){
	gpio_set_value(gpios[0], 1);	// gpiosp[0] is the enable pin
	msleep(20);
	gpio_set_value(gpios[0], 0);	// gpiosp[0] is the enable pin
}

/* set 8 bit data bus */
void lcd_send_byte(char data){
	int i;
	for(i = 0; i < 8; i++){
		/* gpios[2] to gpios[9] are the data pins */
		gpio_set_value(gpios[i+2], ((data) & (1<<i)) >> i);
	}
	lcd_enable();
	msleep(20);
}

/* send command to the lcd  */
void lcd_command(uint8_t data){
	gpio_set_value(REGISTER_SELECT, 0); // RS to instruction
	lcd_send_byte(data);
}

void lcd_data(uint8_t data){
	gpio_set_value(REGISTER_SELECT, 1); // RS to data
	lcd_send_byte(data);
}


/* write data to buffer */
static ssize_t driver_write(struct file *file, const char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(lcd_buffer));

	/* copy data from user*/
	not_copied = copy_from_user(lcd_buffer, usr_buffer, to_copy);

	/* calcaulate data */
	delta = to_copy - not_copied;

	/* set the new data to the display */
	lcd_command(0x1); // clear display

	for(int i = 0; i < to_copy; i++){
		lcd_data(lcd_buffer[i]);
	}
	return delta;
}

static int driver_open(struct inode *driver_file, struct file *instance){
	printk("LCD_Driver - open was called!\n");
	return 0;
}

static int driver_close(struct inode *driver_file, struct file *instance){
	printk("LCD_Driver - close was called!\n");
	return 0;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_close,
	.write = driver_write
};

static int __init ModuleInit(void) 
{ 
	printk("Hello kernel!!!\n"); 
	char *names[] = {"ENABLE_PIN", "REGISTER_SELECT", "DATA_PIN0", 
					"DATA_PIN1", "DATA_PIN2", "DATA_PIN3", "DATA_PIN4", 
					"DATA_PIN5", "DATA_PIN6", "DATA_PIN7"};
	int i;

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

	/* initialize GPIOs */
	printk("initializing GPIOs\n");
	for(i = 0; i < 10; i++){
		if(gpio_request(gpios[i], names[i])){
			printk("gpio %d can't be allocated!\n", gpios[i]);
			goto GPIOInitError;
		}
	}

	printk("setting GPIOs to output\n");
	for(i = 0; i < 10; i++){
		if(gpio_direction_output(gpios[i], 0)){
			printk("gpio %d can't be set to output!\n", gpios[i]);
			goto GPIODirectionError;
		}
	}

	/* init the display */
	lcd_command(0x30); // 8 bit mode
	lcd_command(0xf); // display on, cursor on, blinking on
	lcd_command(0x1); // clear display 
	char text[] = "Hello!";
	for(i = 0; i < sizeof(text)-1; i++){
		lcd_data(text[i]);
	}

	return 0; 

GPIODirectionError:
	i = 9; // set i to 9 to free all the GPIOs
GPIOInitError:
	for(;i>=0; i--)
		gpio_free(gpios[i]);
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
	lcd_command(0x1); // clear display
	for(int i = 0; i < 10; i++){
		gpio_set_value(gpios[i], 0);
		gpio_free(gpios[i]);
	}
	cdev_del(&my_dev);
	device_destroy(my_class, my_dev_num);
	class_destroy(my_class);
	unregister_chrdev_region(my_dev_num, 1);
	printk("Goodbye, kernel\n"); 
} 

module_init(ModuleInit); 
module_exit(ModuleExit); 

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tsai Zhong-Han");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED and reading a button");