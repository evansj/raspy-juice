#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>

#include "juice.h"

int file;
int rj_readstat(void);
int rj232_getc(void);
int rj485_getc(void);
int rj232_read(void);
int rj485_read(void);
int rj232_send(unsigned char *buf, int len);
int rj485_send(unsigned char *buf, int len);
int rj_setservo(int chan, int usec);
int rj_readadc(unsigned char mux);

#define BUFSIZE 64
char inbuf[BUFSIZE];
char outbuf[BUFSIZE];

int main(int argc, char *argv[])
{
    char devbusname[] = "/dev/i2c-0";
    int i2caddr = AVRSLAVE_ADDR;
    
    int count = 0;
    int i, rval, stat, b, adc_v;
    char buf[10];
    double volts = 0.0;
    
    printf("Hello, world!\n");
    
    file = open(devbusname, O_RDWR);
    if (file < 0) {
	printf("open %s: error = %d\n", devbusname, file);
	exit(1);
    }
    else
	printf("open %s: succeeded.\n", devbusname);
    
    if (ioctl(file, I2C_SLAVE, i2caddr) < 0) {
	printf("open i2c slave 0x%02x: error = %s\n\n", i2caddr, "dunno");
	exit(1);
    }
    else
	printf("open i2c slave 0x%02x: succeeded.\n\n", i2caddr);
    
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

	adc_v = rj_readadc(0x47) & 0x3ff;
	volts = 38.14922 * adc_v / 0x3ff;
	printf("  ADC = 0x%04x, % 4d, %f ", adc_v, adc_v, volts);

	for(i = 0; i < 2 * volts; i++)
	    printf("*");
	

	fflush(stdout);

	usleep(100000);
    }
    
}

int rj_readbyte(int subreg, const char *caller)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_read_byte_data(file, subreg);
	if (rval < 0) {
	    printf("rB from %s\n", caller);
	    retry--;
	    usleep(20000);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	printf("%s: i2c_smbus_read_byte retries failed r = 0x%x.\n",
	       caller, rval);
    return rval;
}   

int rj_readword(int subreg, const char *caller)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_read_word_data(file, subreg);
	if (rval < 0) {
	    printf("rW from %s\n", caller);
	    retry--;
	    usleep(20000);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	printf("%s: i2c_smbus_read_word retries failed r = 0x%x.\n",
	       caller, rval);
    return rval;
}   

int rj_writebyte(int subreg, int data, const char *caller)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_write_byte_data(file, subreg, data);
	if (rval < 0) {
	    printf("wB from %s\n", caller);
	    retry--;
	    usleep(20000);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	printf("%s: i2c_smbus_write_byte retries failed r = 0x%x.\n",
	       caller, rval);
    return rval;
}   

int rj_writeword(int subreg, int data, const char *caller)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_write_word_data(file, subreg, data);
	if (rval < 0) {
	    printf("wW from %s\n", caller);
	    retry--;
	    usleep(20000);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	printf("%s: i2c_smbus_write_word retries failed r = 0x%x.\n",
	       caller, rval);
    return rval;
}   

int rj_readstat(void)
{
    return rj_readbyte(GSTAT, __func__);
}

int rj_readadc(unsigned char mux)
{
    int rval;
    rval = rj_writebyte(ADCMUX, mux, __func__);
    rval = rj_readword(ADCDAT, __func__);
    return rval;
} 

int rj232_read(void)
{
    int i = 0;
    while ((rj_readstat() & RXA232) && (i < BUFSIZE)) {
	inbuf[i++] = rj232_getc();
    }
    inbuf[i] = 0;
    return i;
}

int rj485_read(void)
{
    int i = 0;
    while ((rj_readstat() & RXA485) && (i < BUFSIZE)) {
	inbuf[i++] = rj485_getc() & 0x7f;
    }
    inbuf[i] = 0;
    return i;
}

int rj232_getc(void)
{
    return rj_readbyte(RS232D, __func__);
}

int rj485_getc(void)
{
    return rj_readbyte(RS485D, __func__);
}

int rj232_send(unsigned char *buf, int len)
{
    int i, rval;
    for (i = 0; i < len; i++)
	rval = rj_writebyte(RS232D, *buf++, __func__);
} 

int rj485_send(unsigned char *buf, int len)
{
    int i, rval;
    for (i = 0; i < len; i++)
	rval = rj_writebyte(RS485D, *buf++, __func__);
}

int rj_setservo(int chan, int usec)
{
    return rj_writeword(chan, usec, __func__);
}
