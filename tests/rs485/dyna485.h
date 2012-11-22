/***********************************************************************
 * Dyna485.h - Header file for RS485 with Dynamixel(C)-style packets.
 *
 * Requires usi_rs485.c software UART for AVR USI peripheral.
 *
 * 20100706 Adnan - Initial
 ***********************************************************************/

#ifndef __DYNA_485_H
#define __DYNA_485_H

#include <stdint.h>
//#include "usi_rs485.h"

#define EEPROM_NODE_ADDR 0x10
#define BROADCAST_NODEADDR 0xfe

enum pkt_state {
    IDLE = 0,
    SYNCED1,
    SYNCED2,
    SYNCED_ID,
    PAYLOADING,
    FULL_PACKET
};

#define PKT_MAXLEN 64

unsigned char node_addr;
unsigned char pkt_state;
unsigned char pay_idx, pkt_dst, pkt_len, cksum;
unsigned char payload[PKT_MAXLEN]; // does not contain nodeID and len.

extern int dyna_getpkt(void);
extern void dyna_reply(uint8_t status, uint8_t len, uint8_t *payload);

#define dyna_reset()   (pkt_state = IDLE)
#define dyna_stat(ERR) dyna_reply((ERR), 0, 0)

#define DYNA_PING 0x01
#define DYNA_READ 0x02
#define DYNA_WRITE 0x03
#define DYNA_REG_WR 0x04
#define DYNA_ACTION 0x05
#define DYNA_RESET 0x06
#define DYNA_SYN_WR 0x83

#define DYNA_ERR_INSTRUCTION 0b01000000
#define DYNA_ERR_OVERLOAD 0b00100000
#define DYNA_ERR_CHECKSUM 0b00010000
#define DYNA_ERR_RANGE 0b00001000
#define DYNA_ERR_OVERHEAT 0b00000100
#define DYNA_ERR_ANGLE 0b00000010
#define DYNA_ERR_VOLTAGE 0b00000001


#endif //__DYNA_485_H
