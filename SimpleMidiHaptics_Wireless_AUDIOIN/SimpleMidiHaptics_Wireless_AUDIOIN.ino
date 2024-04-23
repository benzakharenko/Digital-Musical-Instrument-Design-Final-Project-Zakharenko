#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <Arduino.h>
#include <BLEMidi.h>

//Defining Input and Output Pins:
// Configuration
const int pot_pin = A0;
const uint8_t midi_channel = 0;
const uint8_t controller_number = 0;
const float c = 0.2;  // low pass filter coefficient (0 < c <= 1)

//initializing functions to be defined later
void onNoteOn();
void onNoteOff();
void onControlChange();
void connected() {
  Serial.println("Connected");
}


//calling the DRV265 class
Adafruit_DRV2605 drv;

void setup() {
  Serial.begin(115200);
  pinMode(pot_pin, INPUT);

  // initialize the vibration motor
  drv.begin();
  drv.selectLibrary(1);
  drv.setMode(DRV2605_MODE_AUDIOVIBE);

  // Initialize the BLE MIDI server
  BLEMidiServer.begin("MIDI Glove");
  BLEMidiServer.setOnConnectCallback(connected);
  BLEMidiServer.setOnDisconnectCallback([]() {  // To show how to make a callback with a lambda function
    Serial.println("Disconnected");
  });
}

void loop() {
  // continually check the loop for Note On and Note Off messages
  BLEMidiServer.setNoteOnCallback(onNoteOn);
  BLEMidiServer.setNoteOffCallback(onNoteOff);
  BLEMidiServer.setControlChangeCallback(onControlChange);
  // for flex sensor
  static float y = 0;
  static uint8_t old_y = 0;

  float x = map(analogRead(pot_pin), 850, 520, 0, 127);
  y += c * (x - y);  // simple low pass IIR filter (see tutorial here : https://tomroelandts.com/articles/low-pass-single-pole-iir-filter )
                     // (when we read the potentiometer, the value flickers a little bit all the time, so we need to do filter it to avoid sending midi messages all the time)
                     // another solution may be better than this type of filter, it is left as an exercise to the reader :)
  if (BLEMidiServer.isConnected() && (uint8_t)y != old_y) {
    BLEMidiServer.controlChange(midi_channel, controller_number, y);
    old_y = (uint8_t)y;
  }
}

// defining the reading value functions
void onNoteOn(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  Serial.printf("Received note on : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  tone(9, 100);
}

void onNoteOff(uint8_t channel, uint8_t note, uint8_t velocity, uint16_t timestamp) {
  Serial.printf("Received note off : channel %d, note %d, velocity %d (timestamp %dms)\n", channel, note, velocity, timestamp);
  noTone(9);
}

void onControlChange(uint8_t channel, uint8_t controller, uint8_t value, uint16_t timestamp) {
  Serial.printf("Received control change : channel %d, controller %d, value %d (timestamp %dms)\n", channel, controller, value, timestamp);
}
