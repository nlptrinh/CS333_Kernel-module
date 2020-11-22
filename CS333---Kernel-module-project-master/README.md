# CS333---Kernel-module-project
Generate random number module

* `make all` to build RanNum_drv.ko
* `sudo insmod RanNum_drv.ko` to install module to kernel
* `cat /proc/devices | grep RanNum_drv` to check the major and minor number of driver
* `dmesg` to view kernel log -> You should see `Initial RanNum driver successfully`
* `sudo chmod 666 /dev/RanNum_dev` to give access for user app
* `ls -la /dev/RanNum_dev` to check giving access successfully
* `cd RanNum_app`
* `make all` to build RanNum_generate.c
* `./RanNum_generate` to run the program. Now you should see a random number is display in your program
* `dmesg` to check the random number in kernel log
* Make sure to remove module after you finish running by using `sudo rmmod RanNum_drv`
