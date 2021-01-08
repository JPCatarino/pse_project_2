#include <xc.h>
#include "ConfigModules.h"

void ConfigIOpins(void)
{
    // Set pins input and output
    TRISE = BUTTON_RE8_MASK | BUTTON_RE9_MASK;
    TRISD = BUTTON_RD0_MASK;
    TRISB = LDR_RB0_MASK;    
 
    // Turn off the led
    LED_CLR();
}

void ConfigOC2(void)
{
    OC2CONbits.ON = 0;                    
    OC2CONbits.OCM = 6;                   
    OC2CONbits.OCTSEL = 0;
    OC2RS = 100;
    OC2CONbits.ON = 1;
}

void ConfigTimer(void)
{
    // Timer2 to OC
    T2CONbits.ON = 0;   
    T2CONbits.T32 = 0;
    T2CONbits.TCS = 0;
    T2CONbits.TCKPS = 0;
    PR2 = PS_OC_VALUE;      
    TMR2 = 0;
    T2CONbits.ON = 1;
    
}

void InitInterruptController(void)
{
    // On/Off Button  
    IEC0bits.INT0IE=0;
    IPC0bits.INT0IP=4;
    INTCONbits.INT0EP = 0;
    IFS0bits.INT0IF=0;
    IEC0bits.INT0IE=1;
        
    // + Button  
    IEC0bits.INT1IE=0;
    IPC1bits.INT1IP=3;
    INTCONbits.INT1EP = 0;
    IFS0bits.INT1IF=0;
    IEC0bits.INT1IE=1;

    // - Button  
    IEC0bits.INT2IE=0;
    IPC2bits.INT2IP=3;
    INTCONbits.INT2EP = 0;
    IFS0bits.INT2IF=0;
    IEC0bits.INT2IE=1;
    
    // UART
    IEC0bits.U1RXIE = 0;
    IPC6bits.U1IP = 5;
    IFS0bits.U1RXIF = 0;        
    IEC0bits.U1RXIE = 1;

    INTCONSET=_INTCON_MVEC_MASK;
}