#include <linux/init.h>
#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/string.h>

/* global variables for sysfs folder and file */
static struct kobject *kobj_ref;

/* This function will be called when we read the sysfs file */
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer){
	return sprintf(buffer, "You have read from /sys/kernel/%s/%s\n", kobj->name, attr->attr.name);
}

/* This function will be called when we write the sysfs file */
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count){
	printk("sysfs_test - You wrote '%s' to /sys/kernel/%s/%s\n", buffer, kobj->name, attr->attr.name);
	return count;
}

static struct kobj_attribute sysfs_attr = __ATTR(sysfs_test, 0644, sysfs_show, sysfs_store);


static int __init ModuleInit(void) 
{ 
	/* creat /sys/kernel/test_dir/kobj_ref */
	printk("sysfs_test - Creating /sys/kernel/test_dir/kobj_ref\n");

	kobj_ref = kobject_create_and_add("test_dir", kernel_kobj);
	if(!kobj_ref){
		printk("sysfs_test - Error creating /sys/kernel/test_dir\n");
		return -ENOMEM;
	}

	/* Create the sysfs file kobj_ref */
	if(sysfs_create_file(kobj_ref, &sysfs_attr.attr)) {
		printk("sysfs_test - Error creating /sys/kernel/test_dir/kobj_ref\n");
		kobject_put(kobj_ref);
		return -ENOMEM;
	}
	return 0;
} 

static void __exit ModuleExit(void) 
{ 
	printk("sysfs_test - Deleting /sys/kernel/test_dir/kobj_ref\n");
	sysfs_remove_file(kobj_ref, &sysfs_attr.attr);
	kobject_put(kobj_ref);
}

module_init(ModuleInit); 
module_exit(ModuleExit);  

MODULE_LICENSE("GPL");