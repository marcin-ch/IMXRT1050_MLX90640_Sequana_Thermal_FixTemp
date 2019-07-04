# IMXRT1050_MLX90640_Sequana_Thermal_FixTemp
Contactless temperature measurement and thermal imager in Arduino-compatible shield format. Software for IMXRT1050-EVK.

# Prerequisites
[Mbed CLI Windows Installer](https://github.com/ARMmbed/mbed-cli-windows-installer/releases/latest)

All the necessary dependencies should be installed with this installer, however if any issues I strongly recommend to visit this [Installation and setup for Windows](https://os.mbed.com/docs/mbed-os/latest/tools/windows.html) page.

# Hardware preparation
Workaround for i.MXRT1050 eval board based on [Important Notes](https://os.mbed.com/platforms/MIMXRT1050-EVK/)
* it means connect D14 (SDA) to A4 and D15 (SCL) to A5

# Working with Mbed CLI project
1. Open command line and go to the location where you want to import this project to 
2. Import this project
```
mbed import https://github.com/marcin-ch/IMXRT1050_MLX90640_Sequana_Thermal_FixTemp
cd IMXRT1050_MLX90640_Sequana_Thermal_FixTemp
```
3. Check what eval board is connected to your computer `mbed detect`
```
[mbed] Detected MIMXRT1050_EVK, port COM26, mounted D:, interface version 0244:
[mbed] Supported toolchains for MIMXRT1050_EVK
+----------------+-----------+-----------+-----------+-----------+-----------+
| Target         | mbed OS 2 | mbed OS 5 |    ARM    |  GCC_ARM  |    IAR    |
+----------------+-----------+-----------+-----------+-----------+-----------+
| MIMXRT1050_EVK | Supported | Supported | Supported | Supported | Supported |
+----------------+-----------+-----------+-----------+-----------+-----------+
Supported targets: 1
Supported toolchains: 3
```
4. Select toolchain `mbed toolchain GCC_ARM`
5. Select target `mbed target MIMXRT1050_EVK`
6. Check current configuration `mbed config --list`
```
TOOLCHAIN=GCC_ARM
TARGET=MIMXRT1050_EVK
```
7. Compile `mbed compile --profile mbed-os/tools/profiles/debug.json`
* first compilation may take a while
8. From ***BUILD\MIMXRT1050_EVK\GCC_ARM-DEBUG*** directory copy ***IMXRT1050_MLX90640_Sequana_Thermal_FixTemp.bin*** file to the eval board with drag & drop method
* the eval board should be seen by your system as removable storage
9. Reset the eval board
10. Project should be now up and running