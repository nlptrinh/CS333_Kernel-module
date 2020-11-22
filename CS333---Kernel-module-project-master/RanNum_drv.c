#include <linux/module.h>	//dinh nghia cac macro nhu module_init
#include <linux/fs.h>		//dinh nghia ham cap phat/giai phong device number
#include <linux/device.h>	//chua cac ham phuc vu viec tao device file
#include <linux/slab.h>		//chua cac ham kmalloc & kfree de cap phat bo nho
#include <linux/cdev.h>		//chua cac ham lam viec voi cdev
#include <linux/uaccess.h> 	//chua cac ham trao doi data giua user & kernel
#include <linux/random.h>	//get_random_bytes

#include "RanNum_drv.h"	//thu vien mo ta cac thanh ghi cua RandomNumber device

#define DRIVER_AUTHOR "Vy Huynh <1751027> | Trinh Ngo <nlptrinh@apcs.vn> | Truc Nguyen <1751112>"
#define DRIVER_DESC "Generate random number module"

char kernel_buffer[4] = {'\0'};
typedef struct vchar_dev {
	unsigned char * control_regs;
	unsigned char * status_regs;
	unsigned char * data_regs;
} vchar_dev_t;

struct _vchar_drv
{
	dev_t dev_num;
	struct class *dev_class;
	struct device *dev;
	vchar_dev_t * vchar_hw;
	struct cdev *vcdev;
	unsigned int open_cnt;
} vchar_drv;

//**************** device specific - START ***************/
// Ham khoi tao thiet bi
int vchar_hw_init(vchar_dev_t *hw)
{
	char * buf;
	buf = kzalloc(NUM_DEV_REGS * REG_SIZE, GFP_KERNEL);
	if(!buf) {
		return -ENOMEM;
	}

	hw->control_regs = buf;
	hw->status_regs = hw->control_regs + NUM_CTRL_REGS;
	hw->data_regs = hw->status_regs + NUM_STS_REGS;

	//khoi tao gia tri ban dau cho cac thanh ghi
	hw->control_regs[CONTROL_ACCESS_REG] = 0x03;
	hw->status_regs[DEVICE_STATUS_REG] = 0x03;

	return 0;
}

// ham giai phong thiet bi
void vchar_hw_exit(vchar_dev_t *hw)
{
	kfree(hw->control_regs);
}

// ham doc tu cac thanh ghi du lieu cua thiet bi
/*int vchar_hw_read_data(vchar_dev_t *hw, int start_reg, int num_regs, char* kbuf)
{
	int read_bytes = num_regs;
	
	//kiem tra quyen doc du lieu
	if ((hw->control_regs[CONTROL_ACCESS_REG] & CTRL_READ_DATA_BIT) == DISABLE)
		return -1;
	//kiem tra dia chi cua kernel co hop le khong
	if(kbuf == NULL)
		return -1;
	//kiem tra vi tri thanh ghi can doc
	if(start_reg > NUM_DATA_REGS)
		return -1;
	//dieu chinh sl thanh ghi data can doc
	if(num_regs > (NUM_DATA_REGS - start_reg))
		read_bytes = NUM_DATA_REGS - start_reg;
	//ghi du lieu tu kernel buffer vao cac thanh ghi
	memcpy(kbuf, hw->data_regs + start_reg, read_bytes);

	//cap nhat so lan doc tu thanh ghi du lieu
	hw->status_regs[READ_COUNT_L_REG] += 1;
	if(hw->status_regs[READ_COUNT_L_REG] == 0)
		hw->status_regs[READ_COUNT_H_REG] += 1;
	//tra ve so byte da doc duoc tu thanh ghi du lieu
	return read_bytes;
}*/

// ham doc tu thanh ghi trang thai cua thiet bi

// ham ghi vao cac thanh ghi dieu khien cua thiet bi

/************************* device specific - END **************/

/********************* OS specific - START ********************/
// cac ham entry point
static int RanNum_driver_open(struct inode *inode, struct file *filp)
{
	vchar_drv.open_cnt++;
	printk("Handle open event (%d)\n", vchar_drv.open_cnt);
	return 0;
}

static int RanNum_driver_release(struct inode *inode, struct file *filp)
{
	printk("Handle close event");
	return 0;
}

static ssize_t RanNum_driver_read(struct file *filp, char *user_buffer, size_t len, loff_t *offset)
{
	int randomNumber;
	int i = 4; //kernel buffer size, 3 slot for number, 1 slot for terminator
	
	printk("Handle read event"); 
    	get_random_bytes(&randomNumber, sizeof(randomNumber));
	randomNumber %= 1000;

	printk(KERN_INFO "Random number is %d\n", randomNumber);

	if(randomNumber != 0)
	{
		if(randomNumber < 0)
			randomNumber += 1000;		
	}
	i -= 2;
	while (randomNumber > 0)
	{
		kernel_buffer[i] = randomNumber % 10 + '0';
		randomNumber /= 10;
		--i;
	}

	//num_bytes = vchar_hw_read_data(vchar_drv.vchar_hw, *off, len, kernel_buf);
	//printk("Read %d bytes from HW\n",num_bytes);

	/*if(num_bytes < 0)
		return -EFAULT;*/
	printk(KERN_INFO "Random number is [Kernel_buffer] %s\n", kernel_buffer);
	//copy_to_user(user_buffer, kernel_buffer, len);
	if(copy_to_user(user_buffer, kernel_buffer, len))
		return -EFAULT;

	return 0;
}

/*static ssize_t RanNum_driver_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int i = 0;
    int randomNumber;
    get_random_bytes(&randomNumber, sizeof(randomNumber));
    if (copy_to_user(buffer, kernel_buffer, len))
    {
        printk(KERN_INFO "\n\nRANDOMMACHINE: Failed\n");
        return -EFAULT;
    }
    if (randomNumber != 0)
    {
	
        while (randomNumber != 0)
        {
            temp[i] = randomNumber % 10 + '0';
            randomNumber = randomNumber / 10;
            i++;
        }
        temp[i] = '\0';
        buffer[i] = '\0';
        i -= 1;
        while (i >= 0)
        {
            *buffer = temp[i];
            i -= 1;
            buffer += 1;
        }
        return 0;
    }
    else
    {

        *(buffer++) = '0';
        *buffer = '\0';
        return 0;
    }
	printk(KERN_INFO "Random number is %d\n", randomNumber);
	return 0;
}*/

static struct file_operations fops = 
{
	.owner = THIS_MODULE,
	.open = RanNum_driver_open,
	.release = RanNum_driver_release,
	.read = RanNum_driver_read,
};

// ham khoi tao driver
static int __init RanNum_driver_init(void)
{
	int ret = 0;
	
	// cap phat device number
	vchar_drv.dev_num = 0;
	ret = alloc_chrdev_region(&vchar_drv.dev_num, 0, 1, "RanNum_device");
	if (ret < 0) {
		printk("Failed to register device number\n");
		goto failed_register_devnum;
	}
	printk("Allocated device number (%d,%d)\n", MAJOR(vchar_drv.dev_num), MINOR(vchar_drv.dev_num));

	// tao device file
	vchar_drv.dev_class = class_create(THIS_MODULE, "class_RanNum_dev");
	if(vchar_drv.dev_class == NULL) {
		printk("Failed to create a device class\n");
		goto failed_create_class;	
	}
	
	vchar_drv.dev = device_create(vchar_drv.dev_class, NULL, vchar_drv.dev_num, NULL, "RanNum_dev");
	if(IS_ERR(vchar_drv.dev)) {
		printk("Failed to create a device\n");
		goto failed_create_device;
	}

	// cap phat bo nho cho cau truc du lieu cua character driver
	vchar_drv.vchar_hw = kzalloc(sizeof(vchar_dev_t), GFP_KERNEL);
	if(!vchar_drv.vchar_hw) {
		printk("Failed to allocate data structure of driver\n");
		ret = -ENOMEM;
		goto failed_allocate_structure;
	}

	// khoi tao thiet bi vat li
	ret = vchar_hw_init(vchar_drv.vchar_hw);
	if (ret < 0) {
		printk("Failed to initialize a virtual character device\n");
		goto failed_init_hw;
	}

	// dki entry points voi kernel
	vchar_drv.vcdev = cdev_alloc();
	if(vchar_drv.vcdev == NULL) {
		printk("Failed to allocate cdev structure\n");
		goto failed_allocate_cdev;
	}
	cdev_init(vchar_drv.vcdev, &fops);
	ret = cdev_add(vchar_drv.vcdev, vchar_drv.dev_num, 1);
	if(ret < 0) {
		printk("Failed to add a char device to the system\n");
		goto failed_allocate_cdev;
	}

	// dki ham xu ly ngat

	printk("Initialize RanNum driver successfully\n");
	return 0;

failed_allocate_cdev:
	vchar_hw_exit(vchar_drv.vchar_hw);
failed_init_hw:
	kfree(vchar_drv.vchar_hw);
failed_allocate_structure:
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
failed_create_device:
	class_destroy(vchar_drv.dev_class);
failed_create_class:
	unregister_chrdev_region(vchar_drv.dev_num, 1);
failed_register_devnum:
	return ret;
}

// ham ket thuc driver
static void __exit RanNum_driver_exit(void)
{
	//huy dang ky xu li ngat
	
	//huy dang ky entry point voi kernel
	cdev_del(vchar_drv.vcdev);

	//giai phong thiet bi vat ly
	vchar_hw_exit(vchar_drv.vchar_hw);

	//giai phong bo nho da cap phat du lieu cua driver
	kfree(vchar_drv.vchar_hw);

	//xoa bo device file
	device_destroy(vchar_drv.dev_class, vchar_drv.dev_num);
	class_destroy(vchar_drv.dev_class);


	//giai phong device number
	unregister_chrdev_region(vchar_drv.dev_num, 1);

	printk("Exit RanNum driver\n");
}
/*************************OS specific - END *************************/


module_init(RanNum_driver_init);
module_exit(RanNum_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("testdevice");
