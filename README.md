> **CHANGELOG:**

**v1.1 (WIP):**
- For data logging: use W25Q SPI Flash Chip instead of MicroSD card. Strong vibrations of rocket while in-flight may corrupt data logged into MicroSD since it is locked in place, hence a soldered alternative, the SPI Flash Chip, should be used. 

**v1.0 (CURRENT):**
- Uses the following: _Arduino Nano, GY-91 Module (MPU9250 + BMP280), BN-880 GPS Module, EG1218 E-Switch, HiLetgo MicroSD Card Reader_
- Arduino Nano: main flight computer
- GY-91 Module: measures 3-axis acceleration, 3-axis tilt, direction of flight (compass), temperature, air pressure, and altitude
- BN-880 GPS Module: tracks coordinates of rocket
- EG1218 E-Switch: for activating/deactivating data logging
- HiLetgo MicroSD Card Reader: logs data into MicroSD card during flight
