/*
© [2022] Microchip Technology Inc. and its subsidiaries.

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

#if 0

#include "mcc_generated_files/system/system.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "partest.h"

#include "StaticAllocation.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY        ( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_PERIOD_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS         ( 200 / portTICK_PERIOD_MS )

/* LED that is toggled periodically  */
#define mainCHECK_TASK_LED          ( 0 )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH             ( 1 )


/*-----------------------------------------------------------*/

/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;
static StackType_t   uxStackRxBuffer[ configMINIMAL_STACK_SIZE ];
static StackType_t   uxStackTxBuffer[ configMINIMAL_STACK_SIZE ];

void FreeRTOS_Initialize(void){

    FreeRTOS_AppStart();

    /* If all is well, the FreeRTOS user application will now be running, and the following
    line will never be reached */
    for( ;; );
}

void  FreeRTOS_AppStart(void)
{
    static TaskHandle_t xCreatedRxTask, xCreatedTxTask;
    static StaticTask_t xTCBRxBuffer, xTCBTxBuffer;

/* StaticQueue_t is a publicly accessible structure that has the same size and
 * alignment requirements as the real queue structure.  It is provided as a
 * mechanism for applications to know the size of the queue (which is dependent on
 * the architecture and configuration file settings) without breaking the strict
 * data hiding policy by exposing the real queue internals.  This StaticQueue_t
 * variable is passed into the xQueueCreateStatic() function calls within this
 * function. */
        static StaticQueue_t xStaticQueue;

/* The queue storage area must be large enough to hold the maximum number of
 * items it is possible for the queue to hold at any one time, which equals the
 * queue length (in items, not bytes) multiplied by the size of each item.  In this
 * case the queue will hold staticQUEUE_LENGTH_IN_ITEMS 32-bit items.  See
 * http://www.freertos.org/Embedded-RTOS-Queues.html */
        static uint8_t ucQueueStorageArea[ mainQUEUE_LENGTH * sizeof( uint32_t ) ];

        /* Create the queue.  xQueueCreateStatic() has two more parameters than the
         * usual xQueueCreate() function.  The first new parameter is a pointer to the
         * pre-allocated queue storage area.  The second new parameter is a pointer to
         * the StaticQueue_t structure that will hold the queue state information in
         * an anonymous way.  If the two pointers are passed as NULL then the data
         * will be allocated dynamically as if xQueueCreate() had been called. */
        xQueue = xQueueCreateStatic( mainQUEUE_LENGTH,            /* The maximum number of items the queue can hold. */
                                     sizeof( uint32_t ),          /* The size of each item. */
                                     ucQueueStorageArea,          /* The buffer used to hold items within the queue. */
                                     &xStaticQueue );             /* The static queue structure that will hold the state of the queue. */

        /* The queue handle should equal the static queue structure passed into the
         * xQueueCreateStatic() function. */
        configASSERT( xQueue == ( QueueHandle_t ) &xStaticQueue );

        if( xQueue != NULL )
        {
            
         /* Create the task.  xTaskCreateStatic() has two more parameters than
         * the usual xTaskCreate() function.  The first new parameter is a pointer to
         * the pre-allocated stack.  The second new parameter is a pointer to the
         * StaticTask_t structure that will hold the task's TCB.  If both pointers are
         * passed as NULL then the respective object will be allocated dynamically as
         * if xTaskCreate() had been called. */
        xCreatedRxTask = xTaskCreateStatic(
            prvQueueReceiveTask,            /* Function that implements the task. */
            "RxStatic",                      /* Human readable name for the task. */
            configMINIMAL_STACK_SIZE,      /* Task's stack size, in words (not bytes!). */
            NULL,                          /* Parameter to pass into the task. */
            mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority of the task. */
            &( uxStackRxBuffer[ 0 ] ),       /* The buffer to use as the task's stack. */
            &xTCBRxBuffer );                 /* The variable that will hold that task's TCB. */

         xCreatedTxTask = xTaskCreateStatic(
            prvQueueSendTask,            /* Function that implements the task. */
            "TxStatic",                      /* Human readable name for the task. */
            configMINIMAL_STACK_SIZE,      /* Task's stack size, in words (not bytes!). */
            NULL,                          /* Parameter to pass into the task. */
            mainQUEUE_SEND_TASK_PRIORITY, /* The priority of the task. */
            &( uxStackTxBuffer[ 0 ] ),       /* The buffer to use as the task's stack. */
            &xTCBTxBuffer );                 /* The variable that will hold that task's TCB. */
            /* Start the tasks and timer running. */
           if ((xCreatedRxTask != NULL )&& (xCreatedTxTask != NULL )) 
           {
               vTaskStartScheduler();
           }
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

  
static void prvQueueSendTask( void *pvParameters )
{
TickType_t xNextWakeTime;
const unsigned long ulValueToSend = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

        /* Send to the queue - causing the queue receive task to unblock and
        toggle the LED.  0 is used as the block time so the sending operation
        will not block - it shouldn't need to block as the queue should always
        be empty at this point in the code. */
        xQueueSend( xQueue, &ulValueToSend, 0U );
    }
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters )
{
unsigned long ulReceivedValue;
const unsigned long ulExpectedValue = 100UL;

    /* Remove compiler warning about unused parameter. */
    ( void ) pvParameters;

    for( ;; )
    {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h. */
        xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

        /*  To get here something must have been received from the queue, but
        is it the expected value?  If it is, toggle the LED. */
        if( ulReceivedValue == ulExpectedValue )
        {
            /* Toggle LED */
            vParTestToggleLED( mainCHECK_TASK_LED );
            ulReceivedValue = 0U;
        }
    }
}


/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
/* If the buffers to be provided to the Idle task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
                                     StackType_t **ppxTimerTaskStackBuffer,
                                     uint32_t *pulTimerTaskStackSize )
{
/* If the buffers to be provided to the Timer task are declared inside this
function then they must be declared static - otherwise they will be allocated on
the stack and so not exists after this function exits. */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[ configTIMER_TASK_STACK_DEPTH ];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif





