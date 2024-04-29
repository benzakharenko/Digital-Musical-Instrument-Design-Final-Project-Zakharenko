#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <Adafruit_LSM6DSOX.h>
#include <Arduino.h>
#include <BLEMidi.h>

//Defining Input and Output Pins:
// Configuration
const int pot_pin = A0;
const uint8_t midi_channel = 0;
const uint8_t controller_number = 17;
const float c = 0.2;  // low pass filter coefficient (0 < c <= 1)

//reset pin
const int reset_pin = 0;

//mapping mode button
const int mapping_mode_button = 1;
int mapping_mode = 0;
const int mapping_mode_max = 4;
const int delay_time = 2000;
unsigned long timeElapsed = 0;

//initializing global variables for gyroscope integration
float xDisplacement = 0;
float yDisplacement = 0;
float zDisplacement = 0;

//defining things from the gyroscope:
typedef enum {
  I2C_PROTOCOL,
  SPI_PROTOCOL,
  SPI_CSONLY_PROTOCOL
} SerialProtocol;

// call the class for the LMS6DSOX
Adafruit_LSM6DSOX sox;
SerialProtocol mode = I2C_PROTOCOL;


//initializing functions to be defined later
void onNoteOn();
void onNoteOff();
void onControlChange();
void printAngularVelocityX(sensors_event_t gyro);
void printAngularVelocityY(sensors_event_t gyro);
void printAngularVelocityZ(sensors_event_t gyro);
void connected() {
  Serial.println("Connected");
}

//calling the DRV265 class
Adafruit_DRV2605 drv;





void setup() {
  Serial.begin(115200);

  //defining the pin of the flex sensor
  pinMode(pot_pin, INPUT);

  //defining the pin of the reset button
  pinMode(reset_pin, INPUT);

  //defning the pin of the mapping mode button
  pinMode(mapping_mode_button, INPUT);

  // initialize the vibration motor
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_INTTRIG);

  // Initialize the BLE MIDI server
  BLEMidiServer.begin("Wave Control");
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback([]() {  // To show how to make a callback with a lambda function
    Serial.println("Disconnected");
  });

// begin the LMS6DSOX communication
sox.begin_I2C();

}





void loop() {
  // continually check the loop for Note On and Note Off messages
  BLEMidiServer.setNoteOnCallback(onNoteOn);
  BLEMidiServer.setNoteOffCallback(onNoteOff);
  BLEMidiServer.setControlChangeCallback(onControlChange);

  //mapping mode toggle
  if (digitalRead(mapping_mode_button) == HIGH && millis() - timeElapsed > delay_time)
  {
    if (mapping_mode == mapping_mode_max)
    {
      mapping_mode = 0;
      Serial.println("NO MAPPING MODE ENABLED");
    }
    else
    {
      mapping_mode++;
      Serial.print("Mapping Mode: " );
      Serial.println(mapping_mode);
    }
    timeElapsed = millis();
  }

  //Flex Sensor-----------------------------------------------------------------------------------------------------------------------------------------------------------
  if (mapping_mode == 0 || mapping_mode == 1)
  {
    static float y = 0;
    static uint8_t old_y = 0;

    //Serial.println(analogRead(pot_pin));

    float x = map(analogRead(pot_pin), 3100, 2600, 0, 127);
    y += c * (x - y);  // simple low pass IIR filter (see tutorial here : https://tomroelandts.com/articles/low-pass-single-pole-iir-filter )
                      // (when we read the potentiometer, the value flickers a little bit all the time, so we need to do filter it to avoid sending midi messages all the time)
                      // another solution may be better than this type of filter, it is left as an exercise to the reader :)
    if (BLEMidiServer.isConnected() && (uint8_t)y != old_y) {
      BLEMidiServer.controlChange(midi_channel, controller_number, y);
      old_y = (uint8_t)y;
    }
  }
  //---------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  // gyroscopic sensors
  sensors_event_t accel, gyro, temp;
  sox.getEvent(&accel, &gyro, &temp);

  if (mapping_mode == 0 || mapping_mode == 2)
  {
    printAngularVelocityX(gyro);
  }
  if (mapping_mode == 0 || mapping_mode == 3)
  {
    printAngularVelocityY(gyro);
  }
  if (mapping_mode == 0 || mapping_mode == 4)
  {
    printAngularVelocityZ(gyro);
  }
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

}






// Receiving Incoming MIDI----------------------------------------------------------------------------------------------------------------------------------------------------
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  Serial.printf("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  drv.setWaveform(0, 47);
  drv.go();
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  Serial.printf("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {
  Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Gyroscope to Displacement to MIDI--------------------------------------------------------------------------------------------------------------------------------------------

void printAngularVelocityX(sensors_event_t gyro) {
  // Angular velocity is measured in radians/s
  float x = gyro.gyro.x;
  // defining variables necessary for the rolling average
  float xArea[10];
  int TimeDelay = millis() % 100;
  Serial.print("TimeDelay: ");
  Serial.println(TimeDelay);
  //integration calculation for x and getting a rolling average
  if (TimeDelay % 10 == 0)
  {
  int CurrentIndex = TimeDelay * 0.1;
  float xInMs = x * 0.001; //convert to rad/ms
  xArea[CurrentIndex] = xInMs * 10; //integration calculation as rectangle estimation
  Serial.print("Current Index: ");
  Serial.println(CurrentIndex);
  Serial.print("Index Value: ");
  Serial.println(xArea[CurrentIndex]);
  }
  if (TimeDelay >= 99)
  {
    int xAreaTotal = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
      xAreaTotal += xArea[i];
    }
    int xArea_Average = xAreaTotal / 10;
    xDisplacement = xDisplacement + xArea_Average;

    //MIDI DELIVERY
    if (x > 0.05 || x < -0.05)
    { // -30 to 30
      int xDeltaToMIDI = map(xDisplacement, 0, 11, 0, 127);
      BLEMidiServer.controlChange(0, 20, xDeltaToMIDI);
    }
    Serial.println("I Have Calculated the Average!!!");

  }

  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    xDisplacement = 0;
  }

  //Serial.print("x/ ");
  //Serial.println(xDisplacement);

}

void printAngularVelocityY(sensors_event_t gyro) {
  // Angular velocity is measured in radians/s
  float y = gyro.gyro.y;

  //integration calculation for y
  float yInMs = y * 0.001; //convert to rad/ms
  float yArea = yInMs * 200; //integration calculation as rectangle estimation
  yDisplacement = yDisplacement + yArea;

  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    yDisplacement = 0;
  }

  delay(50);


  if (y > 0.1 || y < -0.1)
  { //scale -25 to 25
    int yDeltaToMIDI = map(yDisplacement, -25, 25, 0, 127);
    //BLEMidiServer.controlChange(0, 21, yDeltaToMIDI);
  }


}

void printAngularVelocityZ(sensors_event_t gyro) {
  // Angular velocity is measured in radians/s
  float z = gyro.gyro.z;

  //integration calculation for z
  float zInMs = z * 0.001; //convert to rad/ms
  float zArea = zInMs * 200; //integration calculation as rectangle estimation    
  zDisplacement = zDisplacement + zArea;

  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    zDisplacement = 0;
  }

  delay(50);

  if (z > 0.1 || z < -0.1)
  { //scale -25 to 25
    int yDeltaToMIDI = map(yDisplacement, -25, 25, 0, 127);
    BLEMidiServer.controlChange(0, 22, yDeltaToMIDI);
  }

}

