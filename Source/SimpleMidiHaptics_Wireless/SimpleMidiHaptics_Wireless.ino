#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <Adafruit_LSM6DSOX.h>
#include <Arduino.h>
#include <BLEMidi.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// DISPLAY: Use dedicated hardware SPI pins
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
float p = 3.1415926;

//DISPLAY: refresh rate timer
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 200;  //the value is a number of milliseconds

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

//custom map function



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
float loopRateInMicroseconds = 50000;   // 10000us = 10ms



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

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

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

/* optional Z axis to add

  if (mapping_mode == 0 || mapping_mode == 4)
  {
    printAngularVelocityZ(gyro);
  }

  //delayMicroseconds(loopRateInMicroseconds);


*/  
delay(30);
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Display------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - startMillis >= period)  //test whether the period has elapsed
  {
    tft.setTextWrap(true);
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(5, 50);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.println("Mapping Mode: ");
    switch (mapping_mode)
    {
      case 0:
      tft.print("NONE");
      break;

      case 1:
      tft.print("Elbow Bend");
      break;

      case 2:
      tft.print("Wrist Turn");
      break;

      case 3:
      tft.print("Arm Raise/Fall");
      break;

      case 4:
      tft.print("Arm Along Radius");
    }
    /*
    if (mapping_mode == 0)
    {
      tft.print("NO MAPPING MODE ENABLED");
    }
    else if (mapping)
    {
      tft.print(mapping_mode);
    }
    */
    tft.setCursor(5, 110);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2); 
    tft.print("Reset");
    startMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
  }
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

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
    //Serial.print("x Displacement/ ");
    //Serial.println(xDisplacement);

  //MIDI DELIVERY
  if ((x > 0.05 || x < -0.05))
  { // -30 to 30
    float xDeltaToMIDI = int((xDisplacement * 63) + 63);
    if (xDeltaToMIDI > 127)
    {
      xDeltaToMIDI = 127;
    }
    if (xDeltaToMIDI < 0)
    {
      xDeltaToMIDI = 0;
    }
    //Serial.println(xDeltaToMIDI);
    BLEMidiServer.controlChange(0, 20, xDeltaToMIDI);

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

void printAngularVelocityY(sensors_event_t gyro) 
{
  // Angular velocity is measured in radians/s
  float y = gyro.gyro.y;

  //integration calculation for y
  updateMovingAverage(&gyroY, y);


  float yInMs = y * 0.001; //convert to rad/ms
  float yArea = yInMs * 10; //integration calculation as rectangle estimation

  yDisplacement = yDisplacement + yArea;


  //MIDI DELIVERY
  if ((y > 0.05 || y < -0.05))
  { // -30 to 30

    float yDeltaToMIDI = int(mapCustom(yDisplacement, -0.5, 0.5, 0, 127));
    Serial.println(yDeltaToMIDI);

    BLEMidiServer.controlChange(0, 21, yDeltaToMIDI);

  }


  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    for (int i = 0; i < NUM_READINGS; i++) 
    {
      gyroY.readings[i] = 0;
    }
    yDisplacement = 0;
  }
}

/*
void printAngularVelocityZ(sensors_event_t gyro) 
{
  // Angular velocity is measured in radians/s
  float z = gyro.gyro.z;

  //integration calculation for y
  //updateMovingAverage(&gyroZ, z);

  float zInMs = z * 0.001; //convert to rad/ms
  float zArea = zInMs * 10; //integration calculation as rectangle estimation

  zDisplacement = zDisplacement + zArea;
    //Serial.print("Z Displacement/ ");
    //Serial.println(zDisplacement);

  //MIDI DELIVERY
  if ((z > 0.05 || z < -0.05))
  { // -30 to 30
    float zDeltaToMIDI = map(zDisplacement, 0, 11, 0, 127);
    BLEMidiServer.controlChange(0, 22, zDeltaToMIDI);

  }


  // calling reset pin
  if (digitalRead(reset_pin)==HIGH)
  {
    for (int i = 0; i < NUM_READINGS; i++) 
    {
      gyroZ.readings[i] = 0;
    }
    zDisplacement = 0;
  }

}
*/

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
//custom map hat takes floats and clips at 0 and 127
float mapCustom(float x, float in_min, float in_max, float out_min, float out_max) 
{
  float Mapped = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (Mapped > 127)
  {
    Mapped = 127;
  }
  if (Mapped < 0)
  {
    Mapped = 0;
  }
  return Mapped;
}

