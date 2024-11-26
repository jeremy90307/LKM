#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

static char text[255];

/* global variables for procfs folder and file */
static struct proc_dir_entry *proc_folder;
static struct proc_dir_entry *proc_file;

/* read data out of the buffer */
static ssize_t procfs_read(struct file *file, char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(text));

	/* copy data to user*/
	not_copied = copy_to_user(usr_buffer, text, to_copy);

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

/* write data to buffer */
static ssize_t procfs_write(struct file *file, const char *usr_buffer, size_t count, loff_t *offs){
	int to_copy, not_copied, delta;

	/* get amount of data to cp*/
	to_copy = min(count, sizeof(text));

	/* copy data to user*/
	not_copied = copy_from_user(text, usr_buffer, to_copy);
	printk("procfs_test - You have written %s to me\n", text);

	/* calcaulate data */
	delta = to_copy - not_copied;

	return delta;
}

static struct proc_ops fops = {
	.proc_read = procfs_read,
	.proc_write  = procfs_write,
};

static int __init ModuleInit(void) 
{ 
	/* creat /proc/test_folder/test_file */
	proc_folder = proc_mkdir("test_folder", NULL);
	if(proc_folder == NULL){
		printk("Error creating /proc/hello\n");
		return -ENOMEM;
	}

	proc_file = proc_create("test_file", 0666, proc_folder, &fops);
	if(proc_file == NULL) {
		printk("procfs_test - Error creating /proc/test_folder/test_file\n");
		proc_remove(proc_folder);
		return -ENOMEM;
	}

	printk("Created /proc/test_folder/test_file\n");
	return 0;
} 

static void __exit ModuleExit(void) 
{ 
	printk("Removing /proc/test_folder/test_file\n");
	proc_remove(proc_file);
	proc_remove(proc_folder);
} 

module_init(ModuleInit); 
module_exit(ModuleExit);  

MODULE_LICENSE("GPL");