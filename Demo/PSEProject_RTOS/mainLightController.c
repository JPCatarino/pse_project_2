/*
 * Paulo Pedreiras, Apr/2020
 *
 * FREERTOS demo for ChipKit MAX32 board
 * - Creates two periodic tasks
 * - One toggles Led LD4, other is a long (interfering)task that 
 *      activates LD5 when executing 
 * - When the interfering task has higher priority interference becomes visible
 *      - LD4 does not blink at the right rate
 *
 * Environment:
 * - MPLAB 5.35
 * - XC32 V2.240
 * - FreeRTOS V10.3.1
 *
 *
 */

/* Standard includes. */
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/attribs.h>


#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* App includes */
#include "../UART/uart.h"
#include "adc_conf.h"
#include "ConfigModules.h"
#include "lightControll.h"

/* Task rates */
#define ACQUIRE_PERIOD_MS 	    ( 100 / portTICK_RATE_MS )
#define UI_PERIOD_MS 	        ( 150  / portTICK_RATE_MS )
#define DECISION_PERIOD_MS 	    ( 200  / portTICK_RATE_MS )
#define ACTUATION_PERIOD_MS 	( 800  / portTICK_RATE_MS )

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define ACQ_PRIORITY	        ( tskIDLE_PRIORITY + 4 )
#define UI_PRIORITY	            ( tskIDLE_PRIORITY + 3 )
#define DECISION_PRIORITY	    ( tskIDLE_PRIORITY + 2 )
#define ACTUATION_PRIORITY	    ( tskIDLE_PRIORITY + 1 )

/* Task Handlers */
static TaskHandle_t AcqHandler = NULL;
static TaskHandle_t UIHandler = NULL;
static TaskHandle_t DecisionHandler = NULL;
static TaskHandle_t ActuationHandler = NULL;

/* Queue*/
#define QUEUE_LENGTH    5
#define ITEM_SIZE       sizeof( int )

QueueHandle_t QueueAcqProc;
QueueHandle_t QueueProcOut;

int LDR_Value;
int number = 0;
int Button = 0;
uint8_t letter = ' ';

button_status OnStatus;
ext_input_status lock;
lightControllState current_state;
terminal_status TerminalStatus;
int Intensity_Stored_Value;
int Intensity_Value;
int Min_Intensity;
int Max_Intensity;
int LDR_Led_On_Value;
int speed;

/*
 * Prototypes and tasks
 */
void AcqTask(void *params)
{
    TickType_t lastticks = xTaskGetTickCount();   

    for(;;) {
        
        IFS1bits.AD1IF = 0;
        AD1CON1bits.ASAM = 1;
        while(IFS1bits.AD1IF == 0);

        LDR_Value = ((ADC1BUF0 * 3300) / 1023);

        //xTaskNotifyGive(ProcHandler); // Notificar

        vTaskDelayUntil(&lastticks, ACQUIRE_PERIOD_MS);
    }
}

void UITask(void *params)
{
    TickType_t lastticks = xTaskGetTickCount();   

    for(;;){
        switch (letter)
        {
            case 'o':
                TerminalStatus = t_On_Off;
                break;
            case 'l':
                lock = (lock + 1) % 2;
                break;
            case '>':
                TerminalStatus = t_More_intensity;
                break;
            case '<':
                TerminalStatus = t_Less_intensity;
                break;
            case 'm':
                current_state = (current_state + 1) % 4;
                break;
            case 'n':
                current_state = (current_state - 1) % 4;
                break;
            case 'w':
                if(number >= Min_Intensity && number <= 100)
                {
                    Max_Intensity = number;
                }
                number = 0;
                break;
            case 's':
                if(number <= Max_Intensity && number >= 0)
                {
                    Min_Intensity = number;
                }
                number = 0;
                break;
            case 'g':
                if(number >= 0)
                {
                    LDR_Led_On_Value = number;
                }
                number = 0;
                break;
            case 't':
                if(number >= 0 && number <= 100)
                {
                    speed = number;
                }
                number = 0;
                break;
            case 'c':   number = 0;             break;
            case '0':   number = number*10 + 0; break;
            case '1':   number = number*10 + 1; break;
            case '2':   number = number*10 + 2; break;
            case '3':   number = number*10 + 3; break;
            case '4':   number = number*10 + 4; break;
            case '5':   number = number*10 + 5; break;
            case '6':   number = number*10 + 6; break;
            case '7':   number = number*10 + 7; break;
            case '8':   number = number*10 + 8; break;
            case '9':   number = number*10 + 9; break;
        }
        letter = ' ';

        switch (Button)
        {
            case 1:
                OnStatus = (OnStatus + 1) % 2;
                break;
            case 2:
                if(lock == Unlocked)
                {
                    current_state = (current_state + 1) % 4;
                }
                break;
            case 3:
                if(lock == Unlocked)
                {
                    current_state = (current_state - 1) % 4;
                }
                break;
        }
        Button = 0;
        vTaskDelayUntil(&lastticks, UI_PERIOD_MS);
    }
}

void DecisionTask(void *params)
{
    TickType_t lastticks = xTaskGetTickCount();   

    for(;;){

        switch (current_state) 
        {
            case ManualMode:

                if((TerminalStatus == t_On_Off) || ((lock == Unlocked) && (OnStatus == b_Pressed)))
                {
                    if(Intensity_Value == 0)
                    {
                        Intensity_Value = 100;
                    }
                    else
                    {
                        Intensity_Value = 0;
                    }
                }     

                OnStatus = b_Wait;
                TerminalStatus = t_Wait;
                break;
            case Dimmer:

                if(Intensity_Value < Min_Intensity)
                {
                    Intensity_Value = Min_Intensity;
                }
                else if(Intensity_Value > Max_Intensity)
                {
                    Intensity_Value = Max_Intensity;
                }

                if((TerminalStatus == t_On_Off) || ((lock == Unlocked) && (OnStatus == b_Pressed)))
                {
                    if(Intensity_Value > Min_Intensity)
                    {
                        Intensity_Stored_Value = Intensity_Value;
                        Intensity_Value = Min_Intensity;
                    }
                    else 
                    {
                        if(Intensity_Stored_Value < Min_Intensity)
                        {
                            Intensity_Value = Min_Intensity; 
                        }
                        else if(Intensity_Stored_Value > Max_Intensity)
                        {
                            Intensity_Value = Max_Intensity; 
                        }
                        else
                        {
                            Intensity_Value =  Intensity_Stored_Value; 
                        }
                    }                                                      
                }                
                else 
                {
                    if((TerminalStatus == t_More_intensity) )
                    {
                        if ((Intensity_Value + 10) <= Max_Intensity)
                        {         
                            Intensity_Value = Intensity_Value + 10;
                            Intensity_Stored_Value = Intensity_Value;
                        }
                    } 
                    if((TerminalStatus == t_Less_intensity))
                    {
                        if ((Intensity_Value - 10) >= Min_Intensity) 
                        {           
                            Intensity_Value = Intensity_Value-10;
                            Intensity_Stored_Value = Intensity_Value;
                        }

                    }      
                }       
                OnStatus = b_Wait;
                TerminalStatus = t_Wait;
                break;
            case AutomaticSwitch:
                printf("Automatic\r\n");
                printf("LDR Value: %d\r\n", LDR_Value);
                printf("On_Val: %d\r\n", LDR_Led_On_Value);
                if(LDR_Value > LDR_Led_On_Value)
                {
                    if(Intensity_Value <= Max_Intensity)
                    {
                        if(Intensity_Value + speed > 100)
                        {
                            Intensity_Value = Max_Intensity;
                        }
                        else
                        {
                            Intensity_Value += speed;  
                        }
                    }
                }
                else
                {
                    if(Intensity_Value >= Min_Intensity)
                    {
                        if(Intensity_Value - speed < 0)
                        {
                            Intensity_Value = Min_Intensity;
                        }
                        else
                        {
                            Intensity_Value -= speed;  
                        }
                    }
                }           
                break;
            case AutomaticAjust:
                printf("matic\r\n");
                printf("LDR Value: %d\r\n", LDR_Value);
                printf("On_Val: %d\r\n", LDR_Led_On_Value);
                if(LDR_Value > LDR_Led_On_Value + 100 && Intensity_Value < Max_Intensity)
                {
                    Intensity_Value++;  
                }
                else if(LDR_Value < LDR_Led_On_Value - 100 && Intensity_Value > Min_Intensity)
                {
                    Intensity_Value--;  
                }              
                break;
        }
        vTaskDelayUntil(&lastticks, DECISION_PERIOD_MS);
    }
}

void ActuationTask(void* params){
    TickType_t lastticks = xTaskGetTickCount();   

    for(;;){
        printf("%d", Intensity_Value);
        OC2RS = Intensity_Value*10;
        vTaskDelayUntil(&lastticks, DECISION_PERIOD_MS);
    }
}


void init_light_controller(void){
    OnStatus = b_Wait;
    lock = Unlocked;
    TerminalStatus = t_Wait;
    Intensity_Value = 0;
    Intensity_Stored_Value = 100;
    Min_Intensity = 0;
    Max_Intensity = 100;
    LDR_Led_On_Value = 2500;
    speed = 1;
    current_state = ManualMode;
}

void __ISR(_EXTERNAL_0_VECTOR,IPL4AUTO) button_on_off_isr(void)
{
    if( Button == 0 ) 
    {
        Button = 1;
    }
    IFS0bits.INT0IF = 0;
}

void __ISR(_EXTERNAL_1_VECTOR,IPL3AUTO) button_plus_isr(void)
{
    if( Button == 0 )
    {
        Button = 2;
    }
    IFS0bits.INT1IF = 0;
}

void __ISR(_EXTERNAL_2_VECTOR,IPL3AUTO) button_less_isr(void)
{
    if( Button == 0 ) 
    {
        Button = 3;
    }
    IFS0bits.INT2IF = 0;
}

void __ISR(_UART_1_VECTOR, IPL5AUTO) uart_isr (void)
{
    if( letter == ' ' ) 
    {
        GetChar(&letter);
    }
    IFS0bits.U1RXIF = 0;    
}

/*
 * Create the demo tasks then start the scheduler.
 */
int mainLightController( void )
{
    
    // Config IO pins
    ConfigIOpins();
    
    // Config timers
    ConfigTimer();  

    // Config Output Compare  
    ConfigOC2();

    // Configure Interrupts
    InitInterruptController();

    // Init ADC
    configureADC1(7);
    adc1_control(1);

    // Init base values
    init_light_controller();

    // Create queue
    QueueAcqProc = xQueueCreate(QUEUE_LENGTH,
                            ITEM_SIZE);
    
    QueueProcOut = xQueueCreate(QUEUE_LENGTH,
                            ITEM_SIZE);

    configASSERT( QueueAcqProc );
    configASSERT( QueueProcOut );

	// Init UART and redirect tdin/stdot/stderr to UART
    if(UartInit(configPERIPHERAL_CLOCK_HZ, 115200) != UART_SUCCESS) {
        PORTAbits.RA3 = 1; // If Led active error initializing UART
        while(1);
    }

     __XC_UART = 1; /* Redirect stdin/stdout/stderr to UART1*/
    
    /* Welcome message*/
    printf("Light Controller \n\r");
          
    /* Create the tasks defined within this file. */
    xTaskCreate( AcqTask, ( const signed char * const ) "Acq", configMINIMAL_STACK_SIZE, NULL, ACQ_PRIORITY, &AcqHandler );
    xTaskCreate( UITask, ( const signed char * const ) "UI", configMINIMAL_STACK_SIZE, NULL, UI_PRIORITY, &UIHandler );
    xTaskCreate( DecisionTask, ( const signed char * const ) "Dec", configMINIMAL_STACK_SIZE, NULL, DECISION_PRIORITY, &DecisionHandler );
    xTaskCreate( ActuationTask, ( const signed char * const ) "Act", configMINIMAL_STACK_SIZE, NULL, DECISION_PRIORITY, &ActuationHandler );

    /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}
