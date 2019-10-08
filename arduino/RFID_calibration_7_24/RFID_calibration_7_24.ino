#include <SoftwareSerial.h> //Used for transmitting to the device
SoftwareSerial softSerial(2, 3); //RX, TX

#include "SparkFun_UHF_RFID_Reader2.h" //Library for controlling the M6E Nano module
RFID nano; //Create instance

#define DELAY_TIME 10 //delay time between each tag read
#define NUMBER_READS 100 // number of tag queries for each measurement
#define READER_POWER 2500 //power of the reader in ceni dBm. Max is 27 dBm

/************************************
 *  Setup
 ***********************************/
void setup()
{  
  Serial.begin(115200);
  while (!Serial); //Block until the serial port to come online
  Serial.println("Serial connected. Beginning program...");
  Serial.println("Data is in CSV format below");
  

  if (setupNano(38400) == false) //Configure nano to run at 38400bps
  {
    Serial.println(F("Module failed to respond. Please check wiring."));

    while (1); //Freeze!
  }

  nano.setRegion(REGION_NORTHAMERICA); //Set to North America

  nano.setReadPower(READER_POWER); // level/100 dBm. Higher values may caues USB port to brown out
  //Max Read TX Power is 27.00 dBm and may cause temperature-limit throttling

  waitForSerialInput();
  nano.startReading(); //Begin scanning for tags
}

/************************************
 *    Main Loop
 ************************************/
void loop() {
  for (int i = 0; i < NUMBER_READS; i++){
    while(!searchForTagAndPrintToSerial()); //search until tag is found
    delay(DELAY_TIME);   
  }
  Serial.println("Tag recording finished");
  waitForSerialInput();
}

/************************************
 *    IO Functions
 ************************************/

void waitForSerialInput(){
  while (!Serial.available()); //Wait for user to send a character
  while (Serial.available() ){
    delay(10); //required to avoid looping multiple times
    Serial.read(); 
  }
}

/************************************
 *    RFID Functions
 ************************************/
bool searchForTagAndPrintToSerial(){
  //declare tag data
  int rssi; // the RSSI for this tag read
  long freq; // the frequency this tag was detected at
  long moisture; // the moisture value of this tag
  byte tagEPCBytes; // the number of bytes of EPC from response
  byte EPCHeader;
  byte EPCFertigate;
  
  //search for and get tag data
  bool tagSuccess = false;
  
  if (nano.check() == true){ //Check to see if any new data has come in from module
    byte responseType = nano.parseResponse(); //Break response into tag ID, RSSI, frequency, and timestamp
    
    if (responseType == RESPONSE_IS_KEEPALIVE){ //if has not found tag
      Serial.println(F("Not Found"));
    } else if (responseType == RESPONSE_IS_TAGFOUND){ //has found tag
       /****************************
       * Get appropriate sections of msg array for necessary displayed information
       * Information includes:
       *      - RSSI
       *      - Frequency
       *      - Moisture Value
       *      - EPC
       *      - EPC Desired Moisture Code (EPC Header)
       ***************************/
      rssi = nano.getTagRSSINew(); //Get the RSSI for this tag read
      freq = nano.getTagFreqNew(); //Get the frequency this tag was detected at
      moisture = nano.getMoistureData();
      EPCHeader = nano.getEPCHeader();

      //Print data to serial
      Serial.print(moisture); Serial.print(",");
      Serial.print(rssi); Serial.print(",");
      Serial.print(freq); Serial.print(",");
      Serial.print(EPCHeader); Serial.print(",");
      
      //freq out of a range is sign of a bad read
      //frequency limits taken from datasheet, supported US region frequency 
      //https://cdn.sparkfun.com/assets/4/e/5/5/0/SEN-14066_datasheet.pdf
      if( 917000 > freq || freq > 927200){ 
        Serial.println("phantom");
        return false;
      }
      Serial.println("");
      tagSuccess = true;
      
    } else if (responseType == ERROR_CORRUPT_RESPONSE){
      Serial.println("Bad CRC");
    } else {
      //Unknown response
      Serial.println("Unknown error");
    } //end tag found
  } // end new data check
  return tagSuccess;
} // end function


//Gracefully handles a reader that is already configured and already reading continuously
//Because Stream does not have a .begin() we have to do this outside the library
boolean setupNano(long baudRate)
{
  nano.begin(softSerial); //Tell the library to communicate over software serial port

  //Test to see if we are already connected to a module
  //This would be the case if the Arduino has been reprogrammed and the module has stayed powered
  softSerial.begin(baudRate); //For this test, assume module is already at our desired baud rate
  while(!softSerial); //Wait for port to open

  //About 200ms from power on the module will send its firmware version at 115200. We need to ignore this.
  while(softSerial.available()) softSerial.read();
  
  nano.getVersion();

  if (nano.msg[0] == ERROR_WRONG_OPCODE_RESPONSE)
  {
    //This happens if the baud rate is correct but the module is doing a ccontinuous read
    nano.stopReading();

    Serial.println(F("Module continuously reading. Asking it to stop..."));

    delay(1500);
  }
  else
  {
    //The module did not respond so assume it's just been powered on and communicating at 115200bps
    softSerial.begin(115200); //Start software serial at 115200

    nano.setBaud(baudRate); //Tell the module to go to the chosen baud rate. Ignore the response msg

    softSerial.begin(baudRate); //Start the software serial port, this time at user's chosen baud rate
  }

  //Test the connection
  nano.getVersion();
  if (nano.msg[0] != ALL_GOOD) return (false); //Something is not right

  //The M6E has these settings no matter what
  nano.setTagProtocol(); //Set protocol to GEN2

  nano.setAntennaPort(); //Set TX/RX antenna ports to 1

  return (true); //We are ready to rock
}
