#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>

#include "../firmware/juice.h"

int rj_readstat(void);
int rj232_getc(void);
int rj485_getc(void);
int rj232_read(void);
int rj485_read(void);
int rj232_send(unsigned char *buf, int len);
int rj485_send(unsigned char *buf, int len);
int rj_setservo(int chan, int usec);
int rj_readadc(unsigned char mux);
int rj_readver(void);

#define BUFSIZE 64
char inbuf[BUFSIZE];
char outbuf[BUFSIZE];

int file;

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

    i = 0;
    rj_writeword(RJ_VERSION, __func__);

    do {
	inbuf[i] = rj_readbyte(RJ_VERSION, __func__);
    } while ((inbuf[i] != 0) && (i++ < (BUFSIZE - 2) ) );

    inbuf[i] = 0;

    printf("len = %d, Version = %s\n", strlen(inbuf), inbuf);

    close(file);

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
	inbuf[i++] = rj485_getc();
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
