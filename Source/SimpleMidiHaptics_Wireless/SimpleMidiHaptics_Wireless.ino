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


int Timer = 0;
//reset pin
const int reset_pin = 2;

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


unsigned long xTimeEllapsed = 0;
unsigned long yTimeEllapsed = 0;
unsigned long zTimeEllapsed = 0;


//rolling average code definitions
#define NUM_READINGS 10
struct MotionParameter {
  float readings[NUM_READINGS];
  float total = 0;
  int readIndex = 0;
};

MotionParameter gyroX;
MotionParameter gyroY;
MotionParameter gyroZ;

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
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp);
void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp);
void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp);
void connected() {
  Serial.println("Connected");
}

//calling the DRV265 class
Adafruit_DRV2605 drv;


// One million microseconds in one second
float loopRateInMicroseconds = 20000;   // 10000us = 10ms



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

  // Initialize all reading to 0
  for (int i = 0; i < NUM_READINGS; i++) {
    gyroX.readings[i] = 0;
    gyroY.readings[i] = 0;
    gyroZ.readings[i] = 0;
  }

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
    if (BLEMidiServer.isConnected() && (uint8_t)y != old_y) 
    {
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

  delayMicroseconds(loopRateInMicroseconds);
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

void printAngularVelocityX(sensors_event_t gyro) 
{
  // Angular velocity is measured in radians/s
  float x = gyro.gyro.x;

  //integration calculation for y
  updateMovingAverage(&gyroX, x);

  float xInMs = x * 0.001; //convert to rad/ms
  float xArea = xInMs * 10; //integration calculation as rectangle estimation

  xDisplacement = xDisplacement + xArea;

  //MIDI DELIVERY
  if ((x > 0.05 || x < -0.05))
  { // -30 to 30
    int xDeltaToMIDI = map(xDisplacement, 0, 11, 0, 127);
    BLEMidiServer.controlChange(0, 20, xDeltaToMIDI);
    Serial.print("x Displacement/ ");
    Serial.println(xDisplacement);
  }


  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    for (int i = 0; i < NUM_READINGS; i++) 
    {
      gyroX.readings[i] = 0;
    }
    xDisplacement = 0;
  }

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


 // Serial.print("y/ ");
 // Serial.println(yDisplacement);
  if (millis() - yTimeEllapsed > 50 && y > 0.1 || y < -0.1)
  { //scale -25 to 25
    int yDeltaToMIDI = map(yDisplacement, -25, 25, 0, 127);
    //BLEMidiServer.controlChange(0, 21, yDeltaToMIDI);
    yTimeEllapsed = millis();
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



  if (millis() - zTimeEllapsed > 50 && z > 0.1 || z < -0.1)
  { //scale -25 to 25
    int yDeltaToMIDI = map(yDisplacement, -25, 25, 0, 127);
    BLEMidiServer.controlChange(0, 22, yDeltaToMIDI);
    zTimeEllapsed = millis();
  }

}

void updateMovingAverage(MotionParameter* param, float value) {
  // Subtract value previously stored in a given register
  param->total -= param->readings[param->readIndex];

  // Update stored value at register and total
  param->readings[param->readIndex] = value;
  param->total += value;

  param->readIndex++;
  
  if (param->readIndex >= NUM_READINGS) {
    param->readIndex = 0;
  }
}

