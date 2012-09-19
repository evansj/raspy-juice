#ifndef __JUICE_DEV_H__
#define __JUICE_DEV_H__

#include "../firmware/juice.h"

#define BUFSIZE 64
char version_str[BUFSIZE];
unsigned char rs232_inbuf[BUFSIZE];
unsigned char rs485_inbuf[BUFSIZE];

int rj_readbyte(int subreg);
int rj_writebyte(int subreg, int data);
int rj_readword(int subreg);
int rj_writeword(int subreg, int data);

int rj_open(const char *devbusname, int i2caddr);
char *rj_getversion(void);
int rj_setservo(int chan, int usec);
int rj_readstat(void);
int rj_readadc(unsigned char mux);
int rj232_read(void);
int rj485_read(void);
int rj232_getc(void);
int rj485_getc(void);
int rj232_send(unsigned char *buf, int len);
int rj485_send(unsigned char *buf, int len);

#endif /* __JUICE_DEV_H__ */
