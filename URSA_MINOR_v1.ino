/*
  Developed using Arduino IDE 2.2.1
  Library Versions:
    SD 1.2.4
    Adafruit BMP280 Library 2.6.8
    Adafruit BusIO 1.14.5
    Adafruit Unified Sensor 1.1.14
    MPU9250_asukiaaa 1.5.13
    TinyGPSPlus 1.0.3
*/

// --> libraries
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <MPU9250_asukiaaa.h>
#include <Adafruit_BMP280.h>
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
// NOTE: "Adafruit Unified Sensor" and "Adafruit BusIO" libraries have to be installed into the IDE for this to be used
// GY-91 LIBRARIES TUTORIAL: https://electropeak.com/learn/interfacing-gy-91-9-axis-mpu9250-bmp280-module-with-arduino/

// --> defines & constants
const int eswitch_pin = 2;
const int microSD_MISO = 50;  // 12 for Nano, 50 for Mega
const int microSD_MOSI = 51;  // 11 for Nano, 51 for Mega
const int microSD_SCK = 52;   // 13 for Nano, 52 for Mega
const int microSD_CS = 53;    // 10 for Nano, 53 for Mega
const int gy91_SCL = A5;
const int gy91_SDA = A4;
static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 38400;

// --> global variables
// GENERAL
bool STATUS_INIT = true;
// EG-1218 E-Switch
int eswitch_last_status = false;  // status of E-Switch right before most recent function call
int status;
// MicroSD Card Module
File datalog;
File datalog_entry;
File directory;
String datalog_filename = String("ursalogX.csv");  // NOTE: For whatever reason, there is a 12-character limit to the string one can use as an argument to File open()
String filename;
int datalog_count;
unsigned long time_start;   // millis() of when stopwatch started
unsigned long time_record;  // millis() of time in stopwatch we want to observe
// For stopwatch: we observe time via time = time_record - time_start
// GY-91 Module
Adafruit_BMP280 BMPsensor; // I2C
MPU9250_asukiaaa MPUsensor;
float aX, aY, aZ, aSqrt, gX, gY, gZ, mDirection, mX, mY, mZ, temp, pres, alt;
//BN-880 GPS Module
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

// --> user-defined function prototypes

// EG-1218 E-Switch
bool ACT_eswitch();
/*
  BELONGS TO: setup()
  PURPOSE: Sets pinmode for e-switch pin to INPUT (to activate it)
  NOTE: Serial.begin also needs to be declared in setup() for this to work
*/
bool eswitch();
/*
  BELONGS TO: loop()
  PURPOSE: Collects input from EG1218 e-switch, returns 0 (deactivate data collection)
    or 1 (activate data collection).
*/

// MicroSD Card Module
bool ACT_microSD();
/*
  BELONGS TO: setup()
  PURPOSE: Sets pinmodes for microSD, shows indicator that it works
  NOTE: For this to work, MicroSD card must be formatted to FAT16 or FAT32, with 1024 bytes of allocation unit size
*/
void CHECK_microSD();
/*
  BELONGS TO: loop()
  PURPOSE: A repeatedly-run function that helps determine course of action of microSD module, given the ON or OFF states of the e-switch
  If: eswitch is set to ON, for the first time - then create a new .csv file for data collection
    Create file named "ursa_minor_logX.csv",where "X" in logX is
      n+1, where n = number of files in the microSD
    Run stopwatch (set value for time_start)
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, for the first time - then data collection is done for the session, process is ended and data is saved to microSD
    ___
  Else If: eswitch is set to ON, NOT for the first time - then continue data collection as usual
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, NOT for the first time - then nothing has to be done by the microSD module
    Do nothing
*/
String FILENAME_microSD();
/*
  PURPOSE: To provide a filename for the .csv file in the MicroSD
  PROCESS:
    Counts number of files in MicroSD
    Uses said number as X in "ursa_minor_logX.csv"
    Returns string of filename
*/
void HEADER_microSD(File csvfile);
/*
  PURPOSE: Create a header (1st row text) for the table being made in the .csv file
*/
void microSD(File csvfile);
/*
  PURPOSE: Collect data from GY-91 & BN-880 GPS module, and organizes them into the .csv datalog file
  PROCESS:
    Open file based on filename in string value of datalog_filename
    If: datalog file is empty
      Start clock by defining time_start = millis() (for reference with the data)
    Collect instantaneous data from GY-91 module
    Collect instantaneous data from BN-880 GPS module
*/

// GY-91 Module
bool ACT_GY91();
/*
  BELONGS TO: setup()
  PURPOSE: To initialize both MPU9250 and BMP280 components of GY-91 module
  REFERENCE: https://github.com/asukiaaa/MPU9250_asukiaaa/blob/master/README.md
*/
void collectMPU_GY91();
/*
  BELONGS TO: loop()
  PURPOSE: Collects values from MPU9250 component of GY-91 (accelerometer, gyrometer, and compass), assigns them to specified variables
  REFERENCE: https://github.com/asukiaaa/MPU9250_asukiaaa/blob/master/README.md
*/
void collectBMP_GY91();
/*
  BELONGS TO: loop()
  PURPOSE: Collects values from BMP280 component of GY-91 (temperature, pressure, and altitude), assigns them to specified variables
  REFERENCE: https://github.com/asukiaaa/MPU9250_asukiaaa/blob/master/README.md
*/
void PRINT_GY91();
/*
  BELONGS TO: loop()
  PURPOSE: For printing the input values of GY-91 into Serial monitor, mostly used for testing.
*/

// BN-880 GPS Module
bool ACT_BN880();
/*
  BELONGS TO: setup()
  PURPOSE: To initialize BN-880 module
*/
void BN880();
/*
  BELONGS TO: loop()
  PURPOSE: To collect data from module: latitude, longitude, and compass direction (from magnetometer)
  ALLEGED PROBLEM: GPS has to be used in outdoors so that GPS signals may be collected by module,
    which is why there doesn't seem to be any results when testing indoors
*/

// --> setup()
void setup() {
  Serial.begin(115200);
  while (!Serial) {
    Serial.println("\n>> Serial monitor not working or something. Try again lol.\n");
  }
  Serial.println("\n>> Initialization BEGIN\n");
  STATUS_INIT = ACT_eswitch();
  STATUS_INIT = STATUS_INIT && ACT_microSD();
  STATUS_INIT = STATUS_INIT && ACT_GY91();
  STATUS_INIT = STATUS_INIT && ACT_BN880();
  if (!STATUS_INIT) {
    Serial.println("\n>> Initialization INCOMPLETE.");
    while (1);
  }
  Serial.println("\n>> Initialization COMPLETE.\n");
}

// --> loop()
void loop() {
  CHECK_microSD();
  collectMPU_GY91();
  collectBMP_GY91();
  PRINT_GY91();
  eswitch_last_status = eswitch();
  //Serial.println(eswitch_last_status);
  delay(100);
}


// --> user-defined functions

// EG-1218 E-Switch
bool ACT_eswitch() {
  pinMode(eswitch_pin, INPUT);
  Serial.println("EG-1218 E-Switch initialization SUCCESS.");
  return true;
}
bool eswitch() {
  int eswitch_state = digitalRead(eswitch_pin);
  if (eswitch_state == HIGH) {
    return true;
  } else if (eswitch_state == LOW) {
    return false;
  }
}

// MicroSD Card Module
bool ACT_microSD() {
  pinMode(microSD_CS, OUTPUT);
  digitalWrite(microSD_CS, HIGH);
  if (!SD.begin(microSD_CS)) {
    Serial.println("MicroSD card initialization FAILED.");
    return false;
  }
  Serial.println("MicroSD card initialization SUCCESS.");
  return true;
}
void CHECK_microSD() {
  status = eswitch();
  if (status == true && eswitch_last_status == false) {
    filename = FILENAME_microSD();
    datalog = SD.open(filename, FILE_WRITE);
    HEADER_microSD(datalog);
    microSD(datalog);
    datalog.close();
  } else if (status == true && eswitch_last_status == true) {
    datalog = SD.open(filename, FILE_WRITE);
    microSD(datalog);
    datalog.close();
  }
}
String FILENAME_microSD() {
  datalog_count = 0;
  directory = SD.open("/");
  while (1) {
    File datalog_entry = directory.openNextFile();
    if (!datalog_entry)  // No more files remaining
    {
      datalog_filename = "ursa" + String(datalog_count);
      datalog_filename = datalog_filename + ".csv";
      datalog_count = 0;  // To reset the datalog_count, in case it will be called again
      directory.close();
      return datalog_filename;  // Return string of the filename with X = datalog_count
    }
    datalog_count++;
    // REFERENCE: http://www.bwrealconsulting.com/articles/view/files-on-sd-card-directory-with-arduino
  }
}
void HEADER_microSD(File csvfile) {
  // ACCELEROMETER (in G's)
  csvfile.print("aX"); 
  csvfile.print(",");
  csvfile.print("aY"); 
  csvfile.print(",");
  csvfile.print("aZ"); 
  csvfile.print(",");
  csvfile.print("aSqrt"); 
  csvfile.print(",");
  // GYROMETER (in Degrees/Second)
  csvfile.print("gX");
  csvfile.print(",");
  csvfile.print("gY");
  csvfile.print(",");
  csvfile.print("gZ");
  csvfile.print(",");
  // MAGNETOMETER (in Degrees), range is from (-180,180), where 0 is North and lim+-180 is South
  csvfile.print("mX");
  csvfile.print(",");
  csvfile.print("mY");
  csvfile.print(",");
  csvfile.print("mZ");
  csvfile.print(",");
  csvfile.print("mDirection");
  csvfile.print(",");
  // BMP280 (Temperature in Celsius, Pressure in Pa, Altitude in Meters)
  csvfile.print("Temperature");
  csvfile.print(",");
  csvfile.print("Pressure");
  csvfile.print(",");
  csvfile.println("Altitude");
  // Header for GPS is WIP
}
void microSD(File csvfile) {
  // 1. DATA FROM GY-91
  // ACCELEROMETER (in G's)
  csvfile.print(aX); 
  csvfile.print(",");
  csvfile.print(aY); 
  csvfile.print(",");
  csvfile.print(aZ); 
  csvfile.print(",");
  csvfile.print(aSqrt); 
  csvfile.print(",");
  // GYROMETER (in Degrees/Second)
  csvfile.print(gX);
  csvfile.print(",");
  csvfile.print(gY);
  csvfile.print(",");
  csvfile.print(gZ);
  csvfile.print(",");
  // MAGNETOMETER (in Degrees), range is from (-180,180), where 0 is North and lim+-180 is South
  csvfile.print(mX);
  csvfile.print(",");
  csvfile.print(mY);
  csvfile.print(",");
  csvfile.print(mZ);
  csvfile.print(",");
  csvfile.print(mDirection);
  csvfile.print(",");
  // BMP280 (Temperature in Celsius, Pressure in Pa, Altitude in Meters)
  csvfile.print(temp);
  csvfile.print(",");
  csvfile.print(pres);
  csvfile.print(",");
  csvfile.println(alt);
  // 2. DATA FROM BN-880 (WIP)
  return;
}

// GY-91 Module
bool ACT_GY91() {
  Wire.begin();  // Include "gy91_SDA,gy91_SCL" in function arguments () if using ESP32 instead of Arduino
  // PART A: MPU9250 Component
  MPUsensor.setWire(&Wire);
  MPUsensor.beginAccel();
  MPUsensor.beginGyro();
  MPUsensor.beginMag();
  // PART B: BMP20 Component
  if (!BMPsensor.begin(0x76)) { // 0x76 is the I2C address of a BMP280
    Serial.println("GY-91 Module initialization FAILED.");
    return false;
  } else {
    BMPsensor.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
    Serial.println("GY-91 Module initialization SUCCESS.");
    return true;
  }
}
void collectMPU_GY91() {
  // 1. Accelerometer
  MPUsensor.accelUpdate();
  aX = MPUsensor.accelX();
  aY = MPUsensor.accelY();
  aZ = MPUsensor.accelZ();
  aSqrt = MPUsensor.accelSqrt();
  // 2. Gyrometer
  MPUsensor.gyroUpdate();
  gX = MPUsensor.gyroX();
  gY = MPUsensor.gyroY();
  gZ = MPUsensor.gyroZ();
  // 3. Magnetometer (Compass)
  MPUsensor.magUpdate();
  mX = MPUsensor.magX();
  mY = MPUsensor.magY();
  mZ = MPUsensor.magZ();
  mDirection = MPUsensor.magHorizDirection();
}
void collectBMP_GY91() {
  temp = BMPsensor.readTemperature(); // in Celsius
  pres = BMPsensor.readPressure(); // in Pa
  alt = BMPsensor.readAltitude(1013.25); // in m
  // NOTE: To use altitude, a parameter must be put in the function that indicates the current hectopascal pressure (hPa) at sea level.
  // Example: BMPsensor.readAltitude(1013.25) ---> this means pressure at sea level is 1013.25 hPa
}
void PRINT_GY91() {
  Serial.println("ACCEL:                           GYRO:                       MAGN:                               TEMP,PRES,ALT:");
  Serial.print(aX);
  Serial.print("   ");
  Serial.print(aY);
  Serial.print("   ");
  Serial.print(aZ);
  Serial.print("   ");
  Serial.print(aSqrt);
  Serial.print("      ");
  Serial.print(gX);
  Serial.print("   ");
  Serial.print(gY);
  Serial.print("   ");
  Serial.print(gZ);
  Serial.print("      ");
  Serial.print(mX);
  Serial.print("   ");
  Serial.print(mY);
  Serial.print("   ");
  Serial.print(mZ);
  Serial.print("   ");
  Serial.print(mDirection);
  Serial.print("      ");
  Serial.print(temp);
  Serial.print("   ");
  Serial.print(pres);
  Serial.print("   ");
  Serial.println(alt);
}

// BN-880 GPS Module
bool ACT_BN880() {
  ss.begin(GPSBaud);
  Serial.println("BN-880 GPS Module initialization SUCCESS.");
  return true;
}
















