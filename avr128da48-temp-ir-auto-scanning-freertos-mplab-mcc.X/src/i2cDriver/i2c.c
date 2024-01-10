

#include "mcc_generated_files/system/system.h"
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef TWI0_BAUD
#undef TWI0_BAUD
#endif

#define TWI0_BAUD(F_SCL, T_RISE)                        ((((((float)F_CPU / (float)F_SCL)) - 10 - ((float)F_CPU * T_RISE))) / 2)
#define I2C_SCL_FREQ                                    100000  //Frequency in Hz
#define I2C_1BIT_DELAY                                  (int)(1+1*1000000.0/(float)I2C_SCL_FREQ)

enum {I2C_INIT = 0, I2C_ACKED, I2C_NACKED, I2C_READY, I2C_ERROR};

void I2C_0_Init(void)
{
    //Set PC2-> SDA, PC3-SCL pins
    PORTMUX.TWIROUTEA = 0x02;
    
    //SDAHOLD OFF; FMPEN disabled; 
    TWI0.CTRLA = 0x00;
    TWI0.MCTRLA = 0x00;
    
    //Debug Run
    TWI0.DBGCTRL = 0x00;
    
    //Master Baud Rate Control
    TWI0.MBAUD = (uint8_t)TWI0_BAUD(I2C_SCL_FREQ, 0);
    
    //Master Address
    TWI0.MADDR = 0x00;
    
    //FLUSH enabled; ACKACT ACK; MCMD NOACT; 
    TWI0.MCTRLB = TWI_FLUSH_bm;
    
    //Master Data
    TWI0.MDATA = 0x00;
    
    //RIEN disabled; WIEN disabled; QCEN disabled; TIMEOUT DISABLED; SMEN disabled; ENABLE enabled; 
    TWI0.MCTRLA = TWI_ENABLE_bm;
    
    //RIF disabled; WIF disabled; CLKHOLD disabled; ARBLOST disabled; BUSERR disabled; BUSSTATE UNKNOWN; 
    TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
}


static bool i2c_0_CheckBus(void)
{
    /* if the I2C bus is non-idle force it to idle */
    if((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc)
    {
        TWI0.MCTRLB = TWI_FLUSH_bm;
        TWI0.MSTATUS = TWI_BUSSTATE_IDLE_gc;
        _delay_us(I2C_1BIT_DELAY);
        if((TWI0.MSTATUS & TWI_BUSSTATE_gm) != TWI_BUSSTATE_IDLE_gc)
            return false;
        else
            return true;
    }
    return true;
}


static uint8_t i2c_0_WaitW(void)
{
    uint8_t state = I2C_INIT;
    do
    {
        if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
        {
            if(!(TWI0.MSTATUS & TWI_RXACK_bm))
            {
                /* slave responded with ack - TWI goes to M1 state */
                state = I2C_ACKED;
            }
            else
            {
                /* address sent but no ack received - TWI goes to M3 state */
                state = I2C_NACKED;
            }
        }
        else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
        {
            /* get here only in case of bus error or arbitration lost - M4 state */
            state = I2C_ERROR;
        }
    } while(!state);
    return state;
}

static uint8_t i2c_0_WaitR(void)
{
    uint8_t state = I2C_INIT;
    do
    {
        if(TWI0.MSTATUS & (TWI_WIF_bm | TWI_RIF_bm))
        {
            state = I2C_READY;
        }
        else if(TWI0.MSTATUS & (TWI_BUSERR_bm | TWI_ARBLOST_bm))
        {
            /* get here only in case of bus error or arbitration lost - M4 state */
            state = I2C_ERROR;
        }
    } while(!state);
    return state;
}

 /* returns how many bytes have been sent, -1 means NACK at address, 0 means slave ACKed to slave address */
uint8_t I2C_0_SendData(uint8_t address, uint8_t *pData, uint8_t len)
{
    uint8_t retVal = (uint8_t)-1;
    if(i2c_0_CheckBus() == false)
        return retVal;
    
    /* start transmitting the slave address */
    TWI0.MADDR = address & ~0x01;
    if(i2c_0_WaitW() != I2C_ACKED)
        return retVal;

    retVal = 0;
    if((len != 0) && (pData != NULL ))
    {
        while(len--)
        {
            //_delay_us(I2C_1BIT_DELAY);
            TWI0.MDATA = *pData;
            if(i2c_0_WaitW() == I2C_ACKED)
            {
                retVal++;
                pData++;
                continue;
            }
            else // did not get ACK after slave address
            {
                break;
            }
        }
    }
    return retVal;
}

uint8_t I2C_0_GetData(uint8_t address, uint8_t *pData, uint8_t len) // returns how many bytes have been received, -1 means NACK at address
{
    uint8_t retVal = (uint8_t)-1;
    if(i2c_0_CheckBus() == false)
        return retVal;
    
    /* start transmitting the slave address */
    TWI0.MADDR = address | 0x01;
    if(i2c_0_WaitW() != I2C_ACKED)
        return retVal;

    retVal = 0;
    if((len != 0) && (pData !=NULL ))
    {
        while(len--)
        {
            if(i2c_0_WaitR() == I2C_READY)
            {
               *pData = TWI0.MDATA;
                TWI0.MCTRLB = (len == 0)? TWI_ACKACT_bm | TWI_MCMD_STOP_gc : TWI_MCMD_RECVTRANS_gc;
                retVal++;
                pData++;
                continue;
            }
            else
                break;
        }
    }
    return retVal;
}

void    I2C_0_EndSession(void)
{
    _delay_us(I2C_1BIT_DELAY);
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
}