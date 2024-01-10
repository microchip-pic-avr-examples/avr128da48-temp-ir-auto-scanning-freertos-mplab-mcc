/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#include <stdlib.h>

#include <avr/interrupt.h>
#include "FreeRTOS.h"
#include "task.h"
#include "../../timer/tcb0.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the AVR port.
 *----------------------------------------------------------*/

/* Start tasks with interrupts enables. */
#define portFLAGS_INT_ENABLED   ((StackType_t)0x80)

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
any details of its type. */
typedef void                    RTOS_TCB_t;
extern volatile                 RTOS_TCB_t *volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/* Perform hardware setup to enable ticks from timer. */
static void prvSetupTimerInterrupt(void);
/*-----------------------------------------------------------*/

/* See header file for description. */
StackType_t *pxPortInitialiseStack(StackType_t *pxTopOfStack,
                                   TaskFunction_t pxCode,
                                   void *pvParameters)
{
    uint16_t usAddress;

    /*lint -e950 -e611 -e923 Lint doesn't like this much - but nothing I can do about it. */

    /* Place a few bytes of known values on the bottom of the stack.
    This is just useful for debugging.  Uncomment if needed. */
    /* *pxTopOfStack = 0x11; */
    /* pxTopOfStack--; */
    /* *pxTopOfStack = 0x22; */
    /* pxTopOfStack--; */
    /* *pxTopOfStack = 0x33; */
    /* pxTopOfStack--; */

    /* The start of the task code will be popped off the stack last, so place
     * it on first. */
    usAddress = (uint16_t)pxCode;
    *pxTopOfStack = (StackType_t)(usAddress & (uint16_t)0x00ff);
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = (StackType_t)(usAddress & (uint16_t)0x00ff);
    pxTopOfStack--;

    /* Next simulate the stack as if after a call to portSAVE_CONTEXT().
    portSAVE_CONTEXT places the flags on the stack immediately after r0
    to ensure the interrupts get disabled as soon as possible, and so ensuring
    the stack use is minimal should a context switch interrupt occur. */
    *pxTopOfStack = (StackType_t)0x00; /* R0 */
    pxTopOfStack--;
    *pxTopOfStack = portFLAGS_INT_ENABLED;
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t)0x00; /* RAMPZ */
    pxTopOfStack--;

    /* Now the remaining registers.  The compiler expects R1 to be 0. */
    *pxTopOfStack = (StackType_t)0x00; /* R1 */

    /* Leave R2 - R23 untouched */
    pxTopOfStack -= 23;

    /* Place the parameter on the stack in the expected location. */
    usAddress = (uint16_t)pvParameters;
    *pxTopOfStack = (StackType_t)(usAddress & (uint16_t)0x00ff);
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = (StackType_t)(usAddress & (uint16_t)0x00ff);

    /* Leave register R26 - R31 untouched */
    pxTopOfStack -= 7;

    /*lint +e950 +e611 +e923 */

    return pxTopOfStack;
}

/*-----------------------------------------------------------*/

BaseType_t xPortStartScheduler(void)
{
    /* Setup the hardware to generate the tick. */
    prvSetupTimerInterrupt();

    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();

    /* Simulate a function call end as generated by the compiler.  We will now
     * jump to the start of the task the context of which we have just restored. */
    asm volatile("ret");

    /* Should not get here. */
    return pdTRUE;
}

/*-----------------------------------------------------------*/

void vPortEndScheduler(void)
{
    /* vPortEndScheduler is not implemented in this port. */
}
/*-----------------------------------------------------------*/

/* Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute. */
void vPortYield(void) __attribute__((naked));

void vPortYield(void)
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
    asm volatile("ret");
}
/*-----------------------------------------------------------*/

/* Manual context switch callable from ISRs.  The first thing
 * we do is save the registers so we can use a naked attribute. */
void vPortYieldFromISR(void) __attribute__((naked));

void vPortYieldFromISR(void)
{
    portSAVE_CONTEXT();
    vTaskSwitchContext();
    portRESTORE_CONTEXT();
    asm volatile("reti");
}

/*-----------------------------------------------------------*/
static void prvTickCallback(void)   
{
    if( xTaskIncrementTick() != pdFALSE )
    {
        vTaskSwitchContext();
    }
}

/* Setup timer to generate a tick interrupt. */
static void prvSetupTimerInterrupt(void)
{
    /* Configure low-power timer  used in tickless mode */
    TICK_TMR_CaptureCallbackRegister(prvTickCallback);
	TICK_TMR_EnableCaptInterrupt();
    TICK_TMR_Start();
}
