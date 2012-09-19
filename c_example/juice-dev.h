#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>

#include "../firmware/juice.h"

#define RETRY_TIMEOUT	20000


#if 0
#define GSTAT	0x00
#define RXA232	0x01	/* RS232 data available */
#define RXA485	0x04	/* RS485 data available */
#define EEBUSY	0x20	/* EEPROM write in progress */
#define ADCBUSY	0x40	/* ADC conversion in progress */
#define RS232D	0x10
#define RS485D	0x20
#define SERVO_0 0x01
#define SERVO_1 0x02
#define SERVO_2 0x03
#define SERVO_3 0x04
#endif

static int rj_file = 0;
static int rj_errno;

int rj_readbyte(int subreg);
int rj_writebyte(int subreg, int data);
int rj_readword(int subreg);
int rj_writeword(int subreg, int data);

int rj_readbyte_dbg(int subreg, const char *caller);
int rj_writebyte_dbg(int subreg, int data, const char *caller);
int rj_readword_dbg(int subreg, const char *caller);
int rj_writeword_dbg(int subreg, int data, const char *caller);

#define BUFSIZE 64
char version_str[BUFSIZE];
unsigned char rs232_inbuf[BUFSIZE];
unsigned char rs485_inbuf[BUFSIZE];

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

/**********************************************************************
 *
 **********************************************************************/

int rj_readbyte(int subreg);
int rj_readword(int subreg);
int rj_writebyte(int subreg, int data);
int rj_writeword(int subreg, int data);
