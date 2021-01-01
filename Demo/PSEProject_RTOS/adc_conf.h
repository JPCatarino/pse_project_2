/* 
 * File:   adc_conf.h
 * Author: jorpc
 *
 * Created on 4 de Dezembro de 2020, 3:42
 */

#ifndef ADC_CONF_H
#define	ADC_CONF_H

#ifdef	__cplusplus
extern "C" {
#endif

    void configureADC1(int mode);
    void adc1_control(uint8_t trun);


#ifdef	__cplusplus
}
#endif

#endif	/* ADC_CONF_H */

