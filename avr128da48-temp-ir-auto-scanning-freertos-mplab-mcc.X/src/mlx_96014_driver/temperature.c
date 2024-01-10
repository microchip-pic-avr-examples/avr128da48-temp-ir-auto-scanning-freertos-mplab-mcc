/*
 * File:   temperature.c
 * Author: M16128
 *
 * Created on June 29, 2020, 1:11 PM
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "mcc_generated_files/system/system.h"
#include "FreeRTOS.h"
#include "task.h"

double Temperature_Convert(uint8_t byteL, uint8_t byteH)
{
    double celsius,tempData;
    double tempFactor = 0.02;
    tempData = (double)(((byteH & 0x007F) << 8) + byteL);
    tempData = (tempData * tempFactor)-0.01;
    celsius = tempData + TEMP_ABSOLUTE_ZERO;
    return celsius;
}

double Temperature_ReadObject(void)
{
    uint8_t data[3];
    double t_obj_C = TEMP_ABSOLUTE_ZERO;
    if(MLX_READ_TOBJECT(data))
    {
        t_obj_C = Temperature_Convert(data[0], data[1]);
    }
    return t_obj_C;
}

double Temperature_ReadAmbient(void)
{
    uint8_t data[3];
    double t_amb_C = TEMP_ABSOLUTE_ZERO;
    if(MLX_READ_TAMBIENT(data))
    {
        t_amb_C = Temperature_Convert(data[0], data[1]);
    }
    return t_amb_C;
}

double Temperature_MaxAmb(uint8_t times)
{
    uint8_t i;
    double temp = TEMP_ABSOLUTE_ZERO, max = TEMP_ABSOLUTE_ZERO;

    for (i=0;i<times;i++)
    {
        temp = Temperature_ReadAmbient();
        if (temp > max)
            max = temp;
        vTaskDelay(pdMS_TO_TICKS(MEASURE_INTERVAL));
    }
    return max;
}

double Temperature_MaxObj(uint8_t times)
{
    uint8_t i;
    double temp = TEMP_ABSOLUTE_ZERO, max = TEMP_ABSOLUTE_ZERO;

    for (i=0;i<times;i++)
    {
        temp = Temperature_ReadObject();
        if (temp > max)
            max = temp;
        vTaskDelay(pdMS_TO_TICKS(MEASURE_INTERVAL));
    }
    return max;
}