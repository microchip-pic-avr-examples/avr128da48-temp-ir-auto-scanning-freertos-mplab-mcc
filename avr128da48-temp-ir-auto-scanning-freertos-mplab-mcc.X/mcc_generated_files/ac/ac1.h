/**
 * AC1 Generated Driver API Header File
 * 
 * @file ac1.h
 * 
 * @defgroup  ac1 AC1
 * 
 * @brief Contains the API prototypes for the AC1 driver.
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


#ifndef AC1_H_INCLUDED
#define AC1_H_INCLUDED
 
#include "../system/utils/compiler.h"

#ifdef __cplusplus  
extern "C" {
#endif

/**
 * @ingroup ac1
 * @brief Initializes the AC1. This routine should only be called once during system initialization.
 * @param None.
 * @return None.
*/
int8_t AC1_Initialize(void);
volatile bool acInterruptFlg;
#ifdef __cplusplus
}
#endif

#endif  /* AC1_H_INCLUDED */