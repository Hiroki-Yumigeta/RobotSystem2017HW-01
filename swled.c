#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/io.h>
#include <linux/delay.h>
/* define */
#define BASE_ADD      0x3f000000
#define BASE_ADD_GPIO (0x200000 + BASE_ADD)
#define INPUT  0
#define OUTPUT 1
#define PIN_SW  20
#define PIN_LED 21

MODULE_AUTHOR("Hiroki Yumigeta");
MODULE_DESCRIPTION("driver for LED control by Pushbutton Switch");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

/* global value */
static dev_t dev;
static volatile u32 *gpio_base = NULL;
static struct cdev cdv;
static struct class *cls = NULL;

static ssize_t sw_read(struct file* filp, char*  buf, size_t count, loff_t* pos){
    char rw_buf[4];
    int ret = 0;
    printk(KERN_INFO "sw_read is called\n");

    gpio_base[37] = 0x1 & 0x03;
    msleep(1);
    gpio_base[38] = 0x01<<PIN_SW;
    msleep(1);
    gpio_base[37] = 0;
    gpio_base[38] = 0;

    ret = ((gpio_base[13] & (0x01 << PIN_SW)) != 0);
    sprintf(rw_buf, "%d\n", ret);
    count = strlen(rw_buf);
    if(copy_to_user((void*)buf, &rw_buf, count) != 0){
        printk(KERN_ERR "read buffer error\n");
        printk(KERN_ERR "rw_buf: %s\n",rw_buf);
        printk(KERN_ERR "count: %d\n", count);
        return -EFAULT;
    }
    *pos += count;
    return count;
}
static ssize_t led_write(struct file* filp, const char*  buf, size_t count, loff_t* pos){
    char c;
    printk(KERN_INFO "led_write is called\n");

    if(copy_from_user(&c, buf, sizeof(char)))
        return -EFAULT;
    if(c == '0')
        gpio_base[10] = 1 << PIN_LED;
    else if(c == '1')
        gpio_base[7] = 1 << PIN_LED;
    return 1;
}
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .read  = sw_read,
    .write = led_write
};

/* function */
void init_sw(void){
    const u32 sw = PIN_SW;//GPIO20
    const u32 index = sw/10;
    const u32 shift = (sw%10)*3;
    const u32 mask = ~(0x7<<shift)*3;
    gpio_base[index] = (gpio_base[index] & mask) | (0x0 << shift);// 000
    /*
    gpio_base[37] = 0x1;
    msleep(1);
    gpio_base[38] = 0x01<<pin;
    msleep(1);
    gpio_base[37] = 0;
    gpio_base[38] = 0;
    */
}
void init_led(void){
    const u32 led = PIN_LED;// GPIO21
    const u32 index = led/10;
    const u32 shift = (led%10)*3;
    const u32 mask = ~(0x7<<shift)*3;
    gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);// 001
}

static int __init init_mod(void){
    int retval;
    /* I/O mapping */
    gpio_base = ioremap_nocache(BASE_ADD_GPIO, 0xA0);
    init_sw();
    init_led();
    retval = alloc_chrdev_region(&dev, 0, 1, "swled");
    if(retval<0){
        printk(KERN_ERR "alloc_chrdev_region failed.\n");
        return retval;
    }
    printk(KERN_INFO "%s is loaded. major:%d\n", __FILE__, MAJOR(dev));
    cdev_init(&cdv, &led_fops);
    retval = cdev_add(&cdv, dev, 1);
    if(retval < 0){
        printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
        return retval;
    }
    cls = class_create(THIS_MODULE, "swled");
    if(IS_ERR(cls)){
        printk(KERN_ERR "class_create failed.");
        return PTR_ERR(cls);
    }
    device_create(cls, NULL, dev, NULL, "swled%d", MINOR(dev));
    return 0;
}

static void __exit cleanup_mod(void){
    cdev_del(&cdv);
    device_destroy(cls, dev);
    class_destroy(cls);
    unregister_chrdev_region(dev, 1);
    printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
