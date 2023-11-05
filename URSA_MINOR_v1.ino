/*
  Developed using Arduino IDE 2.2.1
  Library Versions:
    SD 1.2.4
*/

// --> libraries
#include <SD.h>
#include <SPI.h>

// --> defines & constants
const int eswitch_pin = 2;
const int microSD_MISO = 50;    // 12 for Nano, 50 for Mega
const int microSD_MOSI = 51;    // 11 for Nano, 51 for Mega
const int microSD_SCK = 52;    // 13 for Nano, 52 for Mega
const int microSD_CS = 53;    // 10 for Nano, 53 for Mega

// --> global variables
// GENERAL
bool STATUS_INIT = true;
// EG-1218 E-Switch
int eswitch_state;    // Input variable for E-Switch
int eswitch_last_status = false;
int status;
// MicroSD Card Module
File datalog;
File datalog_entry;
String datalog_filename = String("ursa_minor_logX.csv");
int datalog_count;
unsigned long time_start;    // millis() of when stopwatch started
unsigned long time_record;    // millis() of time in stopwatch we want to observe
// For stopwatch: we observe time via time_record - time_start


// --> user-defined function prototypes

// GY-91
// void ACT_GY91();
/*
  BELONGS TO: setup()
*/

// BN-880 GPS

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
  If: eswitch is set to ON, for the first time
    Create file named "ursa_minor_logX.csv",where "X" in logX is
      X+1 number of files in the microSD
    Run stopwatch (set balue for time_start)
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, for the first time
    ___
  Else If: eswitch is set to ON, NOT for the first time
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, NOT for the first time
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
void microSD();
/*
  PURPOSE: Collect data from GY-91 & BN-880 GPS module, and organizes them into the .csv datalog file
  PROCESS:
    Open file based on filename in string value of datalog_filename
    If: datalog file is empty
      Start clock by defining time_start = millis() (for reference with the data)
    Collect instantaneous data from GY-91 module
    Collect instantaneous data from BN-880 GPS module
*/

// --> setup()
void setup()
{
  Serial.begin(9600);
  Serial.println("\n>> Initialization BEGIN\n");
  STATUS_INIT = ACT_eswitch();
  STATUS_INIT = ACT_microSD() && STATUS_INIT;
  if (!STATUS_INIT)
  {
    Serial.println("\n>> Initialization INCOMPLETE.");
    while(1);
  }
  Serial.println("\n>> Initialization COMPLETE.\n");
}

// --> loop()
void loop()
{
  //CHECK_microSD();
  eswitch_last_status = eswitch();
  Serial.println(eswitch_last_status);
  delay(50000);
}


// --> user-defined functions

// EG-1218 E-Switch
bool ACT_eswitch()
{
  pinMode(eswitch_pin, INPUT);
  Serial.println("EG-1218 E-Switch initialization SUCCESS.");
  return true;
}
bool eswitch()
{
  eswitch_state = digitalRead(eswitch_pin);
  if (eswitch_state == HIGH)
  {
    return true;
  }
  else if (eswitch_state == LOW)
  {
    return false;
  }
}

// MicroSD Card Module
bool ACT_microSD()
{
  pinMode(microSD_CS,OUTPUT);
  digitalWrite(microSD_CS,HIGH);
  if (!SD.begin(microSD_CS))
  {
    Serial.println("MicroSD card initialization FAILED.");
    return false;
  }
  Serial.println("MicroSD card initialization SUCCESS.");
  return true;
}
void CHECK_microSD()
{
  status = eswitch();
  if (status == true && eswitch_last_status == false)
  {
    = SD.open(FILENAME_microSD);
  }
}

/*
  BELONGS TO: loop()
  If: eswitch is set to ON, for the first time
    Create file named "ursa_minor_logX.csv",where "X" in logX is
      X+1 number of files in the microSD
    Run stopwatch (set balue for time_start)
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, for the first time
    Call function DEACT_microSD(datalog_filename)
  Else If: eswitch is set to ON, NOT for the first time
    Call function microSD(datalog_filename)
  Else If: eswitch is set to OFF, NOT for the first time
    Do nothing
*/

String FILENAME_microSD()
{
  datalog_count = 0;
  datalog = SD.open("/");
  while(1)
	{
 		File datalog_entry = datalog.openNextFile();
		if (!datalog_entry)    // No more files remaining
		{
			datalog_filename = "ursa_minor_log" + datalog_count;
      datalog_filename = datalog_filename + ".csv";
      datalog_count = 0;     // To reset the datalog_count, in case it will be called again
      return datalog_filename;    // Return string of the filename with X = datalog_count
		}
		datalog_count++;
    // REFERENCE: http://www.bwrealconsulting.com/articles/view/files-on-sd-card-directory-with-arduino
  }
}



/*
  PURPOSE: To provide a filename for the .csv file in the MicroSD
  PROCESS:
    Counts number of files in MicroSD
    Uses said number as X in "ursa_minor_logX.csv"
    Returns string of filename
*/




