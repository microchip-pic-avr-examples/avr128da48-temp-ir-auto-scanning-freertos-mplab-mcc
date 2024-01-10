/*
� [2022] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/ 

#include "mcc_generated_files/system/system.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "semphr.h"
#include "partest.h"
#include "serial.h"

#define UART_STRING_SIZE                    255
#define ADC0_OUTSIDE_WINDOW_FLG           (ADC0.INTFLAGS&ADC_WCMP_bm)

#define tempDIFF_CHECK_FREQ                 pdMS_TO_TICKS(800)
#define doorOPEN_INTERVAL                   pdMS_TO_TICKS(3650)

#define TempDisplayQueue_LENGTH             2
#define UartPrintQueue_LENGTH               3

#define OledSetup_PRIORITY                ( tskIDLE_PRIORITY + 3 )
#define OledDisplay_PRIORITY              ( tskIDLE_PRIORITY + 2 )
#define TemperatureMeas_PRIORITY          ( tskIDLE_PRIORITY + 1 )
#define MotionCheck_PRIORITY              ( tskIDLE_PRIORITY + 1 )
#define DoorOpen_PRIORITY                 ( tskIDLE_PRIORITY + 1 )
#define UartPrint_PRIORITY                ( tskIDLE_PRIORITY + 1 )

#define TEMP_MEASURE_NOTIF_INDEX            0
#define DOOR_CLOSED_NOTIF_INDEX             0x2
#define DOOR_OPEN_NOTIF_INDEX               0x8

enum enumPrintLUT{
    motion,
    detected,
    stepIn,
    frontOf,
    theRound,
    sensor,
    measuring,
    stayStill,
    TempOut,
    ofRange,
    access,
    denied,
    measured,
    tempBiggerThan37,
    tempIs,
    normal,
    door,
    opened,
    closed,
    noPerson
            
};


/*      *** Type Definitions ***     */
typedef struct OledPrintArgs
{
    char FirstString[16];
    char SecondString[16];
    double TemperatureArg;
    bool isTempValid;
    
}OledPrintArgs_t;

typedef struct TempObj
{
    double tempAmbient,tempObject,tempMax;
}TempObj_t;


/*      *** Handlers ***     */
static TaskHandle_t TempMeasHandler=    NULL;
static TaskHandle_t MotionCheckHandler= NULL;
static QueueHandle_t TempDisplayQueue=  NULL;
static QueueHandle_t UartPrintQueue=    NULL;
static TimerHandle_t TimeoutTimerHandler=NULL;

/*      *** Functions  ***     */

void        timeoutTimerExpired();
void        setArgsToSend   (const char* stringArg1,const char* stringArg2, double Temp, bool validity,OledPrintArgs_t *ArgToModify);
static void UartPrintTask   (void *pvParameters);
static void OledDisplayTask (void *pvParameters);
static void TempMeasTask    (void *pvParameters);
static void MotionCheckTask (void *pvParameters);
static void OledSetupTask   (void *pvParameters);

void FreeRTOS_Initialize(void){

    FreeRTOS_AppStart();

    /* If all is well, the FreeRTOS user application will now be running, and the following
    line will never be reached */
    for( ;; );
}

void  FreeRTOS_AppStart(void)
{
    
    TempDisplayQueue =  xQueueCreate( TempDisplayQueue_LENGTH, sizeof( OledPrintArgs_t ) );
    UartPrintQueue  =   xQueueCreate(UartPrintQueue_LENGTH, UART_STRING_SIZE*sizeof(char));
    TimeoutTimerHandler=xTimerCreate
                 ( "Timeout Timer",         //Timer name
                   pdMS_TO_TICKS(4000),     //Timeout in ticks
                   pdTRUE,                  //auto-reload setting (pdTRUE or pdFALSE)
                   NULL,                    //Timer ID
                   timeoutTimerExpired );   //Timer Expire Callback Function
    
    if( TempDisplayQueue != NULL && UartPrintQueue!=NULL && TimeoutTimerHandler!=NULL )
    {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate( OledSetupTask,                     // The function that implements the task.
                    "Oled setup Tsk",                   // The text name assigned to the task - for debug only as it is not used by the kernel. 
                    2*configMINIMAL_STACK_SIZE,         // The size of the stack to allocate to the task. 
                    NULL,                               // The parameter passed to the task - not used in this case. 
                    OledSetup_PRIORITY,                 // The priority assigned to the task. 
                    NULL );                             // The task handle is not required, so NULL is passed. 
        
        xTaskCreate( OledDisplayTask,                   // The function that implements the task.
                    "Oled Disp Tsk",                    // The text name assigned to the task - for debug only as it is not used by the kernel. 
                    5*configMINIMAL_STACK_SIZE,         // The size of the stack to allocate to the task. 
                    NULL,                               // The parameter passed to the task - not used in this case. 
                    OledDisplay_PRIORITY,               // The priority assigned to the task. 
                    NULL );                             // The task handle is not required, so NULL is passed. 
        
        xTaskCreate( UartPrintTask,                     // The function that implements the task.
                    "Uart Task",                        // The text name assigned to the task - for debug only as it is not used by the kernel. 
                    2*configMINIMAL_STACK_SIZE,         // The size of the stack to allocate to the task. 
                    NULL,                               // The parameter passed to the task - not used in this case. 
                    UartPrint_PRIORITY,                 // The priority assigned to the task. 
                    NULL );                             // The task handle is not required, so NULL is passed. 
        
        xTaskCreate( TempMeasTask,                      // The function that implements the task. */
                    "Temp Meas Task",                   // The text name assigned to the task - for debug only as it is not used by the kernel. */
                    5*configMINIMAL_STACK_SIZE,         // The size of the stack to allocate to the task. */
                    NULL,                               // The parameter passed to the task - not used in this case. */
                    TemperatureMeas_PRIORITY,           // The priority assigned to the task. */
                    &TempMeasHandler );                 // Task Handler      
        
         xTaskCreate( MotionCheckTask,                  // The function that implements the task. */
                    "Motion Task",                      // The text name assigned to the task - for debug only as it is not used by the kernel. */
                    2*configMINIMAL_STACK_SIZE,         // The size of the stack to allocate to the task. */
                    NULL,                               // The parameter passed to the task - not used in this case. */
                    MotionCheck_PRIORITY,               // The priority assigned to the task. */
                    &MotionCheckHandler );              // Task Handler       
         
        
         
        vTaskSuspend(TempMeasHandler);  
        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }
 
     /* If all is well, the scheduler will now be running, and the following
    line will never be reached.  If the following line does execute, then
    there was either insufficient FreeRTOS heap memory available for the idle
    and/or timer tasks to be created, or vTaskStartScheduler() was called from
    User mode.  See the memory management section on the FreeRTOS web site for
    more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
    mode from which main() is called is set in the C start up code and must be
    a privileged mode (not user mode). */

    for( ;; );
}

void setArgsToSend(const char* stringArg1,const char* stringArg2, double Temp, bool validity,OledPrintArgs_t *ArgToModify)
{

    if(strlen(stringArg1)<16)
        strcpy(ArgToModify->FirstString,stringArg1);
    
    if(strlen(stringArg2)<16)
        strcpy(ArgToModify->SecondString,stringArg2);
    
    ArgToModify->TemperatureArg=Temp;
    ArgToModify->isTempValid=validity;
    
}

static void UartPrintTask(void *pvParameters)
{
    static char pReceivedStr[UART_STRING_SIZE];
    for(;;)
    {
        xQueueReceive( UartPrintQueue, &pReceivedStr, portMAX_DELAY );
        vSerialPutString( NULL, (signed char*) pReceivedStr, strlen(pReceivedStr) );
        xSerialPutChar( NULL, '\r', pdMS_TO_TICKS(1) );
        xSerialPutChar( NULL, '\n', pdMS_TO_TICKS(1) );
        pReceivedStr[0]='\0'; 
    }
}



/* TO DO: SPI transfer works, but only transfers garbage data,
 *        Debug further into SPI driver.
 * TEMP FIX: Used previous bare-metal drivers.
 */
static void OledDisplayTask(void *pvParameters)
{
    static OledPrintArgs_t ReceivedArg;
    static char temp_string[16];
    
    for(;;)
    {
            xQueueReceive( TempDisplayQueue, &ReceivedArg, portMAX_DELAY );
            vTaskDelay(pdMS_TO_TICKS(500));
            oledC_clearScreen();
            xQueueSend(UartPrintQueue,&ReceivedArg.FirstString,0U);
            xQueueSend(UartPrintQueue,&ReceivedArg.SecondString,0U);

            strcpy(temp_string, ReceivedArg.FirstString);
            oledC_printString(temp_string,OLEDC_COLOR_WHITE,0);

            strcpy(temp_string, ReceivedArg.SecondString);
            oledC_printString(temp_string,OLEDC_COLOR_WHITE,1);

            if(ReceivedArg.TemperatureArg > TEMP_ABSOLUTE_ZERO)
            {
                vTaskDelay(pdMS_TO_TICKS(500));
                if(ReceivedArg.TemperatureArg<=(double)999.99)
                {
                    sprintf(
                            temp_string,"%d.%1u C", 
                            (int) ReceivedArg.TemperatureArg, 
                            (int) ((ReceivedArg.TemperatureArg - (int) ReceivedArg.TemperatureArg ) * 10)
                            );
                    
                    xQueueSend(UartPrintQueue,&temp_string,0U);
                    if (ReceivedArg.isTempValid)
                        oledC_printString(temp_string,OLEDC_COLOR_GREEN,2);
                    else
                        oledC_printString(temp_string,OLEDC_COLOR_RED,2);
                }
                else
                {
                    sprintf(temp_string,"Invalid");
                    oledC_printString(temp_string,OLEDC_COLOR_RED,2);
                }
            }
            
        vTaskDelay(pdMS_TO_TICKS(700));
        if(strcmp(ReceivedArg.SecondString,"closed")==0)
            xTaskNotifyGiveIndexed(TempMeasHandler,DOOR_CLOSED_NOTIF_INDEX);
        if(strcmp(ReceivedArg.SecondString,"opened")==0)
            xTaskNotifyGiveIndexed(TempMeasHandler,DOOR_OPEN_NOTIF_INDEX);
        oledC_clearScreen();
        
        
    }
}

static void OledSetupTask(void *pvParameters)
{
    for(;;)
    {
        taskENTER_CRITICAL();
        oledC_setup();
        oledC_clearScreen();
        taskEXIT_CRITICAL();
        vTaskDelete(NULL); /*Delete Current task as we will never need it again after start-up*/
    }
}
static void MotionCheckTask(void *pvParameters)
{
    ADC0_StartConversion(ADC_MUXPOS_AIN4_gc);
    for(;;)
    {
        if(ADC0_OUTSIDE_WINDOW_FLG)
            {
                ADC0.INTFLAGS = ADC_WCMP_bm;
                ADC0_StopConversion();
                vTaskResume(TempMeasHandler);
                vTaskSuspend(NULL);                 //Do not check for motion while temp is being measured.
            }
       
    }    
}
static void TempMeasTask(void * pvParameters)
{
     static TempObj_t measuredTemp={
        .tempAmbient=0,
        .tempMax=0,
        .tempObject=0
    };
     
    static char const *PrintLUT[]={
         "Motion",      "detected",
         "Step in",     "front of",
         "the round",   "sensor",
         "Measuring",   "Stay still",
         "Temp out",    "of range:",
         "Access",      "denied",
         "Measured",    "Temp>37:",
         "Temp is",     "normal:",
         "Door",        "opened",
         "closed",      "No person"
         
     };
    static bool tempDiffOkFlg=false;
    static OledPrintArgs_t argsToSend;
    
    for(;;)
    {
        
        if(TimeoutTimerHandler!=NULL)
        {
            if( xTimerStart( TimeoutTimerHandler, 0 ) != pdPASS )
             {
                 /* The timer could not be set into the Active
                 state. */
             }
            else
            {
                setArgsToSend(PrintLUT[motion], PrintLUT[detected], TEMP_HIDDEN, TEMP_INVALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                
                setArgsToSend(PrintLUT[stepIn], PrintLUT[frontOf], TEMP_HIDDEN, TEMP_INVALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                
                setArgsToSend(PrintLUT[theRound], PrintLUT[sensor], TEMP_HIDDEN, TEMP_INVALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                
                xTimerStart(TimeoutTimerHandler,0U);
                while(  ulTaskNotifyTakeIndexed( TEMP_MEASURE_NOTIF_INDEX,pdTRUE,0 )!=1 )
                {
                    /* Do not let the temperature measurement be interrupted
                     * as it may lead to corrupted data, thus we enter critical section */
                    taskENTER_CRITICAL();
                    measuredTemp.tempAmbient = Temperature_ReadAmbient();
                    measuredTemp.tempObject = Temperature_ReadObject();
                    taskEXIT_CRITICAL();
                    
                    if ( (measuredTemp.tempObject-measuredTemp.tempAmbient) > TEMP_DIFF)
                        tempDiffOkFlg=true;
                    
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                xTimerStop(TimeoutTimerHandler,0U);
                
            }
        }
        if(tempDiffOkFlg==true)
        {
            setArgsToSend(PrintLUT[measuring], PrintLUT[stayStill], TEMP_HIDDEN, TEMP_INVALID,&argsToSend);
            xQueueSend( TempDisplayQueue, &argsToSend, 0U );

            //vTaskDelay(pdMS_TO_TICKS(1000));

            /* Do not let the temperature measurement be interrupted
             * as it may lead to corrupted data, thus we enter critical section */
            taskENTER_CRITICAL();
            measuredTemp.tempMax = Temperature_MaxObj(REPEATED_MEAS)+TEMP_MEASURE_CORRECTION;
                    //Sensor gives consistently low numbers, giving an offset of a few degrees C here
            taskEXIT_CRITICAL();

            if ((measuredTemp.tempMax > TEMP_HIGH) || (measuredTemp.tempMax < TEMP_LOW)) 
            {
              setArgsToSend(PrintLUT[TempOut], PrintLUT[ofRange], measuredTemp.tempMax, TEMP_INVALID,&argsToSend);
              xQueueSend( TempDisplayQueue, &argsToSend, 0U );
             // vTaskDelay(pdMS_TO_TICKS(500));
              setArgsToSend(PrintLUT[access], PrintLUT[denied], measuredTemp.tempMax, TEMP_INVALID,&argsToSend);
              xQueueSend( TempDisplayQueue, &argsToSend, 0U );
              //Buzzer_Sound(BEEP_ERROR);
              //vTaskDelay(pdMS_TO_TICKS(2000));
            } 
            else if (measuredTemp.tempMax > TEMP_GOOD) 
            {
                setArgsToSend(PrintLUT[measured], PrintLUT[tempBiggerThan37], measuredTemp.tempMax, TEMP_INVALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
               // vTaskDelay(pdMS_TO_TICKS(500));
                setArgsToSend(PrintLUT[access], PrintLUT[denied], measuredTemp.tempMax, TEMP_INVALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                //Buzzer_Sound(BEEP_DENIED);
                vTaskDelay(pdMS_TO_TICKS(2000));
            } 
            else 
            {
                setArgsToSend(PrintLUT[tempIs], PrintLUT[normal], measuredTemp.tempMax, TEMP_VALID,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                setArgsToSend(PrintLUT[door],PrintLUT[opened], TEMP_HIDDEN, 0,&argsToSend); //Send the open door display command
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                if( xTaskNotifyWaitIndexed(DOOR_OPEN_NOTIF_INDEX,0xffffffff, 0xffffffff,NULL, portMAX_DELAY)==1);//wait for the print to be finished and then open the door
                    LED_SetHigh(); //Door Opened
                vTaskDelay(doorOPEN_INTERVAL);
                setArgsToSend(PrintLUT[door], PrintLUT[closed], TEMP_HIDDEN, 0,&argsToSend);
                xQueueSend( TempDisplayQueue, &argsToSend, 0U );
                if(ulTaskNotifyTakeIndexed( DOOR_CLOSED_NOTIF_INDEX,pdTRUE,portMAX_DELAY )==1);//wait for the print to be finished and then close the door
                    LED_SetLow(); //Door Closed
            }
        }
        else
        {
            setArgsToSend(PrintLUT[noPerson], PrintLUT[detected], TEMP_HIDDEN, TEMP_INVALID,&argsToSend);
            xQueueSend( TempDisplayQueue, &argsToSend, 0U );
        }
        
      tempDiffOkFlg=false;
      ADC0_StartConversion(ADC_MUXPOS_AIN4_gc);
      vTaskResume(MotionCheckHandler);
      vTaskSuspend(NULL);
    }
    
    
}


void timeoutTimerExpired()
{
    xTaskNotifyGiveIndexed(TempMeasHandler,TEMP_MEASURE_NOTIF_INDEX);
}

