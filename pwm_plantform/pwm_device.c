#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <asm/uaccess.h>

#include "fs4412_pwm.h"


#define EXYNOS4412_GPD0CON  	0x114000A0
#define EXYNOS4412_TIMER_BASE  	0x139D0000

#define TCFG0	0x00
#define TCFG1	0x04
#define TCON	0x08
#define TCNTB0	0x0C
#define TCMPB0	0x10



void pwm_release(struct device *dev){

}

static struct resource pwm_resources[]={
        [0]={
            .start=EXYNOS4412_GPD0CON,
            .flags=IORESOURSE_MEM,
        },
        [1]={
                .start=EXYNOS4412_TIMER_BASE+TCFG0,
                .flags=IORESOURSE_MEM,
        },
        [2]={
                .start=EXYNOS4412_TIMER_BASE+TCFG1,
                .flags=IORESOURSE_MEM,
        },
        [3]={
                .start=EXYNOS4412_TIMER_BASE+TCON,
                .flags=IORESOURSE_MEM,
        },
        [4]={
                .start=EXYNOS4412_TIMER_BASE+TCNTB0,
                .flags=IORESOURSE_MEM,
        },
        [5]={
                .start=EXYNOS4412_TIMER_BASE+TCMPB0,
                .flags=IORESOURSE_MEM,
        }
};

static struct platform_device pwm_device={
    .name="fs4412_pwm",
    .id=-1,
    .dev={
        .release = pwm_release,
    },
    .num_resources=6,
    .resource=pwm_resources,
};

static int fs4412_pwm_init(void)
{
    return platform_device_register(&pwm_device);
}

static void fs4412_pwm_exit(void)
{
    platform_device_unregister(&pwm_device);
}

module_init(fs4412_pwm_init);
module_exit(fs4412_pwm_exit);
MODULE_LICENSE("GPL");