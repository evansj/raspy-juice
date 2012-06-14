#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>

int main(int argc, char *argv[])
{
    int count = 0;
    int file;
    int adapter_nr = 0; /* probably dynamically determined */
    char devname[20];
    int addr = 0x48; /* The I2C address */
    unsigned char reg = 0x00; /* Device register to access */
    int rdval;
    char buf[10];
    
    printf("Hello, world!\n");
  
    snprintf(devname, 19, "/dev/i2c-%d", adapter_nr);
    file = open(devname, O_RDWR);
    if (file < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      printf("open %s: error = %d\n", devname, file);
      exit(1);
    }
    else
      printf("open %s: succeeded.\n", devname);

    if (ioctl(file, I2C_SLAVE, addr) < 0) {
      /* ERROR HANDLING; you can check errno to see what went wrong */
      printf("open i2c slave 0x%02x: error = %s\n", addr, "dunno");
      exit(1);
    }
    else
      printf("open i2c slave 0x%02x: succeeded.\n", addr);


    while (1) {
	printf("Another try %d\n", count++);

        printf("Another try %d\n", count++);


	/* Using SMBus commands */
	rdval = i2c_smbus_read_byte_data(file, reg);
	if (rdval < 0) {
	  /* ERROR HANDLING: i2c transaction failed */
	  printf("i2c_smbus_read_byte transaction failed.\n");
	} else {
	  printf("i2c_smbus_read_byte value = 0x%02x\n", rdval);
	}


	usleep(500000);
    }
    
}

   
