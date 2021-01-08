#include <xc.h>
#include "adc_conf.h"

    void configureADC1(int mode){
        AD1CON1bits.SSRC = mode;
        AD1CON1bits.CLRASAM = 1;

        AD1CON1bits.FORM = 0;
        AD1CON2bits.VCFG = 0;
        AD1CON2bits.SMPI = 0;
        AD1CON3bits.ADRC = 1;
        AD1CON3bits.SAMC = 16;   
        AD1CHSbits.CH0SA = 0;
        AD1PCFGbits.PCFG0 = 0;

    }
    
    
    void adc1_control(uint8_t trun){
        if(trun)
        {
            AD1CON1bits.ON = 1;
        }
        else
        {
            AD1CON1bits.ON = 0;
        }   
    }