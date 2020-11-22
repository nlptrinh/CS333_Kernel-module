/*
 * ten file: RanNum_generate.c
 * mo ta   : day la chuong trinh tren user space tuong tac voi vchar_device
 *           vchar_device la mot thiet bi nam tren RAM.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define BUFFER_LENGTH 4
#define DEVICE_NODE "/dev/RanNum_dev"
static char receive[BUFFER_LENGTH];

int main() {
	int ret, fd;

	fd = open(DEVICE_NODE, O_RDWR);  
	if(fd < 0) {
        	printf("Can not open the device file\n");
        	exit(1);
    	}
 
   	printf("\n\nReading from the device...\n");
   	ret = read(fd, receive, BUFFER_LENGTH);  
   	if(ret < 0){
       		printf("Could not read from %s\n", DEVICE_NODE);
       		return errno;
   	}
   	else{
       		printf("Random number is %s\n\n",receive);
   	}
   	close(fd);
   	return 0;
}
