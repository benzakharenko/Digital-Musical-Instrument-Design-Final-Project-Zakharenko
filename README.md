# Digital Musical Instrument Design Final Project Zakharenko
Welcome to the repository for Ben Zakharenko's Digital Music Instrument Design final project at Berklee College of Music!

## Concept
The device to be designed for this project, codenamed project "Wave Control" for the moment, is an arm-attached instrument that has two primary purposes—to receive precise haptic feedback remotely from a device, and to transmit arm related gestures (such as rolling the arm or bending the elbow) back to a receiving device. This can be used either for real time instrument alteration or performance, as a way to receive audio-rate physical feedback from the electronic instrument while also allowing one to alter it, or for interactive exhibition/toy purposes (for example, a theoretical Star Wars experience at DisneyWorld where guests with this device could press a button on their arm and wave their hand to open doors with "the Force"). For reference visually, one could imagine something similar to the arm controller Syndrome, the villain in *The Incredibles*, wears.

## Delivery
There are three theoretical stages of this instrument. 


Good outcome:
* successful haptics
* hardwired demo with the teensy
* a few gestures, such as elbow bends, captured to transmit back via MIDI
* Corresponding Max patch to demonstrate instrument performance capabilities

Better outcome:
* wireless BLE MIDI implementation
* more controls and gestures added
* powered via portable battery

Best outcome:
* Added BLE receiver to make any device with USB capabilities and MIDI processing ability compatible
* add proper rechargeable battery and related circuit
* add gyroscopic and accelerometer support to capture arm movements
* proper casing and enclosure
* . . . and more if time allows!

## Implementation

The main challenge with this project, if it reaches full fruition, would be in the following three areas: getting responsive and useful haptics, implementing wireless technology, and learning a new wireless compatible board.

For the wireless compatible board, the three options I'm considering are an ESP32, a Raspberry Pi Pico W, or an Adafruit Flora board with a paired Bluefruit LE module. All three have wireless capabilities and a substantial community, so finding libraries to transmit wireless MIDI signals should be possible. My choice of board will depend upon the libraries I find that exist for this, on top of the I/O choice on each board.

For the haptics, I am currently leaning towards the idea of using piezos and simply converting MIDI input directly to frequency pitch and amplitude of vibrations, but may also consider using simple haptic motors or solenoids. 

Most of the theoretical inputs that would be put in otherwise are fairly self-explanatory as we had gone over them in class—elbow bends could be captured with a flex sensor, presses could be read by buttons, force sensitive resistors or capacitive sensors, the sensor module in our package could deliver accelerometer and gyroscopic motions, and a soft pot could be used as a potentiometer to capture sliding motions.