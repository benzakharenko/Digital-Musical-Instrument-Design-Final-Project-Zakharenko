# User Manual

This documentation will cover some more specifics about the current things the Wave Control is capable of. For setup information and usage, please refer to the [Getting Started Guide](./GettingStarted.md).

## Wireless Communication
The Wave Control operates via MIDI BLE ("_Bluetooth Low Energy_"). This means that the device can operate freely, without wires plugged into it (other than the power cable!)

## Haptics

The Wave Control is designed to turn any incoming MIDI Note signals into haptic vibrations. Follow along with the [Getting Started Guide](./GettingStarted.md) to learn more about how to make this work!

## Gyroscopic Control

The Wave Control also has two axis of angle displacement measurement that can be converted to mappable CC messages. rotating your arm increases or decreases values of MIDI CC #20, whereas moving your arm up and down increases or decreases values of MIDI CC #21!

## Elbow Bend Control

The Wave Control is capable of converting your elbow messages into MIDI CC messages (specifically, MIDI CC #17). The more you bend your elbow, the higher the message delivered will be. 