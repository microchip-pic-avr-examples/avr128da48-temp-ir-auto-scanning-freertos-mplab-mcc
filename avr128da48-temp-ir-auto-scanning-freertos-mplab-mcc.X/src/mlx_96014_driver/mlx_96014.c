/*
 * File:   mlx_90614.c
 * Author: M50683
 *
 * Created on June 11, 2020, 1:11 PM
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "mcc_generated_files/system/system.h"


#define MLX_SLAVE_ADDRESS 0x5A

/* accept 5 bit address field, command types are specified in the header file */
/* returns true if operation is succeeded or false otherwise */
/* data buffer must have at least 3 bytes */
bool MLX_Read(uint8_t address, uint8_t *pData, MLX_CMD_t command_type)
{
#if MCC_API_USED == 1
    uint8_t command;
    bool ret;
#else
    uint8_t command,ret;
#endif
    
    bool  exit_val;
    
    if(pData == NULL)
        return false;
    
    switch(command_type)
    {
        case MLX_CMD_RAM:    command = 0x1F & address; break;
        case MLX_CMD_EEPROM: command = (0x1F & address) | 0x20; break;
        case MLX_CMD_FLAGS:  command = 0xF0; break;
        default:             return false;
    }
#if MCC_API_USED == 1
    ret=TWI0_Write((MLX_SLAVE_ADDRESS), &command, 1);
    if(ret==false)
    {
        DELAY_microseconds(1);
        TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
        exit_val = false;
    }
    else  // received ACK
    {
        ret = TWI0_Read((MLX_SLAVE_ADDRESS), pData, 3);
        DELAY_microseconds(1);
        TWI0.MCTRLB |= TWI_MCMD_STOP_gc;
        if(ret == false)  // did not get ACK
        {
            exit_val = false;
        }
        else    // received ACK
        {
            exit_val = true;
        }
    }
    return exit_val;
#else
    ret = I2C_0_SendData((MLX_SLAVE_ADDRESS<<1), &command, 1);
    if(ret == 0xFF) // did not get ACK
    {
        I2C_0_EndSession();
        exit_val = false;
    }
    else  // received ACK
    {
        ret = I2C_0_GetData((MLX_SLAVE_ADDRESS<<1), pData, 3);
        I2C_0_EndSession();
        if(ret == 0xFF)  // did not get ACK
        {
            exit_val = false;
        }
        else    // received ACK
        {
            exit_val = true;
        }
    }
    return exit_val;
#endif
}