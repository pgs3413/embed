#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "fs4412_pwm.h"

#define PWM_MAJOR 501
#define PWM_MINOR 0
#define PWM_NUM	  1

struct pwm_device
{
    struct cdev cdev;
    void __iomem *gpd0con;
    void __iomem *tcfg0;
    void __iomem *tcfg1;
    void __iomem *tcon;
    void __iomem *tcntb0;
    void __iomem *tcmpb0;
};

static struct pwm_device *pwm_dev;
struct semaphore sem;

static int fs4412_pwm_open(struct inode *inode, struct file *file)
{
    writel((readl(pwm_dev->gpd0con) & ~0xf) | 0x2,pwm_dev->gpd0con);
    writel((readl(pwm_dev->tcfg0) & ~0xff) | 0xf9,pwm_dev->tcfg0);
    writel(readl(pwm_dev->tcfg1) & ~ 0xf | 0x2 ,pwm_dev->tcfg1);
    writel(100, pwm_dev->tcntb0);
    writel(90, pwm_dev->tcmpb0);
    writel((readl(pwm_dev->tcon) & ~0xff) | 0x7 ,pwm_dev->tcon);
    return 0;
}

static int fs4412_pwm_release(struct inode *inode, struct file *file)
{
    return 0;
}
#define FRE 25000
static long fs4412_pwm_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int nr;

    down(&sem);

    if(copy_from_user((void *)&nr, (void *)arg, sizeof(nr)))
        return -EFAULT;

    switch(cmd) {
        case PWM_ON:
            writel((readl(pwm_dev->tcon) & ~0xf) | 0x9 ,pwm_dev->tcon);
            break;
        case PWM_OFF:
            writel(readl(pwm_dev->tcon) & ~0xf ,pwm_dev->tcon);
            break;
        case SET_PRE:
            break;
        case SET_CNT:
            if(nr==0){
                writel((readl(pwm_dev->gpd0con) & ~0xf) | 0x4,pwm_dev->gpd0con);
            }else if(nr==1){
                writel((readl(pwm_dev->gpd0con) & ~0xf) | 0x2,pwm_dev->gpd0con);
                writel((readl(pwm_dev->tcon) & ~0xf) | 0x9 ,pwm_dev->tcon);
                writel(FRE/1000, pwm_dev->tcntb0);
                writel(FRE/2000, pwm_dev->tcmpb0);
            }else if(nr==2){
                writel((readl(pwm_dev->gpd0con) & ~0xf) | 0x2,pwm_dev->gpd0con);
                writel((readl(pwm_dev->tcon) & ~0xf) | 0x9 ,pwm_dev->tcon);
                writel(FRE/200, pwm_dev->tcntb0);
                writel(FRE/400, pwm_dev->tcmpb0);
            }
            break;
        default:
            printk("Invalid argument\n");
            return -EINVAL;
    }

    up(&sem);

    return 0;
}

static struct file_operations fs4412_pwm_fops = {
        .owner = THIS_MODULE,
        .open = fs4412_pwm_open,
        .release =fs4412_pwm_release,
        .unlocked_ioctl = fs4412_pwm_unlocked_ioctl,
};

static int pwm_probe(struct platform_device *dev){

    struct resource *resources[6];
    for(int i=0;i<6;i++){
        resources[i]=plantform_get_resourse(dev,IORESOURSE_MEM,i);
    }

    struct class *cls;
    int ret;
    dev_t devno = MKDEV(PWM_MAJOR, PWM_MINOR);

    printk("pwm init\n");

    ret = register_chrdev_region(devno, PWM_NUM, "pwm");
    if (ret < 0) {
        printk("failed to register_chrdev_region\n");
        return ret;
    }

    pwm_dev = kzalloc(sizeof(*pwm_dev), GFP_KERNEL);
    if (pwm_dev == NULL) {
        ret = -ENOMEM;
        printk("failed to kzalloc\n");
        goto err1;
    }

    cdev_init(&pwm_dev->cdev, &fs4412_pwm_fops);
    pwm_dev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&pwm_dev->cdev, devno, PWM_NUM);
    if (ret < 0) {
        printk("failed to cdev_add\n");
        goto err2;
    }

    pwm_dev->gpd0con = ioremap(resources[0], 4);
    pwm_dev->tcfg0 = ioremap(resources[1], 4);
    pwm_dev->tcfg1 = ioremap(resources[2], 4);
    pwm_dev->tcon = ioremap(resources[3], 4);
    pwm_dev->tcntb0 = ioremap(resources[4], 4);
    pwm_dev->tcmpb0 = ioremap(resources[5], 4);


    cls = class_create(THIS_MODULE, "pwm");
    device_create(cls, NULL, devno, NULL, "pwm");

    sema_init(&sem, 1);

    return 0;

    err3:
    cdev_del(&pwm_dev->cdev);
    err2:
    kfree(pwm_dev);
    err1:
    unregister_chrdev_region(devno, PWM_NUM);
    return ret;
}

static int pwm_remove(struct patform_device *dev){

}

static struct platform_driver pwm_driver={
    .driver={
      .name="fs4412_pwm",
    },
    .probe=pwm_probe,
    .remove=pwm_remove,
};

static int fs4412_pwm_init(void)
{
    return platform_driver_register(&pwm_driver);
}

static void fs4412_pwm_exit(void)
{
    platform_driver_unregister(&pwm_driver);
}

module_init(fs4412_pwm_init);
module_exit(fs4412_pwm_exit);
MODULE_LICENSE("GPL");