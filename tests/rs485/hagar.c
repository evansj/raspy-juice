#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "juice-dev.h"
#include "../firmware/juice.h"

#define BUFSIZE 64
char inbuf[BUFSIZE];
char outbuf[BUFSIZE];
char sbuf[256];

void dyna_ping(int id);


int main(int argc, char *argv[])
{
    char devbusname[] = "/dev/i2c-0";
    int i2caddr = AVRSLAVE_ADDR;
    char *version;
    
    int count = 0;
    int i, rval, stat, b, adc_v;
    double volts = 0.0;
    
    int servo0pwm = 1500, servo0spddir = 50;
    int servo1pwm = 1500, servo1spddir = 100;
    int servo2pwm = 1500, servo2spddir = 150;
    int servo3pwm = 1500, servo3spddir = 200;

    printf("Hello, world!\n");
    
    rval = rj_open(devbusname, i2caddr);
    if (rval < 0) {
	printf("open %s: failed, rval = %d\n", devbusname, rval);
	exit(1);
    }
    else
	printf("juice: open at 0x%02x: succeeded.\n", i2caddr);

    version = rj_getversion();
    if (version != NULL)
	printf("juice: firmware version = %s\n", version);
    else {
	printf("juice: rj_getversion failed.\n");
	exit(2);
    }
    printf("\n");

    while (1) {

	count++;
	printf("\33[2K\r%06d:", count);
	
	stat = rj_readstat();

	if (stat & RXA485) {
	    rval = rj485_read(inbuf, BUFSIZE);
	    printf("\nrs485: %s:\n", inbuf);
	    for (i = 0; i < rval; i++)
		printf("0x%02x ", inbuf[i]);
	    printf("\n");
	}

	dyna_ping(1);


#if 0
	if ((count % 10) == 0) {
	    sprintf(outbuf, "Hello 485!!! count = %d\n\r", count); 
	    rj485_send(outbuf, strlen(outbuf));
	}
#endif

	usleep(1000000);
    }
    
}
