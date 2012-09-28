#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "juice-dev.h"
#include "../firmware/juice.h"

#include <asm-generic/termbits.h>

#define BUFSIZE 64
char inbuf[BUFSIZE];
char outbuf[BUFSIZE];

int main(int argc, char *argv[])
{
    char devbusname[] = "/dev/i2c-0";
    int i2caddr = AVRSLAVE_ADDR;
    char *version;

    int count = 0;
    int i, rval, stat, b, baud;
    
    printf("Hello, world! I am example3 - reading/writing data from AVR-RS485");
    
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

    baud = 230400;
    if (rj485_setbaud(baud))
	printf("rs485: set baud failed.\n");
    else
	printf("rj485: set baud to %dbps succeeded.\n", baud);

    while (1) {
	count++;
	printf("\33[2K\r%06d:", count);
	
	stat = rj_readstat();
	if (stat >= 0) {
	  for (b = 7; b >= 0; b--)
	    printf("%c", (stat & (1 << b)) ? '1' : '0');
	  printf(": ");
	  fflush(stdout);
	  if (stat & ADCBUSY)
	    printf("adcbusy:");
	  if (stat & EEBUSY)
	    printf("eebusy:");
	  if (stat & RXA485)
	    printf("rs485:");
	  if (stat & RXA232)
	    printf("rs232:");
	}

	fflush(stdout);

	if (stat & RXA485) {
	    rval = rj485_read((unsigned char *)inbuf, BUFSIZE);
	    printf("\nrs485: ");
	    for (i = 0; inbuf[i] != 0; i++)
		printf("0x%02x ", inbuf[i]);
	    printf(": %s \n", inbuf);
	}

	if ((count % 10) == 0) {
	    sprintf(outbuf, "Hello 485!!! count = %d\n\r", count);
	    rj485_send((unsigned char *)outbuf, strlen(outbuf));
	}

	usleep(100000);
    }
    
}

