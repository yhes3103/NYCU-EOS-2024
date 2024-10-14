# NYCU-EOS-2024
    These are records of my codes, reports and what I've learned in NYCU Embedded Operating System. 
## Pretest
- Only those who pass the pretest can take this course.
## Lab1
- This lab is about setting up environment on Raspberry Pi.
- What I've learned:
    * NAT
    * Bridge
## Lab2
- This lab is about Building kernel.
- We are required to meet the goals.
    * Q1: Shrink the size of your kernel image. 
    * Q2: Benchmark your kernel, revise it, and improve its performance. Rerun the benchmark to prove your performance. 
    * Q3: Patch your kernel to support real-time tasks.  
## Lab3
- This lab is about creating my own driver based on GPIO driver.
- It's suggested that you check Lab 4 first, as I completed it earlier. More details are recorded there.
- Notes:
    Here are some basic GPIO commands.
    * Each GPIOs imformaiton are under /sys/class/gpio, you should see those GPIOs which have been exported.
    * `& echo 17 > /sys/class/gpio/export` to export GPIO17 for example. Use `&ls` to check.
    * `& echo "out" > /sys/class/gpio/gpio17/direction` to make gpio17 output mode.
    * `& echo 1 > /sys/class/gpio/gpio17/value` to set value.
    * `& cat /sys/class/gpio/gpio17/value` to see the value of GPIO17.
    # Lab3-1
    - `& sudo insmod lab3-1_driver.ko` to insert kernel object.
    - `& ls -l /dev` to check whether the device has been created or not. For more imformation, use `& dmesg|tail`.
    - `& sudo chmod 777 /dev/etx_device` as I name my driver etx_device.
    - lab3-1_writer.c
        * Usage: `$ ./lab3-1_writer <student ID>` 
    # Lab3-2
    - `& sudo insmod lab3-2_driver.ko` to insert kernel object.
    - `& ls -l /dev` to check whether the device has been created or not. For more imformation, use `& dmesg|tail`.
    - `& sudo chmod 777 /dev/seg_device` as I name my driver seg_device.
    - lab3-2_writer.c
        * Usage: `$ ./lab3-2_writer <student ID>` 
## Hw1
## Lab4
- This lab is about creating my driver.
- Create kernel object(.ko) then insert module.
    * make sure you've already set the `$(PWD)` to the correct file. In my case, `& PWD=/home/wang/linux`.
    * `$ make` with Makefile
- `$ scp mydev.ko pi@ipaddr:~/` to send .ko file to Rpi.
- Insert module and create device node.
    * `& sudo insmod mydev.ko` and `sudo rmmod mydev.ko` to insert and remove.
    * `& sudo mknod /dev/my_device c 255 0` to make device node. c stands for character device, 255 and 0 for major num and minor num.
    * `& sudo rm /dev/my_device` to delete device node.
    * `& dmesg` to see any information for kernel info.
    * `& ls -l /dev` to see if device node created or not.
    * `& echo "A">/dev/my_device` for testing. You should see more information after `dmesg`. Make sure you have sufficient permissions to read and write, if not, use `$ sudo chmod 666 /dev/my_device`
- writer.c
    * Usage: `$ ./writer <name>` 
- reader.c
    * Usage: `$ ./reader <server ip> <port> </dev/my_device>`
- run reader and writer at the same time.
    * `sudo ./demo.sh` to run reader and writer at the same time.
- seg.py
    * Usage: `$ python3 seg.py <port>`
- What I've learned:
    * cross compile set up in Makefile.
    * uniform APIs which are defined in /include/linux/fs.h.
    * basic driver.c structure and compile into .ko file.
    * Copy_from_user(), Copy_to_user()
    * driver table, device table
- Notes:
    * A major number is a unique identifier assigned to a device driver in the Linux kernel. On the other hands, A minor number is a smaller identifier that is used in conjunction with the major number to uniquely identify a specific device within a class of devices. For example, mojor number for a disk driver is 8, minor number for /dev/sda, /dev/sdb, /dev/sdc is 0, 1, 2.
    * To create more devices from the same driver, just `mknod`. For example, `mknod /dev/my_device1 c 255 0` and `mknod /dev/my_device2 c 255 1` to create different device instances.

