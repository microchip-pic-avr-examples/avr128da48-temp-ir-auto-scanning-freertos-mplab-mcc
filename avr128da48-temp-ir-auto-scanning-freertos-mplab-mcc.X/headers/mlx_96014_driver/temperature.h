/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   temperature.c
 * Author: M16128
 *
 * Created on June 29, 2020, 1:11 PM
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef TEMPERATURE_H
#define	TEMPERATURE_H


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

/* returns temperature in Celsius degrees */
double Temperature_Convert(uint8_t byteL, uint8_t byteH);

/* returns Object temperature in Celsius degrees */
double Temperature_ReadObject(void);

/* returns Ambient temperature in Celsius degrees */
double Temperature_ReadAmbient(void);

/* returns Maximum Ambient temperature in Celsius degrees */
double Temperature_MaxAmb(uint8_t times);

/* returns Maximum Object temperature in Celsius degrees */
double Temperature_MaxObj(uint8_t times);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* TEMPERATURE_H */

