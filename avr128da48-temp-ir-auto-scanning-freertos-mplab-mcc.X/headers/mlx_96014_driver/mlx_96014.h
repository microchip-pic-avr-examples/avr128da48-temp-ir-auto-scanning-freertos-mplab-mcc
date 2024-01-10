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
#ifndef MLX_96014_H
#define	MLX_96014_H


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

#define MLX_TEMP_AMBIENT_ADDRESS   0x06
#define MLX_TEMP_OBJECT_ADDRESS    0x07
    
typedef enum {MLX_CMD_RAM, MLX_CMD_EEPROM, MLX_CMD_FLAGS} MLX_CMD_t;

/* accept 5 bit address field, command types are specified in the header file */
/* returns true if operation is succeeded or false otherwise */
/* data buffer must have at least 3 bytes */
bool MLX_Read(uint8_t address, uint8_t *pData, MLX_CMD_t command_type);

#define MLX_READ_TAMBIENT(PDATA)  MLX_Read(MLX_TEMP_AMBIENT_ADDRESS, PDATA, MLX_CMD_RAM)
#define MLX_READ_TOBJECT(PDATA)   MLX_Read(MLX_TEMP_OBJECT_ADDRESS,  PDATA, MLX_CMD_RAM)

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* MLX_90614_H */

