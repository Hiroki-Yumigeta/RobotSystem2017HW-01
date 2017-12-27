#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/io.h>

#define INPUT  0
#define OUTPUT 1

#define SWPIN  20
#define LEDPIN 21

MODULE_AUTHOR("Hiroki Yumigeta");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

static dev_t dev;
static struct cdev cdv;

static struct class *cls = NULL;

/* アドレスをマッピングするための配列 */
static volatile u32 *gpio_base = NULL;



static ssize_t sw_read(struct file* filp, const char*  buf, size_t count, loff_t* pos){
    char c;
    /* copy_from_user:ユーザーランドからの字の読み込みからカーネルにsizeof(char)個コピー */
    if(copy_from_user(&c, buf, sizeof(char)))
        return -EFAULT;

    /* GPFSET0のGPIO25に対応するところに1を書き込む -> OFF */
    /* GPFSET0のGPIO25に対応するところに0を書き込む -> ON */
    if(c == '0')
        gpio_base[13] = 1 << LEDPIN;
    else if(c == '1')
        gpio_base[7] = 1 << LEDPIN;

    printk(KERN_INFO "led_write is called\n");
    return 1;
}

static ssize_t led_write(struct file* filp, const char*  buf, size_t count, loff_t* pos){
    char c;
    /* copy_from_user:ユーザーランドからの字の読み込みからカーネルにsizeof(char)個コピー */
    if(copy_from_user(&c, buf, sizeof(char)))
        return -EFAULT;

    /* GPFSET0のGPIO25に対応するところに1を書き込む -> OFF */
    /* GPFSET0のGPIO25に対応するところに0を書き込む -> ON */
    if(c == '0')
        gpio_base[10] = 1 << LEDPIN;
    else if(c == '1')
        gpio_base[7] = 1 << LEDPIN;

    printk(KERN_INFO "led_write is called\n");
    return 1;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .write = led_write
};

static int __init init_mod(void){
    int retval;
    
    /* ioremap_nocache: GPIIOのレジスタを配列にマッピング */
    /* 0x3f200000: GPIOレジスタの最初のアドレス */
    /* 0xA0: 必要なアドレスの範囲 */
    gpio_base = ioremap_nocache(0x3f200000, 0xA0);
    
    /* GPIOピンを出力に設定 */
    /* GPIO25の機能->GPFSEL2 */
    const u32 led = LEDPIN;// GPIO21
    //  0b(00000000 00000000 00000000 00011001)

    const u32 index = led/10;// GPFSEL <index>
    // 2
    const u32 shift = (led%10)*3;// 15bit
    // 5*3=15

    const u32 mask = ~(0x7<<shift)*3;
    // ~((0b(00000000 00000000 00000000 00000111)<<15)*3)
    // ~(0b(00000000 00000011 10000000 00000000)*3)
    // ~0b(00000000 00001010 10000000 00000000)
    //  0b(11111111 11110101 01111111 11111111)

    /* GPFSEL2の17-15のビットを001に設定 */
    gpio_base[index] = (gpio_base[index] & mask) | (0x1 << shift);// 001
    //  base=0x(3f 20 00 02)
    //       0b(00111111 00100000 00000000 00000010)
    //  0b(00111111 00100000 10000000 00000010)

    retval = alloc_chrdev_region(&dev, 0, 1, "myled");
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
    /* クラスの作成 */
    cls = class_create(THIS_MODULE, "myled");
    if(IS_ERR(cls)){
        printk(KERN_ERR "class_create failed.");
        return PTR_ERR(cls);
    }

    // クラスへ情報を書き込み
    /* デバイス情報の作成 */
    /* device_create(クラス, NULL, デバイス, NULL, デバイス名, デバイスのマイナー番号) */
    device_create(cls, NULL, dev, NULL, "myled%d", MINOR(dev));
    return 0;
}

static void __exit cleanup_mod(void){
    cdev_del(&cdv);

    /* デバイス情報の削除 */
    device_destroy(cls, dev);

    /* クラスの削除 */
    class_destroy(cls);

    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
