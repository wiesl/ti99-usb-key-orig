# TI-99-usb-keys
Teensy 3.1 sketch to translate usb keyboard to TI-99/4a 15 pin interface.

This project interfaces a Teensy 3.1 and a Circuits@Home USB Host shield, to take input from a USB keyboard, and simulate the
matrix keyboard built into the TI. 

The USB keyboard can be used with the TI keyboard still in place. The user can switch between them at will.

At this time, it is on a breadboard. 
All of the PC Keys have been mapped, except caps-lock is never on, and num-lock always is.

I have no mapping for HOME, END, PGDN, PGUP, and gui keys (windows keys)

[Pictures](https://goo.gl/photos/xhLKCTwCPuAeUbP86)

## Resources

[Teensy 3.1](https://www.pjrc.com/teensy/teensy31.html)

[USB Host Shield](https://www.circuitsathome.com/products-page/arduino-shields/usb-host-shield-for-arduino-pro-mini) 

[Mod to teensy arduino libraries for OUTPUT_OPENDRAIN](https://forum.pjrc.com/threads/7531-Internal-pull-up-and-pull-down-resistors-on-teensy-3?p=63944&viewfull=1#post63944)

[USB Host shield libary 2.0](https://github.com/felis/USB_Host_Shield_2.0)


