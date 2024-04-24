# Developer Manual
Welcome to the Developer Manual for the Wave Control!
## Bill of Materials

The following table lists the components you'll need to build the current iteration of the Wave Control:

| Component | Quantity | Cost |
|---|---|---|
| Feather Adafruit ESP32-S3 Reverse TFT Feather | 1 | $24.95  |
| 2.5" Flex Sensor | 1 | $17.95 |
| Adafruit DRV2605L Haptic Motor Driver | 1 | $7.95|
| Adafruit LSM6DSOX1 Sensor | 1 |$11.95 |
| STEMMA I2C cable connectors| 2| $0.95|
| Haptic Motor | 1 | $1.95|
| Jumper wires | 6 | $1.95 (for 20) |
| Breadboard or Proto Board | 1 | $2.00 |
| 45 kOhm Resistor | 1 | N/A|
| USB C Cable | 1 | N/A |
| Total Cost| |$70.60 |



## Implementation Details

Here's some of the implementation details for this product!

### Sensors and Gestures

Overall, my initial conception ideas for this product were something that delivered haptic feedback to improve realism of playing MIDI instruments, as well as convert certain large arm motion gestures to control the sounds being played.

To achieve the haptic feedback, I experimented with both piezos and a haptic motor. While piezos would be simpler to implement, I was unable to get them to get loud enough to vibrate meaningfully. Thus, I decided to go with the above haptic motor and haptic motor driver since they were well documented by Adafruit and STEMMA compatible. 

As for the motion, one key gesture I wanted to capture was the bending of the elbow—this would be useful in cases like getting closer to a keyboard while playing, a common motion done by many players as they get more intense. To capture this, I chose to use a flex sensor for it's accuracy (and because I had one on hand xD).

I also wanted to be able to capture gestures such as raising a hand up and down, as well as rotating your arm, translating position to control values. For this task, I decided to use the X and Y rotation axis of the gyroscope on Adafruit's LSM6DSOX1 Sensor (which I convert to distance values, as I will talk more about later). 

Finally, I wanted this device to be wireless and allow freedom of movement on stage—thus I decided to use an ESP32 and the BLE MIDI library to implement this whole project. 


### Data Processing

The processing for all of these controls was fairly simple!

* For the haptic feedback, I simply set the ESP32 to react to incoming MIDI Note On messages sent to it from the DAW. 

* The flex sensor was also fairly straightforward to incorporate-I simply scaled the incoming signal linearly to MIDI compatible values (0-127) with the map() function in the Arduino IDE. I then sent the output as MIDI CC signals.

* The gyroscope control is what took more processing in the end. Since a gyroscope gives velocity data as radians/sec and I wanted to measure displacement in radians initially, this involved integrating velocity values, calculating the area every 200 ms and adding that amount to global displacement variables. From there, I scaled the radian values to 0-127 via the map function and sent them as CC signals. 

### Future Plans

Overall, I got much of what I intended in this project done. However, I would have loved to make the haptic motor react directly to the audio coming from the DAW, sustaining and following the envelope shape of the sound being played. This would be a much more involved process on both the DAW side and the Wave Control side however, and is an avenue to be explored later if deemed reasonable. 

Aside from this, I simply seek to add an extra few controls, the ability to enable and disable certain controls via buttons, and utilize the screen on the S3 Reverse TFT used! 

(Oh, and of course an end design that involves something that's not a sock).