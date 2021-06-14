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

MODULE_LICENSE("Dual BSD/GPL");

#define GPK1CON 0x11000C20

static int key_major = 240;
static int key_minor = 0;

struct key_device
{
	int keyval;
	struct cdev cdev;
	struct semaphore sem;
	wait_queue_head_t rq;
} key_device;

void *gpk1con;
struct class *cls;
struct device *dev;

static int fs4412_key_open(struct inode *inode, struct file *filp)
{
	struct key_device *dev = container_of(inode->i_cdev, struct key_device, cdev);

	filp->private_data = dev;
	printk("fs4412_key is opened\n");
	return 0;
}

static int fs4412_key_release(struct inode *inode, struct file *filp)
{
	printk("fs4412_key is closed\n");
	return 0;
}

static int fs4412_key_read(struct file *filp, char __user *buf, size_t size, loff_t *off)
{
	struct key_device *dev = filp->private_data;

	down(&dev->sem);
	while (dev->keyval == 0)
	{
		up(&dev->sem);
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		if (wait_event_interruptible(dev->rq, dev->keyval != 0))
			return -ERESTARTSYS;
		down(&dev->sem);
	}
	if (copy_to_user(buf, &dev->keyval, sizeof(int)))
	{
		up(&dev->sem);
		return -EFAULT;
	}
	dev->keyval = 0;
	up(&dev->sem);

	return sizeof(int);
}

static struct file_operations fs4412_key_ops = {
	.owner = THIS_MODULE,
	.open = fs4412_key_open,
	.release = fs4412_key_release,
	.read = fs4412_key_read,
};

static irqreturn_t handler(int irqno, void *dev_id)
{
	static long old_jiffies = 0;

	if (jiffies - old_jiffies < 20) return IRQ_HANDLED;

	old_jiffies = jiffies;
	printk("irq: interrupt %d\n", irqno);
	switch (irqno)
	{
	case IRQ_EINT(10) :
		key_device.keyval = 1;
		break;
	case IRQ_EINT(9) : //这里添加了音量减按键的中断
		key_device.keyval = 2;
		break;
	}
	wake_up(&key_device.rq);

	return IRQ_HANDLED;
}

static int key_request_irq(void)
{
	int result;

	result = request_irq(IRQ_EINT(10), handler, IRQF_DISABLED|IRQF_TRIGGER_FALLING, "key_1", NULL);
	if (result)	{
		printk("key: request irq %d failed!\n", IRQ_EINT(10));
		return result;
	}
	
	//这里请求音量减按键的中断
	result = request_irq(IRQ_EINT(9), handler, IRQF_DISABLED|IRQF_TRIGGER_FALLING, "key_2", NULL);
	if (result)	{
		printk("key: request irq %d failed!\n", IRQ_EINT(9));
		return result;
	}
	

	return 0;

}

static void init_key(void)
{
	gpk1con = ioremap(GPK1CON, 4);
	printk("%x\n",gpk1con);
	writel((readl(gpk1con) | 0xffffff), gpk1con);
}

static void key_free_irq(void)
{
	free_irq(IRQ_EINT(10), NULL);
	free_irq(IRQ_EINT(9), NULL);//这里释放音量减按键的中断
}

static int key_setup_cdev(struct cdev *cdev, struct file_operations *ops)
{
	int result;
	dev_t devno = MKDEV(key_major, key_minor);
	cdev_init(cdev, ops);
	cdev->owner = THIS_MODULE;
	result = cdev_add(cdev, devno, 1);
	if (result)
	{
		printk("key: fail to add cdev\n");
		return result;
	}

	cls = class_create(THIS_MODULE, "key");
	if(!cls){
		printk("class_create failed\n");
	}
	dev =  device_create(cls, NULL, devno, NULL, "key");
	if(!dev){
		printk("device_create failed\n");
		goto err1;
	}

	return 0;

err1:
	class_destroy(cls);
	return -1;
}

static int __init fs4412_read_key_init(void)
{
	int result;
	dev_t devno = MKDEV(key_major, key_minor);

	result = register_chrdev_region(devno, 1, "key");
	if (result)
	{
		printk("key: unable to get major %d\n", key_major);
		return result;
	}

	result = key_setup_cdev(&key_device.cdev, &fs4412_key_ops);
	if (result)
		goto _error1;

	result = key_request_irq();
	if (result)
		goto _error2;

	init_key();
	key_device.keyval = 0;
	//init_MUTEX(&key_device.sem);  //  can used in kernel which version is no more than 2.6.35
	sema_init(&key_device.sem, 1);
	init_waitqueue_head(&key_device.rq);
	printk("key : init_module\n");

	return 0;

_error2:
	cdev_del(&key_device.cdev);
_error1:
	unregister_chrdev_region(devno, 1);

	return result;
}

static void __exit fs4412_read_key_exit(void)
{
	dev_t devno = MKDEV(key_major, key_minor);
	key_free_irq();
	device_destroy(cls, devno);
	class_destroy(cls);
	cdev_del(&key_device.cdev);
	unregister_chrdev_region(devno, 1);
	printk("key: cleanup_module!\n");
}

module_init(fs4412_read_key_init);
module_exit(fs4412_read_key_exit);

