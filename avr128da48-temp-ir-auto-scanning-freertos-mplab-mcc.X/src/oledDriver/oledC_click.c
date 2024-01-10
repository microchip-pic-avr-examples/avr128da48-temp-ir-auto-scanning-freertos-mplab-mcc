/**
 * \file
 *
 *
 (c) 2018 Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms,you may use this software and
    any derivatives exclusively with Microchip products.It is your responsibility
    to comply with third party license terms applicable to your use of third party
    software (including open source software) that may accompany Microchip software.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 */

#include "mcc_generated_files/system/system.h"
#include "FreeRTOS.h"
#include "task.h"

#define MAX_ADDRESS_BOUND               95
#define LOWER                           0
#define MIDD                            32
#define UPPER                           96
#define FONT_SIZE_X                     2
#define FONT_SIZE_Y                     3

enum STREAMING_MODES { NOSTREAM, WRITESTREAM, READSTREAM };
static uint8_t streamingMode = NOSTREAM;
static void     startStreamingIfNeeded(OLEDC_COMMAND cmd);
static void     stopStreaming(void);
static uint16_t exchangeTwoBytes(uint8_t byte1, uint8_t byte2);
volatile bool SPI_TransferDoneFlg=false;

#if MCC_API_USED == 1
void SPI_TransmitDataBuffer(uint8_t *arg,uint8_t argLength)
{
        OLED_CS_SetLow();          //We set the CS0(PA7) Pin Low to begin SPI transfer
        SPI0_s.BufferWrite(arg,argLength);
        while(SPI_TransferDoneFlg==false);
        //Wait for the Byte Exchange to be done
            SPI_TransferDoneFlg=false;
            OLED_CS_SetHigh();       //We set the CS0 High to end SPI transfer, after TXCIF is set
}
#endif

void oledC_printString(char *string, uint16_t color, uint8_t line)
{
    if(string == NULL)
        return;
    oledC_DrawString(
    LOWER,
    MIDD*line,
    FONT_SIZE_X+((line==2)?1:0),
    FONT_SIZE_Y,
    (uint8_t *)string,
    color);
}

oledc_color_t oledC_parseIntToRGB(uint16_t raw)
{
	oledc_color_t parsedColor;
	uint8_t       byte1 = raw >> 8;
	uint8_t       byte2 = raw & 0xFF;
	parsedColor.red     = (byte1 >> 3);
	parsedColor.green   = ((byte1 & 0x7) << 3) | (byte2 >> 5);
	parsedColor.blue    = byte2 & 0x1F;
	return parsedColor;
}

uint16_t oledC_parseRGBToInt(uint8_t red, uint8_t green, uint8_t blue)
{
	red &= 0x1F;
	green &= 0x3F;
	blue &= 0x1F;
	uint8_t byte1;
	uint8_t byte2;
	byte1 = (red << 3) | (green >> 3);
	byte2 = (green << 5) | blue;
	return (((uint16_t)byte1) << 8) | byte2;
}

static void startStreamingIfNeeded(OLEDC_COMMAND cmd)
{
	if (cmd == OLEDC_CMD_WRITE_RAM || cmd == OLEDC_CMD_READ_RAM) {
		streamingMode = cmd == OLEDC_CMD_WRITE_RAM ? WRITESTREAM : READSTREAM;
	} else {
		streamingMode = NOSTREAM;
	}
}

static void stopStreaming(void)
{
	streamingMode = NOSTREAM;
}
#if MCC_API_USED == 1
    static uint16_t exchangeTwoBytes(uint8_t byte1, uint8_t byte2)
    {

        if (!SPI0_s.(SPI0_DEFAULT)) {
            return 0xFFFF;
        }

        byte1 = SPI0_s.ByteExchange(byte1);
        byte2 = SPI0_s.ByteExchange(byte2);
        SPI0_s.Close();

        return ((uint16_t)byte1) << 8 | byte2;
    }
#else
    static uint16_t exchangeTwoBytes(uint8_t byte1, uint8_t byte2)
    {

        if (!SPI_MASTER_open(SPI_MASTER_DEFAULT)) {
            return 0xFFFF;
        }

        byte1 = SPI_MASTER_exchange_byte(byte1);
        byte2 = SPI_MASTER_exchange_byte(byte2);

        SPI_MASTER_close();

        return ((uint16_t)byte1) << 8 | byte2;
    }
#endif

#if MCC_API_USED == 1  
    void oledC_sendCommand(OLEDC_COMMAND cmd, uint8_t *payload, uint8_t payload_size)
    {
        uint8_t cmdReg=cmd;
        if (!SPI0_s.(SPI_MASTER_DEFAULT)) {
            return;
        }
        OLED_CS_SetLow();
        OLED_DC_SetLow();

        SPI_TransmitDataBuffer(&cmdReg,1);
        if (payload_size > 0) 
        {
            OLED_DC_SetHigh();
            SPI_TransmitDataBuffer(payload, payload_size);
            OLED_DC_SetLow();
        }
        OLED_CS_SetHigh();

        SPI0_s.Close();

        startStreamingIfNeeded(cmd);
    }
#else
    void oledC_sendCommand(OLEDC_COMMAND cmd, uint8_t *payload, uint8_t payload_size)
    {
        
        if (!SPI_MASTER_open(SPI_MASTER_DEFAULT)) {
            return;
        }
        OLED_CS_SetLow();
        OLED_DC_SetLow();
        SPI_MASTER_exchange_byte(cmd);
        if (payload_size > 0) 
        {
            OLED_DC_SetHigh();
            SPI_MASTER_write_block(payload, payload_size);
            OLED_DC_SetLow();
        }
        OLED_CS_SetHigh();
        SPI_MASTER_close();

        startStreamingIfNeeded(cmd);
    }
#endif

void oledC_setRowAddressBounds(uint8_t min, uint8_t max)
{
	uint8_t payload[2];
	payload[0] = min > MAX_ADDRESS_BOUND ? MAX_ADDRESS_BOUND : min;
	payload[1] = max > MAX_ADDRESS_BOUND ? MAX_ADDRESS_BOUND : max;
	oledC_sendCommand(OLEDC_CMD_SET_ROW_ADDRESS, payload, 2);
}

void oledC_setColumnAddressBounds(uint8_t min, uint8_t max)
{
	min = min > MAX_ADDRESS_BOUND ? MAX_ADDRESS_BOUND : min;
	max = max > MAX_ADDRESS_BOUND ? MAX_ADDRESS_BOUND : max;
	uint8_t payload[2];
	payload[0] = 16 + min;
	payload[1] = max + 16;
	oledC_sendCommand(OLEDC_CMD_SET_COLUMN_ADDRESS, payload, 2);
}

void oledC_setSleepMode(bool on)
{
	oledC_sendCommand(on ? OLEDC_CMD_SET_SLEEP_MODE_ON : OLEDC_CMD_SET_SLEEP_MODE_OFF, NULL, 0);
}

void oledC_setDisplayOrientation(void)
{
	uint8_t payload[1];
	payload[0] = 0x32;
	oledC_sendCommand(OLEDC_CMD_SET_REMAP_DUAL_COM_LINE_MODE, payload, 1);
	payload[0] = 0x20;
	oledC_sendCommand(OLEDC_CMD_SET_DISPLAY_START_LINE, payload, 1);
}

void oledC_startReadingDisplay(void)
{
	oledC_sendCommand(OLEDC_CMD_READ_RAM, NULL, 0);
    OLED_CS_SetLow();
    OLED_DC_SetHigh();
}

void oledC_stopReadingDisplay(void)
{
	oledC_stopWritingDisplay();
}

uint16_t oledC_readColor(void)
{
	if (streamingMode != READSTREAM) {
		oledC_startReadingDisplay();
	}
	if (streamingMode != READSTREAM) {
		return 0xFFFF;
	}
	return exchangeTwoBytes(0xFF, 0xFF);
}

void oledC_startWritingDisplay(void)
{
	oledC_sendCommand(OLEDC_CMD_WRITE_RAM, NULL, 0);
	OLED_CS_SetLow();
    OLED_DC_SetHigh();
}

void oledC_stopWritingDisplay(void)
{
    OLED_CS_SetHigh();
    OLED_DC_SetLow();

	stopStreaming();
}

void oledC_sendColor(uint8_t r, uint8_t g, uint8_t b)
{
	oledC_sendColorInt(oledC_parseRGBToInt(r, g, b));
}

#if MCC_API_USED == 1
    void oledC_sendColorInt(uint16_t raw)
    {
        uint8_t byteArray[2];
        //Divide 16bit Raw data into two 8bit data variables
        byteArray[0]=(raw>>8);
        byteArray[1]=(raw&0x00FF);

        if (streamingMode != WRITESTREAM) {
            oledC_startWritingDisplay();
        }
        if (streamingMode != WRITESTREAM) {
            return;
        }
        SPI_TransmitDataBuffer(byteArray,   sizeof(byteArray)/sizeof(byteArray[0]));
    }
#else
    void oledC_sendColorInt(uint16_t raw)
    {
        if (streamingMode != WRITESTREAM) {
            oledC_startWritingDisplay();
        }
        if (streamingMode != WRITESTREAM) {
            return;
        }
        exchangeTwoBytes(raw >> 8, raw & 0x00FF);
    }
#endif 
void oledC_clearScreen(void)
{
    oledC_setColumnAddressBounds(LOWER,UPPER);
    oledC_setRowAddressBounds(LOWER,UPPER);
    for(uint8_t x = LOWER; x < UPPER; x++){
        for(uint8_t y = LOWER; y < UPPER; y++){
            oledC_sendColorInt(OLEDC_COLOR_BLACK);
        }
    }
}

void oledC_setup(void)
{
    OLED_EN_SetLow();
    OLED_RST_SetHigh();
    OLED_RW_SetLow();
	vTaskDelay(pdMS_TO_TICKS(1));
    OLED_RST_SetLow();
	vTaskDelay(pdMS_TO_TICKS(2000));
    OLED_RST_SetHigh();
    OLED_EN_SetHigh();
	vTaskDelay(pdMS_TO_TICKS(1));
	oledC_setSleepMode(false);
	vTaskDelay(pdMS_TO_TICKS(200));
	oledC_setColumnAddressBounds(0, MAX_ADDRESS_BOUND);
	oledC_setRowAddressBounds(0, MAX_ADDRESS_BOUND);
	oledC_setDisplayOrientation();
}