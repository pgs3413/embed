#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/slab.h>

#define major 500

struct cdev *cd;

int i=1;

int exp_ioctl(struct file *filp,unsigned int cmd,unsigned long arg){
    int y=1;
    i++;
    printk("invoke ioctl:%d &i=%x &y=%x i=%d\n",cmd,&i,&y,i);
    return 0;
}

static const struct file_operations fop={
    .owner=THIS_MODULE,
    .unlocked_ioctl=exp_ioctl
};

int exp_init(void){
    printk("exp init\n");
    dev_t devno = MKDEV(major,0);
    int result=register_chrdev_region(devno,1,"exp");
    cd=kmalloc(sizeof(struct cdev),GFP_KERNEL);
    cdev_init(cd,&fop);
    cd->owner=THIS_MODULE;
    cdev_add(cd,devno,1);
    printk("exp init\n");
    return result;
}

void exp_exit(void){
    cdev_del(cd);
    kfree(cd);
    unregister_chrdev_region(MKDEV(major,0),1);
    printk("exp exit\n");
}

MODULE_LICENSE("Dual BSD/GPL");

module_init(exp_init);
module_exit(exp_exit);