#ifndef IOCTL_EX_H
#define IOCTL_EX_H

#define IOC_MAGIC 100

/* Set the message of the device */
#define WR_VALUE  _IOW(IOC_MAGIC, 0, int32_t *)

/* Get the message of the device */
#define RD_VALUE  _IOR(IOC_MAGIC, 1, int32_t *)

int32_t value = 0;

#endif