#include <xc.h>

#pragma config FOSC = XT
#pragma config WDTE = OFF
#pragma config PWRTE = ON
#pragma config BOREN = ON
#pragma config LVP = OFF
#pragma config CPD = OFF
#pragma config WRT = OFF
#pragma config CP = OFF

unsigned char myTMR0IF = 0; // we use for delay overflow

void delay(int count)
{
    int of_count = 0;

    while (of_count < count)
    {
        if (myTMR0IF)
        {
            of_count++;
            myTMR0IF = 0;
        }
    }
}

void interrupt ISR(void)
{
    int period = 0;
    GIE = 0; // disable all unmasked interrupts (INTCON reg)

    if (CCP1IF == 1) // checks CCP1 interrupt flag
    {
        CCP1IF = 0;            // clears interrupt flag
        TMR1 = 0;              // resets TMR1
        period = CCPR1 / 1000; // transfers captured TMR1 value
                               // normalize the value (make the number smaller)
        period = period * 8;   // multiply by the normalized TMR1 timeout

        // led
        RA0 ^= 1;
    }

    GIE = 1; // enable all unmasked interrupts (INTCON reg)
}

void instCtrl(unsigned char data)
{
    PORTD = data; // load data to port D since my port c is the output pins
    RB5 = 0;      // set RS to 0 (instruction reg)
    RB6 = 0;      // set RW to 0 (write)
    RB7 = 1;      // set E to 1
    delay(5);
    RB7 = 0; // set E to 0 (strobe)
}

void initLCD()
{
    delay(61);

    instCtrl(0x3C); // function set: 8-bit; dual-line
    instCtrl(0x38); // display off
    instCtrl(0x01); // display clear
    instCtrl(0x06); // entry mode: increment; shift off
    instCtrl(0x0E); // display on; cursor off; blink off
}

void dataCtrl(unsigned char data)
{
    PORTD = data; // load data to PORT D since its our output
    RB5 = 1;      // set RS to 1 (data reg)
    RB6 = 0;      // set RW to 0 (write)
    RB7 = 1;      // set E to 1
    delay(5);
    RB7 = 0; // set E to 0 (strobe)
}

void main(void)
{
    // RA0 and LED output
    ADCON1 = 0x06; // set port a digital i/o
    TRISA = 0x00;  // set port a to output
    RA0 = 0;       // initial value to low (LED off)

    // LCD config
    TRISD = 0x00; // set ports d as output
    TRISB = 0x01; // RB0 input for button, rest are output

    // ccp - capture module
    TRISC = 0x04;   // set RC2 to input
    T1CON = 0x30;   // 1:8 prescaler, Timer1 off
    CCP1CON = 0x05; // capture mode: every rising edge (0101)
    CCP1IE = 1;     // enable TMR1/CCP1 match interrupt (PIE1 reg)
    CCP1IF = 0;     // reset interrupt flag (PIR1 reg)
    PEIE = 1;       // enable all peripheral interrupt (INTCON reg)
    GIE = 1;        // enable all unmasked interrupts (INTCON reg)
    TMR1ON = 1;     // Turns on Timer1 (T1CON reg)

    initLCD();      // initialize lcd instructions
    instCtrl(0x02); // set cursor

    while (1)
    {
        delay(61);
        dataCtrl('P');
        dataCtrl('E');
        dataCtrl('R');
        dataCtrl('I');
        dataCtrl('O');
        dataCtrl('D');
        dataCtrl(':');
    }
}