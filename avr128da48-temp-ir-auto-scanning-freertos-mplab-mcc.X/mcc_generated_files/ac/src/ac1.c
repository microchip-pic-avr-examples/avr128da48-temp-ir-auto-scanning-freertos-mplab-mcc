/**
 * AC1 Generated Driver File
 * 
 * @file ac1.c
 * 
 * @ingroup  ac1
 * 
 * @brief Contains the API implementation for the AC1 driver.
 *
 * @version AC1 Driver Version 1.0.0
*/
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


#include "../ac1.h"

volatile bool acInterruptFlg=false;

int8_t AC1_Initialize(void) 
{
    //WINSEL Window function disabled; 
    AC1.CTRLB = 0x0;

    //DACREF 187; 
    AC1.DACREF = 0xBB;
    
    //CMP enabled; INTMODE Positive input goes above negative input; 
    AC1.INTCTRL = 0x31;
    
    //INITVAL LOW; INVERT disabled; MUXNEG DAC Reference; MUXPOS Positive Pin 2;   
    AC1.MUXCTRL = 0x13;
    
    //ENABLE enabled; HYSMODE No hysteresis; OUTEN disabled; POWER Power profile 0, Shortest response time, highest consumption; RUNSTDBY disabled; 
    AC1.CTRLA = 0x1;

    return 0;
}

ISR(AC1_AC_vect)
{
    /* Insert your AC interrupt handling code here */
    acInterruptFlg=true;
    /* The interrupt flag has to be cleared manually */
    AC1.STATUS = AC_CMPIF_bm;
}

