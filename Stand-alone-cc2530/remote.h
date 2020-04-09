#ifndef _REMOTE_H_
#define _REMOTE_H_

#define u32 unsigned long
#define u8  unsigned char

extern char is_serial_receive;
extern u8 bufer[7];
void uart0_init(void);
void rf_init(void);
void uart0_sendbuf(char *pbuf , int len);
void InitKey(void);
void timer1_init(void);
void rf_init(void);
void uart0_sendbuf(char *pbuf , int len);

#endif