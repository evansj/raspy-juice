/***********************************************************************
 * Dyna485.c - Slave driver for RS485 with Dynamixel(C)-style packets.
 *
 * Requires usi_rs485.c software UART for AVR USI peripheral.
 *
 * 20100706 Adnan - Initial
 ***********************************************************************/

//#include <util/delay.h>// Don't forget to set CPU freq in proj-config-options
#include "dyna485.h"
#include "juice-dev.h"

//#define msleep(X)_delay_ms((X))

unsigned char node_addr;
unsigned char pkt_state = IDLE;
unsigned char pay_idx, pkt_dst, pkt_len, cksum;
unsigned char payload[PKT_MAXLEN]; // does not contain nodeID and len.
unsigned char outbuf[PKT_MAXLEN];

int dyna_getpkt(void)
{
    unsigned char c;

    while (rj485_havechar()) {
	c = rj485_getc();
	switch(pkt_state) {
	case IDLE:
	    if (c == 0xFF)
		pkt_state = SYNCED1;
	    break;
	case SYNCED1:
	    if (c == 0xFF)
		pkt_state = SYNCED2;// OK, 2nd 0xFF recvd.
	    else
		pkt_state = IDLE;// Oops, got back to IDLE
	    break;
	case SYNCED2:
	    // If 0xFF, treat as extra sync, else it's a node ID
	    if (c != 0xFF) {
		pkt_dst = c;
		pkt_state = SYNCED_ID;
	    }
	    break;
	case SYNCED_ID:
	    pkt_len = c;// Get the len
	    pay_idx = 0;// Init pointer
	    cksum = c + pkt_dst;// Init checksumming
	    pkt_state = PAYLOADING;
	    break;
	case PAYLOADING:
	    payload[pay_idx++] = c;
	    cksum += c;

	    if (pay_idx >= (pkt_len) || pay_idx >= PKT_MAXLEN) {
		pkt_state = FULL_PACKET;
		// 'c' already contains cksum, so perform checksum test
		if (cksum == 0xff)
		    return pay_idx;
		else
		    return -1;
	    }
	    break;
	default:
	    pkt_state = IDLE;
	    break;
	}
    }
    //CODE: 0=no_packet, pay_idx=good_packet, -1=bad_packet
    return 0;
}

void dyna_send(int id, int len, unsigned char *payload)
{
    uint8_t i, cksum;

    rj485_putc(0xFF);
    rj485_putc(0xFF);
    rj485_putc(id);
    rj485_putc(len+1);   // len+2 because incl status and cksum

    // cksum calculation starts from node_addr to last byte of payload
    cksum = id + len;
    for (i = 0; i < len; i++) {
	cksum += payload[i];
	rj485_putc(payload[i]);
    }
    rj485_putc(~cksum);
}

/***********************************************************************
 * Commands and Replies
 ***********************************************************************/

void dyna_ping(int id)
{
    outbuf[0] = DYNA_PING;
    dyna_send(id, 1, outbuf);
}
