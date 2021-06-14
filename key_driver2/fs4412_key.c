#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>

MODULE_AUTHOR("Bai&Su");
MODULE_DESCRIPTION("A Simple Key_driver with interrupt");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0");

#define GPX1CON 0x11000C20
#define KEY_NAME ("FS4412_key")
#define KEY_MAJOR 233
#define KEY_MINOR 0		
#define KEY_COUNT 1		

struct FS4412_key_struct
{
	int keyval;
	dev_t devno; 
	struct cdev Kdev; 
};
struct FS4412_key_struct FS4412_key;	// 声明对象

struct class *cls;
struct device *dev;
void *gpx1con;

/*****************************************************************************/
static int FS4412_key_open(struct inode *inode, struct file *filp)
{
	struct FS4412_key_struct *dev = container_of(inode->i_cdev, struct FS4412_key_struct, Kdev);
	filp->private_data = dev;
	printk("Succeed! key open\n");
	return 0;
}

static int FS4412_key_release(struct inode *inode, struct file *filp)
{
	printk("key release\n");
	return 0;
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = FS4412_key_open,
	.release = FS4412_key_release,
};

/****************************************************************/
static irqreturn_t key_interrupt(int irqno, void *devid)
{
	printk("irqno = %d\n", irqno);
	return IRQ_HANDLED;
}

static int FS4412_key_init(void)
{
	int ret;
	FS4412_key.devno = MKDEV(KEY_MAJOR, KEY_MINOR);
	ret = register_chrdev_region(FS4412_key.devno, KEY_COUNT, KEY_NAME);
	if (ret)
	{
		printk("Fail to allocate segdis device number\n");
		return ret;
	}

	cdev_init(&FS4412_key.Kdev, &fops);
	ret = cdev_add(&FS4412_key.Kdev, FS4412_key.devno, KEY_COUNT);
	if (ret)
	{
		unregister_chrdev_region(FS4412_key.devno, KEY_COUNT);
		printk("Fail to add segdis to system\n");
		return ret;
	}

    cls = class_create(THIS_MODULE, KEY_NAME);
	if (!cls)
	{
		printk("Fail to create class");
        return -1;
	}
	dev = device_create(cls, NULL, FS4412_key.devno, NULL, KEY_NAME);
	if (!dev)
	{
		printk("Fail to create device");
		class_destroy(cls);
		return -1;
	}

	ret = request_irq(IRQ_EINT(9), key_interrupt, IRQF_DISABLED|IRQF_TRIGGER_FALLING, "key_minus", NULL);
	if (ret < 0)
	{
		printk("Failed request irq: irqno = irq_res->start");
		return ret;
	}
	ret = request_irq(IRQ_EINT(10), key_interrupt, IRQF_DISABLED|IRQF_TRIGGER_FALLING, "key_max", NULL);
	if (ret < 0)
	{
		printk("Failed request irq: irqno = irq_res->start");
		return ret;
	}

    gpx1con = ioremap(GPX1CON, 4);
    writel((readl(gpx1con) | 0xffffff), gpx1con);

	printk("Insmod key Successfully!");
	return 0;

}

static int FS4412_key_exit(struct i2c_client *client)
{
	free_irq(IRQ_EINT(9), NULL);
	device_destroy(cls, FS4412_key.devno);
	class_destroy(cls);
	cdev_del(&FS4412_key.Kdev);
	unregister_chrdev_region(FS4412_key.devno, KEY_COUNT);
	printk("Remove Key Done\n");
	return 0;
}

module_init(FS4412_key_init);
module_exit(FS4412_key_exit);
