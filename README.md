# CHANGELOG

> **v1.1 (WIP):**
- For data logging: use W25Q SPI Flash Chip instead of MicroSD card. Strong vibrations of rocket while in-flight may corrupt data logged into MicroSD since it is locked in place mechanically, hence a soldered alternative, the SPI Flash Chip, should be used. 
 
> **v1.0 (CURRENT):**
- Uses the following: _Arduino Nano, GY-91 Module (MPU9250 + BMP280), BN-880 GPS Module, EG1218 E-Switch, HiLetgo MicroSD Card Reader_
  - _Arduino Nano:_ main flight computer
  - _GY-91 Module:_ measures 3-axis acceleration, 3-axis tilt, direction of flight (compass), temperature, air pressure, and altitude
  - _BN-880 GPS Module:_ tracks coordinates of rocket
  - _EG1218 E-Switch:_ for activating/deactivating data logging
  - _HiLetgo MicroSD Card Reader:_ logs data into MicroSD card during flight
