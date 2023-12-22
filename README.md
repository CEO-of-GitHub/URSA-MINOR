# CHANGELOG

> **v1.1 (CURRENT):**
- Added _W25Q SPI Flash Chip_. Strong vibrations of rocket while in-flight may corrupt data logged into MicroSD since it is locked in place mechanically, hence a soldered alternative, the SPI Flash Chip, should be used.
- Added _NRF24L01 Transceiver Module_. Obtains data from rocket wirelessly for real-time tracking in a telemetry dashboard. 
- MicroSD card module will still be included, but only used for obtaining data from rocket after flight.
 
> **v1.0 (UNFIN. DISCON.):**
- Uses the following: _Arduino Nano, GY-91 Module (MPU9250 + BMP280), BN-880 GPS Module, EG1218 E-Switch, HiLetgo MicroSD Card Reader_
  - _Arduino Nano:_ main flight computer
  - _GY-91 Module:_ measures 3-axis acceleration, 3-axis tilt, direction of flight (compass), temperature, air pressure, and altitude
  - _BN-880 GPS Module:_ tracks coordinates of rocket
  - _EG1218 E-Switch:_ for activating/deactivating data logging
  - _HiLetgo MicroSD Card Reader:_ logs data into MicroSD card during flight
- Setup Code:
  - Begin serial, then initialize sensors. Program halt if initialization incomplete.
- Loop Code:
  - Check for E-Switch switch status, then either make new datalog file or write over existing datalog file depending on whether switch is OFF or ON during previous loop iteration (if at first iteration, it is considered to be OFF).
  - Collect data from GY-91 module & BN-880 GPS module.
  - Identify most recent E-Switch status, to be used as reference for next loop iteration.
  - Delay of 100 ms.
