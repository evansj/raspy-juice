#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <linux/fcntl.h>

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

int file;
int rj_readstat(void);
int rj232_getc(void);
int rj485_getc(void);
int rj232_read(void);
int rj485_read(void);
int rj232_send(unsigned char *buf, int len);
int rj485_send(unsigned char *buf, int len);
int rj_setservo(int chan, int usec);


#define BUFSIZE 64
char inbuf[BUFSIZE];
char outbuf[BUFSIZE];

int main(int argc, char *argv[])
{
    int i;
    int count = 0;
    int adapter_nr = 0; /* probably dynamically determined */
    char devname[20];
    int addr = 0x48; /* The I2C address */
    unsigned char reg = 0x00; /* Device register to access */
    int rval, stat, c, b;
    char buf[10];
    int servo0pwm = 1500, servo0spddir = 20;
    int servo1pwm = 1500, servo1spddir = 20;
    int servo2pwm = 1500, servo2spddir = 20;
    int servo3pwm = 1500, servo3spddir = 20;
    
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


#define SERVO_TEST 1    
#ifdef SERVO_TEST
    rj_setservo(SERVO_0, servo0pwm);
    usleep(500000);
    rj_setservo(SERVO_1, servo1pwm);
    usleep(500000);
    rj_setservo(SERVO_2, servo2pwm);
    usleep(500000);
    rj_setservo(SERVO_3, servo3pwm);
    usleep(500000);
#endif


    while (1) {

	usleep(100000);
	count++;

	if ((count % 5) == 0) {
	    printf("\33[2K\r%06d:", count);
	    stat = rj_readstat();
	    for (b = 7; b >= 0; b--)
		printf("%c", (stat & (1 << b)) ? '1' : '0');
	    printf(": ");
	    fflush(stdout);
	    
	    if (stat >= 0) {
		if (stat & ADCBUSY)
		    printf("adcbusy:");
		
		if (stat & EEBUSY)
		    printf("eebusy:");
		
		if (stat & RXA485) {
		    rval = rj485_read();
		    printf("\nrs485: %s:\n", inbuf);
		    for (i = 0; inbuf[i] != 0; i++)
			printf("0x%02x ", inbuf[i]);
		    printf("\n");
		    
		}
		
		if (stat & RXA232) {
		    rval = rj232_read();
		    printf("\nrs232: %s: \n", inbuf);
		    for (i = 0; inbuf[i] != 0; i++)
			printf("0x%02x ", inbuf[i]);
		    printf("\n");
		}
		printf("");
		
	    }
	}

	if ((count % 10) == 0) {
	    sprintf(outbuf, "Hello 232!!! count = %d\n\r", count);
	    rj232_send(outbuf, strlen(outbuf));
	    sprintf(outbuf, "Hello 485!!! count = %d\n\r", count); 
	    rj485_send(outbuf, strlen(outbuf));
	}
	

#if SERVO_TEST
	servo0pwm += servo0spddir;
	if ((servo0pwm > 2000) || (servo0pwm < 1000))
	    servo0spddir = -servo0spddir;
	rj_setservo(SERVO_0, servo0pwm);

	servo1pwm += servo1spddir;
	if ((servo1pwm > 2000) || (servo1pwm < 1000))
	    servo1spddir = -servo1spddir;
	rj_setservo(SERVO_1, servo1pwm);

	servo2pwm += servo2spddir;
	if ((servo2pwm > 2000) || (servo2pwm < 1000))
	    servo2spddir = -servo2spddir;
	rj_setservo(SERVO_2, servo2pwm);

	servo3pwm += servo3spddir;
	if ((servo3pwm > 2000) || (servo3pwm < 1000))
	    servo3spddir = -servo3spddir;
	rj_setservo(SERVO_3, servo3pwm);
#endif

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
