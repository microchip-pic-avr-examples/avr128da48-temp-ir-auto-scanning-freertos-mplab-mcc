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

#include "partest.h"
#include "recmutex.h"
#include "regtest.h"
#include "semtest.h"
#include "TaskNotify.h"


#define mainSEM_TEST_PRIORITY       ( tskIDLE_PRIORITY + 1UL )
#define mainCHECK_TASK_PRIORITY     ( configMAX_PRIORITIES - 1 )

/* The period between executions of the check task. */
#define mainCHECK_PERIOD            ( ( TickType_t ) 3000 / portTICK_PERIOD_MS )

/* LED that is toggled by the check task.  The check task periodically checks
that all the other tasks are operating without error.  If no errors are found
the LED is toggled.  If an error is found at any time the LED is never toggles
again. */
#define mainCHECK_TASK_LED          ( 0 )

/* The check task, as described at the top of this file. */
static void prvCheckTask( void *pvParameters );

void FreeRTOS_Initialize(void){

    FreeRTOS_AppStart();

    /* If all is well, the FreeRTOS user application will now be running, and the following
    line will never be reached */
    for( ;; );
}

void  FreeRTOS_AppStart(void)
{
    vStartSemaphoreTasks(mainSEM_TEST_PRIORITY);
    vStartTaskNotifyTask();
    vStartRegTestTasks();
    vStartRecursiveMutexTasks();

    /* Create the task that performs the 'check' functionality, as described at
    the top of this file. */ 
    xTaskCreate( prvCheckTask, "Check", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );


    /* Start the tasks and timer running. */
    vTaskStartScheduler();
 
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

/* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
   1 in FreeRTOSConfig.h.
   This function is called from an interrupt context (the RTOS tick interrupt),
   so only ISR safe API functions can be used (those that end in "FromISR()".
*/
void vApplicationTickHook( void )
{
    xNotifyTaskFromISR();
}


static void prvCheckTask( void *pvParameters )
{
TickType_t xLastExecutionTime;
unsigned long ulErrorFound = pdFALSE;

    /* Just to stop compiler warnings. */
    ( void ) pvParameters;

    /* Initialise xLastExecutionTime so the first call to vTaskDelayUntil()
    works correctly. */
    xLastExecutionTime = xTaskGetTickCount();

    /* Cycle for ever, delaying then checking all the other tasks are still
    operating without error.  The onboard LED is toggled on each iteration 
    unless an error occurred. */
    for( ;; )
    {
        /* Delay until it is time to execute again. */
        vTaskDelayUntil( &xLastExecutionTime, mainCHECK_PERIOD );

        /* Check all the demo tasks (other than the flash tasks) to ensure
        that they are all still running, and that none have detected an error. */
        if( xAreSemaphoreTasksStillRunning() != pdTRUE )
        {
            ulErrorFound |= 1UL << 0UL;
        }

        if( xAreTaskNotificationTasksStillRunning() != pdTRUE )
        {
            ulErrorFound |= 1UL << 1UL;
        }

        if( xAreRegTestTasksStillRunning() != pdTRUE )
        {
            ulErrorFound |= 1UL << 2UL;
        }

        if ( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
        {
            ulErrorFound |= 1UL << 3UL;
        }

        if( ulErrorFound == pdFALSE )
        {
            /* Toggle the LED if everything is okay so we know if an error occurs even if not
            using console IO. */
            vParTestToggleLED( mainCHECK_TASK_LED );
        }
    }
}
#endif
