/**
 * @copyright (C) 2017 Melexis N.V.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "mbed.h"
#include "MLX90640_I2C_Driver.h"

I2C i2c(I2C_SDA, I2C_SCL);

void MLX90640_I2CInit()
{   
    //i2c.stop();
}

int MLX90640_I2CRead(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{

/* Melexis MLX9060 driver documentation C:\Projects\Thermal_imager\input\melexis\mlx90640-library-master
 * Note that the address location is being auto incremented while I2C read communication is in progress,
 * but if a new read is initiated the address will be reset.
 * Thus if a large memory should be dumped in more than one I2C read command,
 * special care should be taken so that the appropriate start address is used.
 */
 
 /* Note, Melexis presents data in 16 bits words (2 bytes).
  * That is the reason for below 128 words (16 bits) which is equal to 256 bytes.
  */
    
    int left = nMemAddressRead;
    int reVal = 1; //to indicate an error
    
    do {
        if(left > 128) {
            reVal = MLX90640_I2CReadBlockBytes256(slaveAddr, startAddress, 128, data);
            data += 128;
            startAddress += 128;
            left -= 128;

            if(reVal != 0)
            {
                return reVal;
            }

        } else {
            reVal = MLX90640_I2CReadBlockBytes256(slaveAddr, startAddress, left, data);
            //data += left;
            //startAddress += left;
            left -= left;

            if(reVal != 0)
            {
                return reVal;
            }
        }
    } while (left);

    return 0;
}

int MLX90640_I2CReadBlockBytes256(uint8_t slaveAddr, uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data)
{
    uint8_t sa;                           
    int ack = 0;                               
    int cnt = 0;
    int i = 0;
    char cmd[2] = {0,0};
    char i2cData[1664] = {0};
    uint16_t *p;
    
    p = data;
    sa = (slaveAddr << 1);
    cmd[0] = startAddress >> 8;
    cmd[1] = startAddress & 0x00FF;
    
    //i2c.stop();
    //wait_us(5);    
    ack = i2c.write(sa, cmd, 2, 1);
    
    if (ack != 0x00)
    {
        return -1;
    }
             
    sa = sa | 0x01;
    ack = i2c.read(sa, i2cData, 2*nMemAddressRead, 0);
    
    if (ack != 0x00)
    {
        return -1; 
    }          
    //i2c.stop();   
    
    for(cnt=0; cnt < nMemAddressRead; cnt++)
    {
        i = cnt << 1;
        *p++ = (uint16_t)i2cData[i]*256 + (uint16_t)i2cData[i+1];
    }
    
    return 0;   
} 

void MLX90640_I2CFreqSet(int freq)
{
    i2c.frequency(1000*freq);
}

int MLX90640_I2CWrite(uint8_t slaveAddr, uint16_t writeAddress, uint16_t data)
{
    uint8_t sa;
    int ack = 0;
    char cmd[4] = {0,0,0,0};
    static uint16_t dataCheck;
      
    sa = (slaveAddr << 1);
    cmd[0] = writeAddress >> 8;
    cmd[1] = writeAddress & 0x00FF;
    cmd[2] = data >> 8;
    cmd[3] = data & 0x00FF;

    //i2c.stop();
    //wait_us(5);    
    ack = i2c.write(sa, cmd, 4, 0);
    
    if (ack != 0x00)
    {
        return -1;
    }         
    //i2c.stop();   
    
    MLX90640_I2CRead(slaveAddr,writeAddress,1, &dataCheck);
    
    if ( dataCheck != data)
    {
        return -2;
    }    
    
    return 0;
}