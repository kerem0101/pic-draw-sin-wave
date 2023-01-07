#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <pic18f45k22.h>
#include <htc.h>
#include <pic18.h>
#include "LCD.H"

#define _XTAL_FREQ 40000000
#pragma  config FOSC = HSHP
#pragma config WDTEN = OFF  

static const unsigned char sindata[360] = {128, 130, 132, 134, 136, 139, 141, 143, 145, 147, 150, 152, 154, 156, 158, 160, 163, 165, 167, 169, 171, 173, 175, 177, 179, 181, 183, 185, 187, 189, 191, 193, 195, 197, 199, 201, 202, 204, 206, 208, 209, 211, 213, 214, 216, 218, 219, 221, 222, 224, 225, 227, 228, 229, 231, 232, 233, 234, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 247, 248, 249, 249, 250, 251, 251, 252, 252, 253, 253, 253, 254, 254, 254, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 254, 254, 254, 253, 253, 253, 252, 252, 251, 251, 250, 249, 249, 248, 247, 247, 246, 245, 244, 243, 242, 241, 240, 239, 238, 237, 236, 234, 233, 232, 231, 229, 228, 227, 225, 224, 222, 221, 219, 218, 216, 214, 213, 211, 209, 208, 206, 204, 202, 201, 199, 197, 195, 193, 191, 189, 187, 185, 183, 181, 179, 177, 175, 173, 171, 169, 167, 165, 163, 160, 158, 156, 154, 152, 150, 147, 145, 143, 141, 139, 136, 134, 132, 130, 128, 125, 123, 121, 119, 116, 114, 112, 110, 108, 105, 103, 101, 99, 97, 95, 92, 90, 88, 86, 84, 82, 80, 78, 76, 74, 72, 70, 68, 66, 64, 62, 60, 58, 56, 54, 53, 51, 49, 47, 46, 44, 42, 41, 39, 37, 36, 34, 33, 31, 30, 28, 27, 26, 24, 23, 22, 21, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 8, 7, 6, 6, 5, 4, 4, 3, 3, 2, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 24, 26, 27, 28, 30, 31, 33, 34, 36, 37, 39, 41, 42, 44, 46, 47, 49, 51, 53, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 78, 80, 82, 84, 86, 88, 90, 92, 95, 97, 99, 101, 103, 105, 108, 110, 112, 114, 116, 119, 121, 123, 125};

unsigned int i = 0;

unsigned char line[17] = "                ";

unsigned int Analog[] = {0, 0, 0};
unsigned char value;
unsigned short value_TMR0 = 0x0000;
double new_freq;
unsigned short default_tocon = 0x80;  // T0CON founded for 16 bit and prescale of 1:2
double prescaler = 2.0;              //For [1,20] Hz Prescale always 1:2;

void calc_time_interrupt(double freq_value) {

    freq_value *= 360;

    value_TMR0 = ((10e6) / (prescaler * freq_value));

    value_TMR0 = 65536 - value_TMR0;
    T0CON = default_tocon;
}   

unsigned int gain = 255;
double gain_lcd;
unsigned int offset = 255;
double offset_lcd;
int data;

void main(void) {

    ANSELA = 0x07; //0b0000 0111
    TRISA = 0xF7; //0b1111 0111;
    PORTA = 0x08; //0b0000 1000;
    
    ADCON2 = 0x9A;
    ADCON1 = 0x00;
    ADCON0 = 0x03;

    ANSELD = 0;
    TRISD = 0x00;
    PORTD = 0xFF;

    ANSELB = 0;
    TRISB = 0xC0;
    PORTB = 0;
   
    calc_time_interrupt(1);

    TMR0IE = 1;
    TMR0IP = 1;
    GIE = 1;

    lcd_init();
    lcd_goto(0);
    lcd_puts("SIN GENERATOR");
    
    while (1) {
     
        ADCON0bits.CHS = 0;
        GODONE = 1;
        while (GODONE);
        Analog[0] = (ADRESH << 8) + ADRESL;

        ADCON0bits.CHS = 1;
        GODONE = 1;
        while (GODONE);
        Analog[1] = (ADRESH << 8) + ADRESL;

        ADCON0bits.CHS = 2;
        GODONE = 1;
        while (GODONE);
        Analog[2] = (ADRESH << 8) + ADRESL;

        new_freq = (Analog[2] / 1023.0) * 19 + 1;

        calc_time_interrupt(new_freq);               // Calculates T0CON for new frequency value

        gain = Analog[0] >> 2;

        gain_lcd = (gain * 5.0) / 255.0;

        offset = Analog[1] >> 2;

        offset_lcd = (offset * 5) / 255.0;

        sprintf(line, "Gain(P-P): %4.2fV", gain_lcd );
        lcd_goto(0x40);
        lcd_puts(line);

        sprintf(line, "Offset: %4.2fV", offset_lcd );
        lcd_goto(0x10);
        lcd_puts(line);

        sprintf(line, "Freq: %4.2fHz", new_freq);
        lcd_goto(0x50);
        lcd_puts(line);  
    }

    return;
}

void __interrupt(high_priority) isr() {

    if (TMR0IF) {

        TMR0IF = 0;

        TMR0L = value_TMR0;
        TMR0H = value_TMR0 >> 8;
        
        data = sindata[i];

        data = (data * gain) >> 8;
        data -= gain >> 1;
        data += offset;

        if (data > 255) {
            value = 255;
        } else if (data < 0) {
            value = 0;
        } else {
            value = data;
        }
        
        PORTD = value;
        
        i++;
        
        if (i == 360) {
            i=0;
        }
    }   
}

