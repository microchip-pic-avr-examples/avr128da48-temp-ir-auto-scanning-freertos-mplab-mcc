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
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef _CONFIG_H
#define	_CONFIG_H



#include "oledDriver/oledC_click.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"
#include "spiDriver/spi_baremetal.h"
#include "i2cDriver/i2c.h"
#include "config.h"
#include "mlx_96014_driver/temperature.h"
#include "mlx_96014_driver/mlx_96014.h"
//This file includes all the defines used as parameters in this code example


//Config definitions
#define MCC_API_USED        0 
#define TEMP_VALID          1
#define TEMP_INVALID        0
#define TEMP_GOOD           37.7        // Max Good temp, Celsius Degrees
#define TEMP_DIFF           3.4         // Max Diff T_obj-Tamb, Celsius Degrees
#define TEMP_HIGH           42.0        // Max High temp, Celsius Degrees
#define TEMP_LOW            32.0        // Max Lowe temp, Celsius Degrees
#define TEMP_HIDDEN         (-300.0)      // Hide temperature for Main_Display
#define TIMEOUT_MEAS        300         // Number of Meas before timeout
#define REPEATED_MEAS       100         // Number of repeated measurements
#define TEMP_MEASURE_CORRECTION ( (double)4.5) //Temperature measure sensor adjustment
#define ONE_SECOND          1000        // Delay in miliseconds
#define THRE_SECONDS        3000        // Delay in miliseconds
#define MEASURE_INTERVAL    10          // Delay in miliseconds
#define TEMP_ABSOLUTE_ZERO  (-273.15)   // Absolute Zero in Celsius Degrees


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* _CONFIG_H */

