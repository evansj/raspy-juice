#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c-dev.h>

#include "../firmware/juice.h"

#define I2C_NUM_RETRIES	3
#define RETRY_TIMEOUT	20000

static int rj_file = 0;
static int rj_errno;

int rj_readbyte(int subreg);
int rj_writebyte(int subreg, int data);
int rj_readword(int subreg);
int rj_writeword(int subreg, int data);
int rj_readblock(int subreg, char *inbuf);

int rj_readbyte_dbg(int subreg, const char *caller);
int rj_writebyte_dbg(int subreg, int data, const char *caller);
int rj_readword_dbg(int subreg, const char *caller);
int rj_writeword_dbg(int subreg, int data, const char *caller);

#define BUFSIZE 64
char version_str[BUFSIZE];
unsigned char rs232_inbuf[BUFSIZE];
unsigned char rs485_inbuf[BUFSIZE];

int rj_open(const char *devbusname, int i2caddr)
{
    int rval;

    rj_file = open(devbusname, O_RDWR);
    if (rj_file < 0) {
	rj_errno = rj_file;
	return -1;
    }
    if ((rval = ioctl(rj_file, I2C_SLAVE, i2caddr)) < 0) {
	rj_errno = rval;
	return -2;
    }
    return rj_file;
}

char *rj_getversion(void)
{
    int i;

    i = 0;
    rj_writebyte(RJ_VERSION, 0);

    do {
	version_str[i] = rj_readbyte(RJ_VERSION);
    } while ((version_str[i] != 0) && (i++ < (BUFSIZE - 2) ) );

    version_str[i] = 0;
    return version_str;
}

int rj_setservo(int chan, int usec)
{
    return rj_writeword(chan, usec);
}

int rj_readstat(void)
{
    return rj_readbyte(GSTAT);
}

int rj_readadc(unsigned char mux)
{
    int rval;
    rval = rj_writebyte(ADCMUX, mux);
    rval = rj_readword(ADCDAT);
    return rval;
} 

int rj232_getc(void)
{
    return rj_readbyte(RS232D);
}

int rj232_read(unsigned char *inbuf, int maxlen)
{
    int i = 0;
    while ((rj_readstat() & RXA232) && (i < maxlen)) {
	inbuf[i++] = rj232_getc();
    }
    inbuf[i] = 0;
    return i;
}

int rj485_getc(void)
{
    return rj_readbyte(RS485D);
}

int rj485_read(void)
{
    int i = 0;
    while ((rj_readstat() & RXA485) && (i < BUFSIZE)) {
	rs485_inbuf[i++] = rj485_getc();
    }
    rs485_inbuf[i] = 0;
    return i;
}

int rj232_send(unsigned char *buf, int len)
{
    int i, rval;
    for (i = 0; i < len; i++)
	rval = rj_writebyte(RS232D, *buf++);
    return rval;
} 

int rj485_send(unsigned char *buf, int len)
{
    int i, rval;
    for (i = 0; i < len; i++)
	rval = rj_writebyte(RS485D, *buf++);
    return rval;
}


/**********************************************************************
 *
 **********************************************************************/

int rj_readbyte(int subreg)
{
    int rval, retry = 3;
    do {
	rval = i2c_smbus_read_byte_data(rj_file, subreg);
	if (rval < 0) {
	    retry--;
	    usleep(RETRY_TIMEOUT);
	}
    } while ((rval < 0) && (retry > 0));
    
    if (rval < 0)
	fprintf(stderr, "i2c_smbus_read_byte_data failed.\n");

    return rval;
}   

int rj_readword(int subreg)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_read_word_data(rj_file, subreg);
	if (rval < 0) {
	    retry--;
	    usleep(RETRY_TIMEOUT);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	fprintf(stderr, "i2c_smbus_read_word_data failed.\n");

    return rval;
}   

int rj_readblock(int subreg, char *inbuf)
{
    int rval, retry = 3;
    do {
	rval = i2c_smbus_read_block_data(rj_file, subreg, inbuf);
	if (rval < 0) {
	    retry--;
	    usleep(RETRY_TIMEOUT);
	}
    } while ((rval < 0) && (retry > 0));
   
    if (rval < 0)
	fprintf(stderr, "i2c_smbus_read_block_data failed.\n");

    return rval;
}


int rj_writebyte(int subreg, int data)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_write_byte_data(rj_file, subreg, data);
	if (rval < 0) {
	    retry--;
	    usleep(RETRY_TIMEOUT);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	fprintf(stderr, "i2c_smbus_write_byte_data failed.\n");

    return rval;
}   

int rj_writeword(int subreg, int data)
{
   int rval, retry = 3;
    do {
	rval = i2c_smbus_write_word_data(rj_file, subreg, data);
	if (rval < 0) {
	    retry--;
	    usleep(RETRY_TIMEOUT);
	}
    } while ((rval < 0) && (retry > 0));

    if (rval < 0)
	fprintf(stderr, "i2c_smbus_write_word_data failed.\n");

    return rval;
}   

typedef struct avr_swuart_baud_setting {
    int bps;
    unsigned char prescaler;
    unsigned char ticks;
} avr_swuart_baud_setting;

struct avr_swuart_baud_setting avr_rates[] = {
    { 1200, 0x05, 96 },
    { 2400, 0x04, 96 },
    { 4800, 0x04, 48 },
    { 9600, 0x04, 24 },
    { 14400, 0x04, 16 },
    { 19200, 0x04, 12 },
    { 38400, 0x04, 6 },
};

int avr_rates_max = sizeof(avr_rates) / sizeof (avr_swuart_baud_setting);


int rj232_setbaud(int bps)
{
    int i = 0;
    
    while (i < avr_rates_max) {
	if (avr_rates[i].bps == bps) {
	    printf("rs232_setbaud: FOUND\n");
	    return rj_writeword(RS232BPS, (avr_rates[i].prescaler << 8) | 
				(avr_rates[i].ticks));
	}
	i++;
    }
    return -1;
}

