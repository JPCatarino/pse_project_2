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
#include <string.h>
#include <math.h>

#include <xc.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"


/* App includes */
#include "../UART/uart.h"
#include "adc_conf.h"

/* The rate at which the LED LD5 should flash */
#define ACQUIRE_PERIOD_MS 	 ( 100 / portTICK_RATE_MS )

/* Priorities of the demo application tasks (high numb. -> high prio.) */
#define ACQ_PRIORITY	    ( tskIDLE_PRIORITY + 4 )
#define PROC_PRIORITY	    ( tskIDLE_PRIORITY + 2 )
#define OUT_PRIORITY	    ( tskIDLE_PRIORITY + 1 )

/* Task Handlers */
static TaskHandle_t AcqHandler = NULL;
static TaskHandle_t ProcHandler = NULL;
static TaskHandle_t OutHandler = NULL;

/* Queue*/
#define QUEUE_LENGTH    5
#define ITEM_SIZE       sizeof( int )

QueueHandle_t QueueAcqProc;
QueueHandle_t QueueProcOut;

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

        int tmp = ((ADC1BUF0 * 100) / 1023);

        xQueueSend( QueueAcqProc, &tmp,  10 );

        xTaskNotifyGive(ProcHandler); // Notificar

        vTaskDelayUntil(&lastticks, ACQUIRE_PERIOD_MS);
    }
}

void ProcTask(void *params)
{
    int notificationValue;
    int flag = 1;
    int pos = 0;
    int value[5] = {0,0,0,0,0}; 
    
    for(;;){
        notificationValue = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if( notificationValue > 0){
            int i;
            int sum = 0;
            int tmp;
            int array_length = sizeof(value)/sizeof(value[0]);  

            
            xQueueReceive(QueueAcqProc, &tmp, 10);
            
            value[pos] = tmp;
            pos = (pos+1) % 5;
            
            if(flag == 1 && pos == 4) flag = 0;

            if(flag == 0){
                for(i = 0; i < array_length; i++){
                sum += value[i];
                }

                float avg = floor((sum / QUEUE_LENGTH) * 10) / 10;
                
                xQueueSend( QueueProcOut, &avg,  10 );

                xTaskNotifyGive(OutHandler);
            }
        }
    }
}

void OutTask(void *params)
{
    int notificationValue;
    char result[20];
    float avg;
    for(;;){
        notificationValue = ulTaskNotifyTake(pdTRUE, (TickType_t) portMAX_DELAY);
        if(notificationValue > 0 ){
            xQueueReceive(QueueProcOut, &avg, 10);
            sprintf(result, "The average is: %.1f\n\r", avg);
            PrintStr(result);
        }
    }
}


/*
 * Create the demo tasks then start the scheduler.
 */
int mainSensorDataCollect( void )
{
    // Set AN0 as input
    TRISBbits.TRISB0 = 1;

    // Set RA3 (LD4) as output
    TRISAbits.TRISA3 = 0;
    PORTAbits.RA3 = 0;

    // Init ADC
    configureADC1(7);
    adc1_control(1);

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
    printf("Starting SETR FreeRTOS Demo - Sensor Data Collect\n\r");
    
      
    /* Create the tasks defined within this file. */
    xTaskCreate( AcqTask, ( const signed char * const ) "Acq", configMINIMAL_STACK_SIZE, NULL, ACQ_PRIORITY, &AcqHandler );
    xTaskCreate( ProcTask, ( const signed char * const ) "Proc", configMINIMAL_STACK_SIZE, NULL, PROC_PRIORITY, &ProcHandler );
    xTaskCreate( OutTask, ( const signed char * const ) "Out", configMINIMAL_STACK_SIZE, NULL, OUT_PRIORITY, &OutHandler );

    /* Finally start the scheduler. */
	vTaskStartScheduler();

	/* Will only reach here if there is insufficient heap available to start
	the scheduler. */
	return 0;
}
