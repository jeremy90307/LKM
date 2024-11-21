#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ioctl_ex.h"
 
int main()
{
        int32_t value;
 
        printf("\nOpening Driver\n");
        int dev = open("/dev/ioctl_ex", O_RDWR);
        if(dev == -1) {
                printf("Cannot open device file...\n");
                return 0;
        }

        ioctl(dev, RD_VALUE, (int32_t*) &value);
        printf("Initial value is %d\n", value);

        printf("Enter the Value to send\n");
        scanf("%d",&value);
        printf("Writing Value to Driver\n");
        ioctl(dev, WR_VALUE, (int32_t*) &value); 
 
        printf("Reading Value from Driver\n");
        ioctl(dev, RD_VALUE, (int32_t*) &value);
        printf("Value is %d\n", value);
 
        printf("Closing Driver\n");
        close(dev);
}