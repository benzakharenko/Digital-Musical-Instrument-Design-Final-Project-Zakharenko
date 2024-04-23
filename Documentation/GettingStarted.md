# Getting Started

At them moment, using the Wave Control is incredibly easy! To get started, all you need is the following:

* a Mac computer
* Wave Control Device
* a USB type C cable
* A portable power bank
* any Digital Audio Workstation (a.k.a. DAW) that can send and deliver MIDI messages!

## Turning on the Wave Control

To turn on the Wave Control, simply plug it in via your USB C cable. That's it, it's now on!

## Connecting Wave Control to Your Computer

To pair the Wave Control with your computer, open the AUDIO MIDI SETUP application built into your Mac. Go to "Window->Show MIDI Studio" or Press _CMD+2_. 

This should open up the MIDI Studio window. In the top right corner of this window, there should be a Bluetooth icon. Click this icon, and a window will appear that allows you to connect Bluetooth MIDI devices!

If your Wave Control device is on, it should appear as "MIDI Glove" in this window. Click connect—after a few seconds, the button should change to say "_connected_" and you're ready to get to work!

## Configuring the Wave Control to Work with Your DAW

The Wave Control works by converting motion data and elbow bends to CC data, and it receives MIDI note signals from your DAW to convert them to haptic feedback. 
### Ableton Live Configuration with the Wave Control

Here are the steps to configure Ableton Live to work with your Wave Control device:

1. Open Ableton Live.
2. Open the "Preferences" Panel and Click the Tab that says "Link/Tempo/MIDI."
3. In the MIDI Ports section at the bottom, you should see "MIDI Glove In" and "MIDI Glove Out." Make sure the checkboxes for "Track" and "Remote" **checked** for both. 
4. Close the preferences window. Your controller is now paired!

#### Mapping CC Messages

To map CC messages in Ableton, simply use the key command _CMD+M_ to enter MIDI mapping mode. Click the parameter you would like to map, and make the gesture you would like to control that! 

You can also change the mapping range in the MIDI Mapping window that opens.

Do _CMD+M_ again once you are done to leave MIDI Mapping mode.

#### Getting Haptics

To receive haptics, first identify the MIDI instrument track you would like to have haptic feedback. 

Next, create a new MIDI track. Set the input of this MIDI track under "MIDI From" to be your instruments' name.

Then, in the "MIDI To" sub-menu, select "MIDI Glove." 

Finally, set the monitoring mode to "In."

you should now be receiving haptics from your instruments notes!
