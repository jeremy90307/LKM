obj-m += ioctl_ex.o

obj = test.c
prom = test

all: $(prom) modules

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

$(prom): $(obj)
	gcc -o $(prom) $(obj)

clean:	
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f $(prom)