#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h> /*use for copy_to_user, copy_from_user*/

#define MAJOR_NUM 255
#define DEVICE_NAME "my_device"

MODULE_LICENSE("GPL");

static char namebuf[2] = {0};



// File Operations

static ssize_t my_read(struct file *fp, char *buf, size_t count, loff_t *fpos) {
    char* seg_for_c[27] = {
        "1111001100010001", // A
        "0000011100000101", // b
        "1100111100000000", // C
        "0000011001000101", // d
        "1000011100000001", // E
        "1000001100000001", // F
        "1001111100010000", // G
        "0011001100010001", // H
        "1100110001000100", // I
        "1100010001000100", // J
        "0000000001101100", // K
        "0000111100000000", // L
        "0011001110100000", // M
        "0011001110001000", // N
        "1111111100000000", // O
        "1000001101000001", // P
        "0111000001010000", // q
        "1110001100011001", // R
        "1101110100010001", // S
        "1100000001000100", // T
        "0011111100000000", // U
        "0000001100100010", // V
        "0011001100001010", // W
        "0000000010101010", // X
        "0000000010100100", // Y
        "1100110000100010", // Z
        "0000000000000000"  // 空格
    };

    printk(KERN_INFO "my_read\n");

    int output = 0;
    if((int)namebuf[0] >= 65 && (int)namebuf[0] <= 90){
        output = (int)namebuf[0]- 65;
    }else if((int)namebuf[0] >= 97 && (int)namebuf[0] <= 122){
        output = (int)namebuf[0]- 97;
    }else if((int)namebuf[0] == 32){
        output = 26; //ASCII for space is 32
    }

    /*memory copy from kernel to buffer*/
    /*copy_to_user("user buffer", "kernel buffer", "number of char to be copied")*/
	int ret = 0;
    ret = copy_to_user(buf, seg_for_c[output], 16);
	if (ret == 0) {
		printk(KERN_INFO "kernel send data: %s\n", namebuf);
	} else {
		/* error handle */
		printk(KERN_ERR "Failed to send data to user space\n");
	}

	return count;
}

static ssize_t my_write(struct file *fp,const char *buf, size_t count, loff_t *fpos){
	int ret = 0;
	printk(KERN_INFO "my_write\n");
	
	/* copy data from user space to namebuf */
    /*
    copy_from_user("kernel buffer", "user buffer", "number of char to be copied")
    return number of char which haven't been transmitted
    return 0 means success 
    */
	ret = copy_from_user(namebuf, buf, count);
		if (ret == 0) {
		printk(KERN_INFO "kernel receive data: %s\n", namebuf);
	} else {
		/* error handle */
		printk(KERN_ERR "Failed to receive data from user space\n");
	}    
   
	return count;
}

static int my_open(struct inode *inode, struct file *fp){
	printk(KERN_INFO "my device open\n");
	return 0;
}

struct file_operations my_fops = {
	read: my_read,
	write: my_write,
	open:my_open
};


static int my_init(void) {
	printk(KERN_INFO"module init\n");
	if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0) {	
		printk(KERN_INFO"Can not get major %d\n", MAJOR_NUM);
		return (-EBUSY);
	}
	printk(KERN_INFO"My device is started and the major is %d\n", MAJOR_NUM);
	return 0;
}

static void my_exit(void) {
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_INFO"call exit\n");
}

module_init(my_init);
module_exit(my_exit);
