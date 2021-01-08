#include <xc.h>
#include "ConfigModules.h"
#include "../UART/uart.h"


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
    OC2RS = 0;
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

void print_string(char* message){
    int i=0;
  
    for(i=0;message[i]!=0;i++){
        PutChar(message[i]);
    }
}

void print_current_state(uint8_t currentState){
    switch(currentState){
        case 0: ;
            print_string("\r\nManual Mode\r\n");
            break;
        case 1: ;
            print_string("\r\nDimmer Mode\r\n");
            break;
        case 2: ;
            print_string("\r\nAutomatic Switch Mode\r\n");
            break;
        case 3: ;
            print_string("\r\nAutomatic Adjust Mode\r\n");
            break;            
    }
}

void print_help(void){
    print_string("\nButton (ON/OFF):\tpin3\r");
    print_string("\nButton (+ mode):\tpin2\r");
    print_string("\nButton (- mode):\tpin7\r");
    print_string("\n\nTerminal:\r\n\to (ON/OFF)\r\n\t> (More_intensity)\r\n\t< (Less_intensity)\r\n\tl (lock)\r\n\tm (+ mode)\r\n\tn (- mode)");
    print_string("\n\n\r\tw (Set_max_intensity)\r\n\ts (Set_min_intensity)\r\n\tg (Set_LDR_on_value)\r\n\tm (mode)\r\n\tt (speed)");

}