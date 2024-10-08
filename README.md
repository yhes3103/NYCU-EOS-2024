# NYCU-EOS-2024
    These are records of my codes, reports and what I've learned in NYCU Embedded Operating System. 
## Pretest
- Only those who pass the pretest can take this course.
## Lab1
- This lab is about setting up environment on Raspberry Pi.
- What I've learned
    * NAT
    * Bridge
## Lab2
- This lab is about Building kernel.
- We are required to meet the goals
    * Q1: Shrink the size of your kernel image. 
    * Q2: Benchmark your kernel, revise it, and improve its performance. Rerun the benchmark to prove your performance. 
    * Q3: Patch your kernel to support real-time tasks.  
## Hw1
## Lab4
- This lab is about creating my driver.
- Create kernel object(.ko) then insert module
    * make sure you've already set the `$(PWD)` to the correct file. In my case, `PWD=/home/wang/linux`.
    * `make` with Makefile
- `scp pi@ipaddr my_driver.ko` to send .ko file to Rpi.
- Insert module and create device node.
    * `sudo insmod my_driver` and `sudo rmmod my_driver.ko` to insert and remove.
    * `sudo mknod /dev/my_device c 255 0` to make device node. c stands for character device, 255 and 0 for major num and minor num.
    * `dmesg` to see any information for kernel info.
    * `ls -l /dev` to see if device node created or not.
    * `echo "A">/dev/my_device` for testing. You should see more information after `dmesg`. 
- writer
    * Usage: `./writer <name>` 
- reader
    * Usage: `./reader <server ip> <port> </dev/my_device>`
- seg.py
    * Usage: `python3 seg.py <port>`
- What I've learned
    * cross compile set up in Makefile.
    * Uniform API which is defined in /include/linux/fs.h.
    * basic driver.c structure and compile into .ko file.
    * Copy_from_user(), Copy_to_user()
    * driver table, device table
