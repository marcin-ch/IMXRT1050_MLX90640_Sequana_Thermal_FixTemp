# Problem description
When you uncomment the code:

`I2C i2c(I2C_SDA, I2C_SCL);`

in

`MLX90640\MLX90640_I2C_Driver.cpp`

then project still is able to compile, unfortunately i.MXRT1050 eval board does not start.

# What is the goal
I want to connect Melexis MLX90640 sensor to Arduino connector and be able to read sensor memory through I2C

# Additional info
* workaround for i.MXRT1050 eval board has been done based on [Important Notes](https://os.mbed.com/platforms/MIMXRT1050-EVK/)
  * it means connect D14 (SDA) to A4 and D15 (SCL) to A5
* emWin project prepared based on [How to use emWin with Mbed OS](https://os.mbed.com/blog/entry/How-to-use-emWin-with-Mbed-OS/)
* configurations as follows:
```
mbed config --list
TOOLCHAIN=GCC_ARM
TARGET=MIMXRT1050_EVK
```
```
mbed --version
1.8.3
```
```
arm-none-eabi-gcc --version
arm-none-eabi-gcc (GNU Tools for ARM Embedded Processors 6-2017-q2-update) 6.3.1 20170620 (release) [ARM/embedded-6-branch revision 249437]
```
```
python --version
Python 2.7.14
```
```
pip --version
pip 19.0.1 from c:\python27\lib\site-packages\pip (python 2.7)
```
```
git --version
git version 2.19.0.windows.1
```