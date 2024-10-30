
#include <linux/kernel.h> 
#include <linux/init.h> 
#include <linux/module.h> 
#include <linux/kdev_t.h> 
#include <linux/fs.h> 
#include <linux/cdev.h> 
#include <linux/device.h> 
#include <linux/delay.h> 
#include <linux/uaccess.h>  //copy_to/from_user() 
#include <linux/gpio.h>     //GPIO 

  
//LED is connected to this GPIO 
#define COUNT 8
#define GPIO_17 (17)  // A
#define GPIO_27 (27)  // B
#define GPIO_22 (22)  // C
#define GPIO_10 (10)  // D
#define GPIO_9 (9)   // E
#define GPIO_11 (11)  // F
#define GPIO_5 (5)   // G
#define GPIO_6 (6)   // H(DP)

  
dev_t dev = 0; 
static int gpios[COUNT] = {GPIO_17, GPIO_27, GPIO_22, GPIO_10, GPIO_9, GPIO_11, GPIO_5, GPIO_6};
static struct class *dev_class; 
static struct cdev etx_cdev; 
  
static int __init etx_driver_init(void); 
static void __exit etx_driver_exit(void); 
void set_gpios(int a,int b, int c, int d, int e,int f, int g, int h){
    gpio_set_value(GPIO_17, a);
    gpio_set_value(GPIO_27, b);
    gpio_set_value(GPIO_22, c);
    gpio_set_value(GPIO_10, d);
    gpio_set_value(GPIO_9, e);
    gpio_set_value(GPIO_11, f);
    gpio_set_value(GPIO_5, g);
    gpio_set_value(GPIO_6, h);
}
  
  
/*************** Driver functions **********************/ 
static int     etx_open(struct inode *inode, struct file *file); 
static int     etx_release(struct inode *inode, struct file *file); 
static ssize_t etx_read(struct file *filp,  
                char __user *buf, size_t len,loff_t * off); 
static ssize_t etx_write(struct file *filp,  
                const char *buf, size_t len, loff_t * off); 
/******************************************************/ 
  
//File operation structure  
static struct file_operations fops = 
{ 
  .owner          = THIS_MODULE, 
  .read           = etx_read, 
  .write          = etx_write, 
  .open           = etx_open, 
  .release        = etx_release, 
}; 
 
/* 
** This function will be called when we open the Device file 
*/  
static int etx_open(struct inode *inode, struct file *file) 
{ 
  pr_info("Device File Opened...!!!\n"); 
  return 0; 
} 
 
/* 
** This function will be called when we close the Device file 
*/ 
static int etx_release(struct inode *inode, struct file *file) 
{ 
  pr_info("Device File Closed...!!!\n"); 
  return 0; 
} 
 
/* 
** This function will be called when we read the Device file 
*/  
static ssize_t etx_read(struct file *filp,  
                char __user *buf, size_t len, loff_t *off) 
{ 
  uint gpio_states[COUNT] = {0};
   
  //reading GPIO value 
  for(int i = 0; i < COUNT; i++){
    gpio_states[i] = gpio_get_value(gpios[i]);
  }

  //write to user 
  len = COUNT; 
  if( copy_to_user(buf, gpio_states, len) > 0) { 
    pr_err("ERROR: Not all the bytes have been copied to user\n"); 
  } 
   
  pr_info("Read function : GPIO states = [%d, %d, %d, %d] \n", gpio_states[0],gpio_states[1], gpio_states[2], gpio_states[3]); 
   
  return 0; 
} 
 
/* 
** This function will be called when we write the Device file 
*/  
static ssize_t etx_write(struct file *filp,  
                const char __user *buf, size_t len, loff_t *off) 
{ 
  char rec_buf[2] = {0}; 
   
  if( copy_from_user( rec_buf, buf, len ) > 0) { 
    pr_err("ERROR: Not all the bytes have been copied from user\n"); 
  } 
   
  pr_info("Write Function : %c\n", rec_buf[0]); 
   
  if (rec_buf[0]=='0') { 
    set_gpios(1, 1, 1, 1, 1, 1, 0, 0); 
  } else if (rec_buf[0]=='1') { 
    set_gpios(0, 1, 1, 0, 0, 0, 0, 0); 
  } else if (rec_buf[0]=='2') { 
    set_gpios(1, 1, 0, 1, 1, 0, 1, 0); 
  } else if (rec_buf[0]=='3'){
    set_gpios(1, 1, 1, 1, 0, 0, 1, 0); 
  } else if (rec_buf[0]=='4'){
    set_gpios(0, 1, 1, 0, 0, 1, 1, 0); 
  } else if (rec_buf[0]=='5'){
    set_gpios(1, 0, 1, 1, 0, 1, 1, 0); 
  } else if (rec_buf[0]=='6'){
    set_gpios(1, 0, 1, 1, 1, 1, 1, 0); 
  } else if (rec_buf[0]=='7'){
    set_gpios(1, 1, 1, 0, 0, 0, 0, 0); 
  } else if (rec_buf[0]=='8'){
    set_gpios(1, 1, 1, 1, 1, 1, 1, 0); 
  } else if (rec_buf[0]=='9'){
    set_gpios(1, 1, 1, 1, 0, 1, 1, 0); 
  } else{
    set_gpios(0, 0, 0, 0, 0, 0, 0, 0);
  }
   
  return len; 
} 
 
/* 
** Module Init function 
*/  
static int __init etx_driver_init(void) 
{ 
  /*Allocating Major number*/ 
  if((alloc_chrdev_region(&dev, 0, 1, "seg_Dev")) <0){ 
    pr_err("Cannot allocate major number\n"); 
    goto r_unreg; 
  } 
  pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev)); 
  
  /*Creating cdev structure*/ 
  cdev_init(&etx_cdev,&fops); 
  
  /*Adding character device to the system*/ 
  /*Such as command "mknod" */
  if((cdev_add(&etx_cdev,dev,1)) < 0){ 
    pr_err("Cannot add the device to the system\n"); 
    goto r_del; 
  } 
  
  /*Creating struct class*/ 
  if((dev_class = class_create(THIS_MODULE,"seg_class")) == NULL){ 
    pr_err("Cannot create the struct class\n"); 
    goto r_class; 
  } 
  
  /*Creating device*/ 
  if((device_create(dev_class,NULL,dev,NULL,"seg_device")) == NULL){ 
    pr_err( "Cannot create the Device \n"); 
    goto r_device; 
  } 
   
  //Checking the GPIO is valid or not 
  /* Verify */
  for (int i = 0; i < COUNT; i++) {
    if (gpio_is_valid(gpios[i]) == false) {
        pr_err("GPIO %d is not valid\n", gpios[i]);
        goto r_device;
    }
 }

   
  //Requesting the GPIO 
  /* Label */
 char label[10];  // safe label string

 for (int i = 0; i < COUNT; i++) {
    sprintf(label, "GPIO_%d", gpios[i]);  // generate labels
    if (gpio_request(gpios[i], label) < 0) {
        pr_err("ERROR: GPIO %d request failed\n", gpios[i]);
        goto r_gpio;
    }
    gpio_direction_output(gpios[i], 0);   //configure the GPIO as output 
 }


  /* Using this call the GPIO 21 will be visible in /sys/class/gpio/ 
  ** Now you can change the gpio values by using below commands also. 
  ** echo 1 > /sys/class/gpio/gpio21/value  (turn ON the LED) 
  ** echo 0 > /sys/class/gpio/gpio21/value  (turn OFF the LED) 
  ** cat /sys/class/gpio/gpio21/value  (read the value LED) 
  **  
  ** the second argument prevents the direction from being changed. 
  */ 
  for (int i = 0; i < COUNT; i++){
    gpio_export(gpios[i], false);
  }
   
  pr_info("Device Driver Insert...Done!!!\n"); 
  return 0; 
  
r_gpio: 
  for (int i = 0; i < COUNT; i++) {
    gpio_free(gpios[i]);
  }
r_device: 
  device_destroy(dev_class,dev); 
r_class: 
  class_destroy(dev_class); 
r_del: 
  cdev_del(&etx_cdev); 
r_unreg: 
  unregister_chrdev_region(dev,1); 
   
  return -1; 
} 
 
/* 
** Module exit function 
*/  
static void __exit etx_driver_exit(void) 
{ 
  for(int i = 0; i< COUNT ; i++){
    gpio_unexport(gpios[i]); 
    gpio_free(gpios[i]); 
  }
  
  device_destroy(dev_class,dev); 
  class_destroy(dev_class); 
  cdev_del(&etx_cdev); 
  unregister_chrdev_region(dev, 1); 
  pr_info("Device Driver Remove...Done!!\n"); 
} 
  
module_init(etx_driver_init); 
module_exit(etx_driver_exit); 
  
MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("EmbeTronicX <embetronicx@gmail.com>"); 
MODULE_DESCRIPTION("A simple device driver - GPIO Driver"); 
MODULE_VERSION("1.32"); 