#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/err.h>
#include <linux/gpio.h> 
#include <linux/hrtimer.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/interrupt.h>

//#include <asm/irq.h>
//#include <asm/io.h>

// To find the correct gpio label, check /sys/kernel/debug/gpio
#define GPIO_21 (533) // Clock pin
#define GPIO_20 (532) // Latch pin
#define GPIO_16 (528) // Data pin
#define BUTTON_IRQ (538) // Interrput_pin

static short int button_irq = 0;
static struct input_dev *button_dev;
int size_buttons = 9;
int data_sent_counter = 0;
dev_t dev = 0;
bool clk_state = false;
bool latch_state = false;
bool data[9] = {0};
u64 start_t;
u64 start2_t;
static struct cdev snes_cdev;
static struct class *dev_class;
static struct hrtimer my_hrtimer;
static struct hrtimer my_hrtimer2;
static ssize_t snes_write(struct file *filp, const char *buf, size_t len, loff_t * off);


static struct file_operations fops =
{
  .owner          = THIS_MODULE,
//  .read           = etx_read,
  .write          = snes_write,
//  .open           = etx_open,
//  .release        = etx_release,
};

static int __init snes_controller_init(void);
static void __exit snes_controller_exit(void);

static irqreturn_t button_interrupt(int irq, void *dummy)
{
        printk("button_isr !!!!\n");
        input_report_key(button_dev, BTN_0, 1);
        input_sync(button_dev);
        return IRQ_HANDLED;
}

static ssize_t snes_write(struct file *filp, const char *buf, size_t len, loff_t * off)
{
  uint8_t rec_buf[10] = {0};
  
  if( copy_from_user( rec_buf, buf, len ) > 0) {
    pr_err("ERROR: Not all the bytes have been copied from user\n");
  }
  
  pr_info("Write Function : GPIO_21 Set = %c\n", rec_buf[0]);
  
  if (rec_buf[0]=='1') {
    //set the GPIO value to HIGH
    gpio_set_value(GPIO_21, 1);
  } else if (rec_buf[0]=='0') {
    //set the GPIO value to LOW
    gpio_set_value(GPIO_21, 0);
  } else {
    pr_err("Unknown command : Please provide either 1 or 0 \n");
  }
  
  return len;
  
}

static enum hrtimer_restart test_hrtimer_handler(struct hrtimer *timer) {

  u64 now_t = jiffies;
  //printk("start_t - now_t = %ums\n", jiffies_to_msecs(now_t - start_t));
  hrtimer_forward(timer,hrtimer_cb_get_time(timer),ktime_set(0,ms_to_ktime(1000)));
  gpio_set_value(GPIO_21, clk_state);
  if(clk_state){
    mdelay(100);  
    if(data_sent_counter < size_buttons){
      int rin = gpio_get_value(GPIO_16);
      pr_info("Read value[%d]: %d\n", data_sent_counter, rin);
      data[data_sent_counter] = rin;
      pr_info("data[%d] = %d\n", data_sent_counter, data[data_sent_counter]);
      data_sent_counter++;    
    }
    else{
      pr_info("Data received: %d %d %d %d %d %d %d %d %d\n", data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]);
      input_report_key(button_dev, BTN_0, 1);
      input_sync(button_dev);

    }     
  }
  clk_state = !clk_state;
  return HRTIMER_RESTART;

}

static enum hrtimer_restart test2_hrtimer_handler(struct hrtimer *timer) {
  //u64 now_t = jiffies;
  //printk("start_t - now_t = %um s(2)\n", jiffies_to_msecs(now_t - start2_t));
  pr_info("Activate latch pulse\n");
  hrtimer_forward(timer,hrtimer_cb_get_time(timer),ktime_set(0,ms_to_ktime(20000)));
  gpio_set_value(GPIO_20, 1);
  mdelay(200);
  gpio_set_value(GPIO_20, 0);
  //if(latch_state) {
    data_sent_counter = 0;
  //}
  //latch_state = !latch_state;
  return HRTIMER_RESTART;
}
static int __init snes_controller_init(void)
{
  if((alloc_chrdev_region(&dev, 0, 1, "snes_Dev")) < 0 )
  {
    pr_err("Unable to allocate major number\n");
    unregister_chrdev_region(dev,1);
    return -1;
  }
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
  
  cdev_init(&snes_cdev, &fops);

  if((cdev_add(&snes_cdev, dev, 1)) < 0)
  {
    pr_err("Unable to add device to the system\n");
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1;
  }  

  dev_class = class_create("snes_class"); 
  if(IS_ERR(dev_class))
  {
    pr_err("Unable to create struct class\n");
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);

    return -1;
  }
  
  if(IS_ERR(device_create(dev_class, NULL, dev, NULL, "snes_device")))
  {
    pr_err("Unable to create device\n");
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1;
  }

  if(gpio_request(GPIO_21, "LED_LOL") < 0)
  {
    pr_err("ERROR: GPIO %d request\n", GPIO_21);
    gpio_free(GPIO_21);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1;
  }

  if(gpio_request(GPIO_20, "SNES_LATCH") < 0)
  {
    pr_err("ERROR: GPIO %d request\n", GPIO_20);
    gpio_free(GPIO_20);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1;

  }
  
  if(gpio_request(GPIO_16, "SNES_DATA") < 0)
  {
    pr_err("ERROR: GPIO %d request\n", GPIO_16);
    gpio_free(GPIO_16);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1;
  }
  
  if(gpio_request(BUTTON_IRQ, "irq_button")  < 0)
  {
    pr_err("ERROR: GPIO %d request\n", BUTTON_IRQ);
    gpio_free(BUTTON_IRQ);
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&snes_cdev);
    unregister_chrdev_region(dev,1);
    return -1; 
  }
  

  gpio_direction_output(GPIO_21, 0);
  gpio_direction_output(GPIO_20, 0);
  gpio_direction_input(GPIO_16);
  gpio_direction_input(BUTTON_IRQ);
  
  
  gpiod_export(gpio_to_desc(GPIO_16), false);
  gpiod_export(gpio_to_desc(GPIO_21), false);
  gpiod_export(gpio_to_desc(GPIO_20), false);
  gpiod_export(gpio_to_desc(BUTTON_IRQ), false);

  pr_info("Device Driver Initialized\n");

  hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  my_hrtimer.function = &test_hrtimer_handler;
  start_t = jiffies;
  hrtimer_start(&my_hrtimer, ms_to_ktime(100), HRTIMER_MODE_REL);
 
  hrtimer_init(&my_hrtimer2, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
  my_hrtimer2.function = &test2_hrtimer_handler;
  start2_t = jiffies;
  hrtimer_start(&my_hrtimer2, ms_to_ktime(50), HRTIMER_MODE_REL); 


  //if (request_irq(BUTTON_IRQ, button_interrupt, 0, "button", NULL)) {
  //  printk(KERN_ERR "button.c: Can't allocate irq %d\n", button_irq);
  //return -EBUSY;
  //}
  
  button_dev = input_allocate_device();
  if (!button_dev) {
    printk(KERN_ERR "button.c: Not enough memory\n");
    free_irq(BUTTON_IRQ, button_interrupt);
    return -ENOMEM;
  }
 
  button_dev->evbit[0] = BIT_MASK(EV_KEY);
  button_dev->keybit[BIT_WORD(BTN_0)] = BIT_MASK(BTN_0); 

   
  if (input_register_device(button_dev)) {
    printk(KERN_ERR "button.c: Failed to register device\n");
    input_free_device(button_dev);
    free_irq(BUTTON_IRQ, button_interrupt);
    return -1;
  }
   
  pr_info("Timer Initialized");
  return 0;
}

static void __exit snes_controller_exit(void)
{
  gpiod_unexport(gpio_to_desc(GPIO_21));
  gpiod_unexport(gpio_to_desc(GPIO_20));
  gpiod_unexport(gpio_to_desc(GPIO_16));
  gpio_free(GPIO_21);
  gpio_free(GPIO_20);
  gpio_free(GPIO_16);
  device_destroy(dev_class, dev);
  class_destroy(dev_class);
  cdev_del(&snes_cdev);
  unregister_chrdev_region(dev, 1);
  hrtimer_cancel(&my_hrtimer);
  hrtimer_cancel(&my_hrtimer2);

  input_unregister_device(button_dev);  
//  free_irq(BUTTON_IRQ, button_interrupt);

  pr_info("Driver Removed\n");
  
}


module_init(snes_controller_init);
module_exit(snes_controller_exit);


MODULE_AUTHOR("DevTITANS <devtitans@icomp.ufam.edu.br>");
MODULE_DESCRIPTION("Driver to manage snes controller");
MODULE_LICENSE("GPL");
