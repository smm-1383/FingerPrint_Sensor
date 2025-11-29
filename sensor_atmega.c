#include <mega32.h>
#include <delay.h>
#include <stdlib.h>


// UART
void uart_init(void)
{
    UBRRH = 0;
    UBRRL = 51; // 9600 @ 8MHz

    UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);  // RX interrupt
    UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0); // 8N1
}

void uart_putc(char c)
{
    while(!(UCSRA&(1<<UDRE)));
    UDR = c;
}

void uart_puts(char *s)
{
    while(*s) uart_putc(*s++);
}

void uart_put_uint8(unsigned char v)
{
    char b[4];
    int i=0,j;

    if(v==0){ uart_putc('0'); return; }

    while(v>0){
        b[i++] = (v%10)+'0';
        v/=10;
    }
    for(j=i-1;j>=0;j--) uart_putc(b[j]);
}

// EEPROM
eeprom unsigned char magic;
eeprom unsigned char id_pattern[128]; // id: pattern map

void init_eeprom(void)
{
    unsigned char i;

    if(magic!=0x55){
        for(i=0;i<128;i++) id_pattern[i]=0xFF;
        magic=0x55;
    }
}

// Logic Sensor
unsigned char finger_present(void){
    return (PINA & (1<<7))?1:0; // PA7 = finger detect, is there a fniger on me?
}

unsigned char read_pattern(void){
    unsigned char p=0, i;
    for(i=0;i<7;i++)
        if(PINA&(1<<i)) p|=(1<<i);  // PA0..PA6 = pattern bits, finger's features
    return p; // 0..127
}

// Commands

void cmd_scan(void)
{
    unsigned char p, id;
    unsigned char found = 0;

    if(!finger_present()){
        uart_puts("NOF\r\n");
        return;
    }

    p = read_pattern();

    for(id=0; id<128; id++){
        if(id_pattern[id] == p){
            found = 1;
            break;
        }
    }

    if(!found){
        uart_puts("FAIL\r\n");
    } else {
        uart_puts("OK:");
        uart_put_uint8(id);
        uart_puts("\r\n");
    }
}

void cmd_enroll(unsigned char id)
{
    unsigned char p, old;

    if(!finger_present()){
        uart_puts("NOF\r\n");
        return;
    }

    if(id > 127) id = 127;
    p   = read_pattern();
    old = id_pattern[id];

    if(old == p){
        uart_puts("ENEX\r\n");
        return;
    }

    id_pattern[id] = p;
    uart_puts("ENOK\r\n");
}

void cmd_delete(unsigned char id)
{
    if(id > 127) id = 127;

    if(id_pattern[id] == 0xFF){
        uart_puts("DNEX\r\n");  // Do Not Exist
    } else {
        id_pattern[id] = 0xFF;
        uart_puts("DELOK\r\n");
    }
}

// Command buffer (ISR)
#define CMDLEN 16
volatile char buf[CMDLEN];
volatile char idx=0;
volatile char ready=0;

interrupt [USART_RXC] void rx_isr(void)
{
    char c = UDR;

    if(c=='\r' || c=='\n'){
        if(idx>0){
            buf[idx]=0;
            ready=1;
        }
        idx=0;
    } else {
        if(idx<CMDLEN-1){
            buf[idx++]=c;
        } else idx=0;
    }
}

// MAIN
void main(void)
{
    DDRA=0x00;
    PORTA=0xFF;

    uart_init();
    init_eeprom();
    delay_ms(100);
    uart_puts("READY\r\n");

    #asm("sei")

    while(1){
        if(ready){
            ready=0;

            // Scan
            if(buf[0]=='S' && buf[1]==0){
                cmd_scan();
            }
            // Enroll: E<num>
            else if(buf[0]=='E'){
                unsigned int v = atoi(&buf[1]);
                if(v>127) v=127;
                cmd_enroll((unsigned char)v);
            }
            // Delete: D<num>
            else if(buf[0]=='D'){
                unsigned int v = atoi(&buf[1]);
                if(v>127) v=127;
                cmd_delete((unsigned char)v);
            }
            // else: nothing Yet
        }
    }
}
