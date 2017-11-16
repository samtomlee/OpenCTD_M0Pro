//Wire and MS5803_I2C libraries for communicating with the pressure sensor. 

#include <Wire.h> //used for the conductivity sensor too              
#include <MS5803_14.h> // newer library for pressure sensor with M0 compatibility

//OneWire is used to get data from both temperature and conductivity sensors
#include <OneWire.h>  //CAN WE MAKE THIS LIBRARY SMALLER?

//DallasTemperature is used to query the temeperature sensors           
#include <DallasTemperature.h>   //CAN WE MAKE THIS LIBRARY SMALLER?

// For the SD card reader.
#include <SPI.h>               
#include <SD.h>             

// Declare global variables.
float tempA;
float tempB;
float tempC;

char EC_data[48];    // A 48 byte character array to hold incoming data from the conductivity circuit. 
char *EC;            // Character pointer for string parsing.

byte received_from_sensor = 0;  // How many characters have been received.


OneWire oneWire(6);        // Define the OneWire port for temperature.
DallasTemperature tsensors(&oneWire);  // Define DallasTemperature input based on OneWire.
MS_5803 psensor(4096);          // Define pressure sensor.

// Starts it up.

void setup(void) {
  
  Serial.begin(9600);   // Set baud rate.
  Wire.begin(); // conductivity circuit.
  
  pinMode(11, OUTPUT);  // Set data output pin for the SD card reader.

  while(!Serial){}
  
  // Test to see if the SD card reader is functioning.
  if (!SD.begin(4)) {
    Serial.println("Card failed");  // Log to serial monitor if SD card doesn't work.
    return; 
  }
  delay(500);  // Wait half a second before continuing.

  psensor.initializeMS_5803(); //initialize pressure sensor
  tsensors.begin();  // Intialize the temperature sensors.

  //Headers for the columns
  Serial.println("Pressure     Temp in Celcius     Conductivity");
  Serial.println(" (mbar)");
  
  delay(250);       // Wait a quarter second to continue.
}

void loop(void) {
  
  // Read any pending data from the EC circuit.
  
  Wire.beginTransmission(100); //address for conductivity
  Wire.write("R");
  Wire.endTransmission();
  delay(600); 
  Wire.requestFrom(100,48);

  
  if (Wire.available() > 0) {
    //received_from_sensor = Wire.readBytesUntil(13, EC_data, 48);
    Wire.read(); // ignores the first byte, as it is NULL or blank
    for(int i = 0; i < 48; i++){
      char reading = Wire.read();

      if(reading == 13 || Wire.available() <= 0){break;}
      EC_data[i] = reading;
      
      received_from_sensor = i+1;
    }
    
    // Null terminate the data by setting the value after the final character to 0.
    EC_data[received_from_sensor] = 0;
  }
  else{Serial.println("DOES NOT READ WIRE");}
  
  // Parse data, if EC_data begins with a digit, not a letter (testing ASCII values).
  if ((EC_data[0] >= 48) && (EC_data[0] <= 57)) {
    parse_data();
  }

  delay(10);  // Wait 10 milliseconds.

  // Read temperature sensors.
  tsensors.requestTemperatures();
  tempA = tsensors.getTempCByIndex(0);
  tempB = tsensors.getTempCByIndex(1);
  tempC = tsensors.getTempCByIndex(2);

  //Read the pressure sensor
  psensor.readSensor();


  // Log to the SD card...
  File dataFile = SD.open("datalog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print(psensor.pressure());
    dataFile.print("  ");
    dataFile.print(tempA);
    dataFile.print("  ");
    dataFile.print(tempB);
    dataFile.print("  ");
    dataFile.print(tempC);
    dataFile.print("  ");
    dataFile.println(EC);
    dataFile.close();
  }
  else {

  }

  // Log to the serial monitor.
  Serial.print(psensor.pressure());
  Serial.print("    "); 
  Serial.print(tempA);
  Serial.print("  ");
  Serial.print(tempB);
  Serial.print("  ");
  Serial.print(tempC);
  Serial.print("       ");
  Serial.print(EC);
  Serial.println("");

  delay(50);  // Wait 50 milliseconds.
}


// Parses data from the EC Circuit.
void parse_data() {

  EC = strtok(EC_data, ",");                  

}
