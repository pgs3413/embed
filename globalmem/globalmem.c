#include <linux/module.h>  //module_init module_exit
#include <linux/types.h>  //dev_t
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
  
#define GLOBALMEM_SIZE  0x1000
#define MEM_SET_0 0x1
#define LED_MAGIC_1 'L'
#define MEM_SET_1  _IOW(LED_MAGIC_1, 0, int)
#define LED_MAGIC 'R'
#define MEM_CLEAR  _IOW(LED_MAGIC, 0, int)

#define GLOBALMEM_MAJOR 240     

static int globalmem_major = GLOBALMEM_MAJOR;  
 
struct globalmem_dev                                       
{                                                          
  struct cdev cdev;                          
  unsigned char mem[GLOBALMEM_SIZE];           
};  
  
struct globalmem_dev *globalmem_devp; 

int globalmem_open(struct inode *inode, struct file *filp)  
{  
 
  filp->private_data = globalmem_devp;  
  return 0;  
}  

int globalmem_release(struct inode *inode, struct file *filp)  
{  
  return 0;  
}  
  
 
static int globalmem_ioctl( struct file *filp, unsigned  
  int cmd, unsigned long arg)  
{  
  struct globalmem_dev *dev = filp->private_data;
  int i=0;

  printk(KERN_INFO "globalmem_ioctl is invoked\n");  
  switch (cmd)  
  {  
    case MEM_CLEAR:   		
      printk(KERN_INFO "MEM_CLEAR is invoked\n");  
      memset(dev->mem, 0, GLOBALMEM_SIZE);        
      printk(KERN_INFO "globalmem is set to zero\n");  
      break;  
	case MEM_SET_0:
      printk(KERN_INFO "MEM_SET_0 is invoked\n");  
	  for(i=0;i<5;i++) dev->mem[i]='0';
	  break;
	case MEM_SET_1:
	  for(i=0;i<5;i++) dev->mem[i]='1';
	  break;
    default:  
      return  - EINVAL;  
  }  
  return 0;  
}  
  

static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,  //
  loff_t *ppos)  
{  
  unsigned long p =  *ppos;  
  unsigned int count = size;  
  int ret = 0;  
  struct globalmem_dev *dev = filp->private_data;   
  
 
  if (p > GLOBALMEM_SIZE)  
    return count ?  - ENXIO: 0;   /* return ENXIO: No such device or address */  
  if (count > GLOBALMEM_SIZE - p)  
    count = GLOBALMEM_SIZE - p;  
  
  
  if (copy_to_user(buf, (void*)(dev->mem + p), count))  
  {  
    ret =  - EFAULT;   /* Bad address */  
  }  
  else  
  {  
    *ppos += count;  
    ret = count;  
      
  //  printk(KERN_INFO "read %d bytes(s) from position %d\n", count, p);  
  }  
  
  return ret;  
}  
  
 
static ssize_t globalmem_write(struct file *filp, const char __user *buf,  
  size_t size, loff_t *ppos)  
{  
  unsigned long p =  *ppos;  
  unsigned int count = size;  
  int ret = 0;  
  struct globalmem_dev *dev = filp->private_data; 
    
  
  if (p >= GLOBALMEM_SIZE)  
    return count ?  - ENXIO: 0; //ENXIO: No such device or addres  
  if (count > GLOBALMEM_SIZE - p)  
    count = GLOBALMEM_SIZE - p;  
      
  
  if (copy_from_user(dev->mem + p, buf, count))  
    ret =  - EFAULT;  
  else  
  {  
    *ppos += count;  
    ret = count;  
      
   printk(KERN_INFO "written %d bytes(s) to position %d\n", count, p);  
  }  
  
  return ret;  
}  
  
  
static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)  
{  
  loff_t ret = 0;  
  switch (orig)  
  {  
    case 0:     
      if (offset < 0)  
      {  
        ret =  - EINVAL;     /* Invalid argument */  
        break;  
      }  
      if ((unsigned int)offset > GLOBALMEM_SIZE)  
      {  
        ret =  - EINVAL;  
        break;  
      }  
      filp->f_pos = (unsigned int)offset;  
      ret = filp->f_pos;  
      break;  
    case 1:    
      if ((filp->f_pos + offset) > GLOBALMEM_SIZE)  
      {  
        ret =  - EINVAL;  
        break;  
      }  
      if ((filp->f_pos + offset) < 0)  
      {  
        ret =  - EINVAL;  
        break;  
      }  
      filp->f_pos += offset;  
      ret = filp->f_pos;  
      break;  
    default:  
      ret =  - EINVAL;  
      break;  
  }  
  return ret;  
}  
  

static const struct file_operations globalmem_fops =  //将各个函数挂载在file_operation结构体中
{  
  .owner = THIS_MODULE,  
  .llseek = globalmem_llseek,  
  .read = globalmem_read,  
  .write = globalmem_write,  
  .unlocked_ioctl = globalmem_ioctl,  
  .open = globalmem_open,  
  .release = globalmem_release,  
};  
  

static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)  
{  
  int err, devno = MKDEV(globalmem_major, index);  
  
  cdev_init(&dev->cdev, &globalmem_fops);  
  dev->cdev.owner = THIS_MODULE;  
  dev->cdev.ops = &globalmem_fops;  
  err = cdev_add(&dev->cdev, devno, 1);  
  if (err)  
    printk(KERN_NOTICE "Error %d adding LED%d", err, index);  
}  
  
 
int globalmem_init(void)  
{  
  int result;  
  dev_t devno = MKDEV(globalmem_major, 0);  // dev_t 设备号类型
  

  if (globalmem_major)  
    result = register_chrdev_region(devno, 1, "globalmem");  //向系统注册字符设备
	//register_chrdev（major,name,file_operation);
  else    
  {  
    result = alloc_chrdev_region(&devno, 0, 1, "globalmem");  
    globalmem_major = MAJOR(devno);  
  }    
  if (result < 0)  
    return result;  
      
   
  globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);  
  if (!globalmem_devp)    
  {  
    result =  - ENOMEM;  
    goto fail_malloc;  
  }  
  memset(globalmem_devp, 0, sizeof(struct globalmem_dev));  
    
  globalmem_setup_cdev(globalmem_devp, 0);  
  printk("globalmem driver installed!\n");  
  printk("globalmem_major is:%d\n",globalmem_major);  
  printk("the device name is %s\n", "globalmem");  
  
  return 0;  
  
  fail_malloc: unregister_chrdev_region(devno, 1);  
  return result;  
}  
  

void globalmem_exit(void)  
{  
  cdev_del(&globalmem_devp->cdev);   
  kfree(globalmem_devp);     
  unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); //注销字符设备
  printk("globalmem driver uninstalled!\n");  
}  
  
MODULE_AUTHOR("xxxx");  	//可不要
MODULE_LICENSE("Dual BSD/GPL");  //必须要加开源证书
    
  
module_init(globalmem_init);  //insmod 时执行globalmem_init
module_exit(globalmem_exit);  //rmmod 时执行globalmem_exit
