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

//#include "MLX90640_I2C_Driver.h"
//#include "MLX90640_API.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MLX90640_ADDRESS (0x33) //slave device address (MLX90640 32x24 IR array)
#define IMAGE_OFFSET_Y 32 //free space under the top edge of the display reserved for labels

/*******************************************************************************
 * Variables
 ******************************************************************************/
Serial pc(USBTX, USBRX, 115200);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_EnableLcdInterrupt(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
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
    
    pc.printf("GUI demo start.\r\n");

    GUI_Init();
    
    /* Intro screen */
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear(); //fills the display/the active window with the background color
    GUI_SetBkColor(GUI_GREEN);
    GUI_SetColor(GUI_WHITE);
    GUI_SetFont(&GUI_Font32B_ASCII);
    GUI_DispStringHCenterAt("i.MXRT1050-EVK" , 240, 55);
    GUI_X_Delay(10000);
    GUI_DispStringHCenterAt("MELEXIS MLX90640" , 240, 110);
    GUI_X_Delay(10000);
    GUI_DispStringHCenterAt("FE" , 240, 165);
    GUI_X_Delay(20000);
    
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear(); //fills the display/the active window with the background color
    GUI_SetFont(&GUI_Font24_ASCII);
    GUI_SetColor(GUI_BLUE);
    GUI_DispStringAt("THERMAL IMAGER", 60, 5);
} //end main