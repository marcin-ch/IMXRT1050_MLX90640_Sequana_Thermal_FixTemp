/*
 * The Clear BSD License
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mbed.h"

#include "GUI.h"

extern "C" {
#include "board.h"
#include "pin_mux.h"    
#include "emwin_support.h"
}

#include "MLX90640_I2C_Driver.h"
#include "MLX90640_API.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MLX90640_ADDRESS (0x33) //Slave device address (MLX90640 32x24 IR array)
#define IMAGE_OFFSET_Y 32 //Free space under the top edge of the display reserved for labels,
                          //MLX90640 is 32x24, STM32F746G-DISCO display is 480x272, active area set to 480x240 (32 pixels at the top for labels)
#define IMAGE_HEIGHT 240
#define IMAGE_SCALE_X 10
#define IMAGE_SCALE_Y 10
#define TEMP_MIN 21.0
#define TEMP_MAX 35.0

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint16_t eeMLX90640[832]; //Read all EEPROM data from MLX90640 into a MCU memory location defined by user
static uint16_t mlx90640Frame[834]; //Get all RAM data from MLX90640 into a MCU memory location defined by user
paramsMLX90640 mlx90640; //Variable of type paramsMLX90640 to store extracted parameters from EEPROM
static float mlx90640To[768]; //The calculated object temperatures table
float emissivity = 1; //The emissivity is a property of the measured object
float eTa; //Reflected temperature defined by the user (here the ambient temperature could be used from MLX90640_GetTa() function)
uint8_t ir, ig, ib; //Values to compose pixel color

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_EnableLcdInterrupt(void);

/*******************************************************************************
 * Objects
 ******************************************************************************/
Serial pc(USBTX, USBRX, 115200); //UART, 115200 bauds, 8-bit data, no parity

/*******************************************************************************
 * Code
 ******************************************************************************/
void compute_false_colour(double v, uint8_t *ir, uint8_t *ig, uint8_t *ib) {
    const int NUM_COLORS = 7;
    static float color[NUM_COLORS][3] = { {0,0,0}, {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0}, {1,0,1}, {1,1,1} };
    int idx1, idx2;
    float fractBetween = 0;
    
    float vmin = TEMP_MIN;
    float vmax = TEMP_MAX;
        
    float vrange = vmax-vmin;
    v -= vmin;
    v /= vrange;
    if(v <= 0) {idx1=idx2=0;}
    else if(v >= 1) {idx1=idx2=NUM_COLORS-1;}
    else
    {
        v *= (NUM_COLORS-1);
        idx1 = floor(v);
        idx2 = idx1+1;
        fractBetween = v - float(idx1);
    }

    *ir = (uint8_t)((((color[idx2][0] - color[idx1][0]) * fractBetween) + color[idx1][0]) * 255.0);
    *ig = (uint8_t)((((color[idx2][1] - color[idx1][1]) * fractBetween) + color[idx1][1]) * 255.0);
    *ib = (uint8_t)((((color[idx2][2] - color[idx1][2]) * fractBetween) + color[idx1][2]) * 255.0);
}

void fb_put_pixel(uint16_t x, uint16_t y, uint8_t ir, uint8_t ig, uint8_t ib)
{
	uint32_t color;

	color = 0x00 << 24;
	color |= ib << 16;
	color |= ig << 8;
	color |= ir;

	GUI_SetColor(color);
	GUI_DrawPixel(x, y);
}

void fb_put_rect(uint16_t x, uint16_t y, uint8_t ir, uint8_t ig, uint8_t ib)
{
	uint32_t color;

	color = 0x00 << 24;
	color |= ib << 16;
	color |= ig << 8;
	color |= ir;

	GUI_SetColor(color);
	GUI_FillRect(x, y, x + IMAGE_SCALE_X, y + IMAGE_SCALE_Y);
}

void draw_color_scale()
{
    GUI_SetColor(GUI_BLACK);
	GUI_DrawRect(400, IMAGE_OFFSET_Y - 16, 400+79, 255); //To draw a frame for color scale
    
    int temp_legend_max = TEMP_MAX, temp_legend_min = TEMP_MIN;
	char print_buf_legend_min[4], print_buf_legend_max[4];

    float delta = (TEMP_MAX - TEMP_MIN) / IMAGE_HEIGHT;

    for (int y = 0; y < IMAGE_HEIGHT - 2; y++) {
		for (int x = 0; x < 80 - 2; x++) {
			float temp = (y * delta) + TEMP_MIN;
			compute_false_colour(temp, &ir, &ig, &ib);
			fb_put_pixel(x + 400 + 1, IMAGE_HEIGHT - y + 14, ir, ig, ib); //Calculations to put color scale inside the frame for color scale
		}
	}

    sprintf(print_buf_legend_min, "%2dC", temp_legend_min);
	GUI_SetColor(GUI_BLUE);
	GUI_DispStringAt(print_buf_legend_min, 360, 245);

	sprintf(print_buf_legend_max, "%2dC", temp_legend_max);
	GUI_DispStringAt(print_buf_legend_max, 360, 5);
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_InitI2C1Pins();
    BOARD_InitSemcPins();
    BOARD_InitLcdifPixelClock();
    BOARD_InitLcd();
    
    pc.printf("GUI demo start.\n\r");

    GUI_Init();
    
    /*Intro screen*/
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear(); //Fills the display/the active window with the background color
    GUI_SetBkColor(GUI_GREEN);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_ASCII);
    GUI_DispStringHCenterAt("i.MXRT1050-EVK" , 240, 55);
    GUI_X_Delay(10000);
    GUI_DispStringHCenterAt("MELEXIS MLX90640" , 240, 110);
    GUI_X_Delay(10000);
    GUI_DispStringHCenterAt("FUTURE ELECTRONICS" , 240, 165);
    GUI_X_Delay(20000);
    
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear(); //Fills the display/the active window with the background color
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_SetColor(GUI_BLUE);
    GUI_DispStringAt("SEQUANA THERMAL", 60, 5);

    draw_color_scale();

    pc.printf("Ready to go!\n\r");

    MLX90640_I2CFreqSet(1000); //I2C Clock = 1MHz
    MLX90640_SetRefreshRate(MLX90640_ADDRESS, 0x05); //0x05=16Hz
    //MLX90640_SetChessMode(MLX90640_ADDRESS); //Not needed to setup ChessMode as it comes by default

#if 0
    /*Checking current refresh rate and working mode (chess or interleaved)*/
    int curRR;
    curRR = MLX90640_GetRefreshRate(MLX90640_ADDRESS);
    pc.printf("Refresh rate is: 0x%02x\n\r", curRR);
    
    int curMODE;
    curMODE = MLX90640_GetCurMode(MLX90640_ADDRESS);
    pc.printf("Working mode is (1 for chess pattern, 0 for interleaved): %d\n\r", curMODE);
#endif

    MLX90640_DumpEE(MLX90640_ADDRESS, eeMLX90640);

#if 0
    pc.printf("Read EEPROM data is:\n\r");
    for (uint32_t i = 0U; i < 832; i++)
    {
        if (i % 8 == 0)
        {
            pc.printf("\r\n");
        }
        pc.printf("0x%04x  ", eeMLX90640[i]);
    }
    pc.printf("\r\n\r\n");
#endif

    MLX90640_ExtractParameters(eeMLX90640, &mlx90640);

#define NORMAL //Change to TEST for testing case
#ifdef TEST
    for(int i = 0; i < 10; i++)
    {
        MLX90640_GetFrameData(MLX90640_ADDRESS, mlx90640Frame);
        eTa = MLX90640_GetTa(mlx90640Frame, &mlx90640);
        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, eTa, mlx90640To); //Calculating the object temperatures for all 768 pixels

        pc.printf("Calculated object temperatures [%d] \n\r ", i);
        for (uint32_t i = 0U; i < 32; i++)
    	{
    		if (i % 16 == 0)
    		{
    			pc.printf("\r\n");
    		}
    		pc.printf("%.2f  ", mlx90640To[i]);
    	}
    	pc.printf("\r\n\r\n");
    }
#else
    while(1){
        MLX90640_GetFrameData(MLX90640_ADDRESS, mlx90640Frame);
        eTa = MLX90640_GetTa(mlx90640Frame, &mlx90640);
        MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, eTa, mlx90640To); //Calculating the object temperatures for all 768 pixels

        //to consider:
        //when ambient temperature is being used then shif,t is needed #define TA_SHIFT 8, see the MLX90640 driver datasheet 3.1.14 point 2.
        
        for(int y = 0; y < 24 ; y++)
        {
            for(int x = 0; x < 32; x++)
            {
                float val = mlx90640To[32 * (23 - y) + x];
                compute_false_colour(val, &ir, &ig, &ib);
                fb_put_rect(((320 - IMAGE_SCALE_X)-(x*IMAGE_SCALE_X)), (272 - y*IMAGE_SCALE_Y) , ir, ig, ib);
                //fb_put_rect((x*IMAGE_SCALE_X), (272 - y*IMAGE_SCALE_Y) , ir, ig, ib); //when reference tab looks down (mounted in breadboard)
    			//fb_put_rect((320 - IMAGE_SCALE_X)-(x*IMAGE_SCALE_X), y*IMAGE_SCALE_Y + IMAGE_OFFSET_Y, ir, ig, ib); //when reference tab looks up (mounted in breadboard)
            }
        }
    }
#endif
} //end main