#define DEBUG_USB_PRINTING
#define DEBUG_SERIAL
//#define USB_USE_OLD_API

#ifndef USB_USE_OLD_API
// New API (V1.3.0 or newer)
#define USB_HID_PROTCOL_KEYBOARD_CLASS USB_HID_PROTOCOL_KEYBOARD
#else
// Old API (V1.2.1)
#define USB_HID_PROTCOL_KEYBOARD_CLASS HID_PROTOCOL_KEYBOARD
#endif

#include <spi4teensy3.h>
#include <SPI.h>
#ifndef USB_USE_OLD_API
#include <usbhid.h>
#endif
#include <hidboot.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#define NUMLOCK_STARTUP true
#define CAPSLOCK_STARTUP true
#define SCROLLLOCK_STARTUP false

#include "DebugSerial.h"

#include "teensy_reboot_support.h"

#include "TiPins.h"

#include "TiVirtualState.h"

#include "TiScan.h"

#include "TiKbdRptParser.h"

USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTCOL_KEYBOARD_CLASS>    HidKeyboard(&Usb);

TiKbdRptParser Prs;

long lastGoodState;
long firstBoot;

// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 13;

void initialLEDblink()
{
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(200);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW 
}

void setup_serial_debug()
{
    Serial.begin(9600);
}

void setup()
{
#ifdef DEBUG_PRINTING
  setup_serial_debug();
  // Wait to get serial monitor up & running
  delay(10000);
#endif
  
  DEBUG_PRINTLN("setup() begin");
  //initialLEDblink();

  lastGoodState = millis();
  firstBoot = 1;
  
  initData();

  DEBUG_PRINTLN("Usb.Init()");
  // Wait for keyboard to be up
  while (Usb.Init() == -1)
    delay( 20 );

  DEBUG_PRINTLN("Usb.Init() complete");
  HidKeyboard.SetReportParser(0, (HIDReportParser*)&Prs);
  DEBUG_PRINTLN("setup() end");
}

void gpioSetup()
{
  DEBUG_PRINTLN("gpioSetup() begin");
  initPinModes();
  setColumnInterrupts();
  DEBUG_PRINTLN("gpioSetup() end");
}

const char* get_usb_state(uint8_t state)
{
  switch(state)
  {
    case 0x10: return "USB_STATE_DETACHED";
    case 0x11: return "USB_DETACHED_SUBSTATE_INITIALIZE";
    case 0x12: return "USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE";
    case 0x13: return "USB_DETACHED_SUBSTATE_ILLEGAL";
    case 0x20: return "USB_ATTACHED_SUBSTATE_SETTLE";
    case 0x30: return "USB_ATTACHED_SUBSTATE_RESET_DEVICE";
    case 0x40: return "USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE";
    case 0x50: return "USB_ATTACHED_SUBSTATE_WAIT_SOF";
    case 0x51: return "USB_ATTACHED_SUBSTATE_WAIT_RESET";
    case 0x60: return "USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE";
    case 0x70: return "USB_STATE_ADDRESSING";
    case 0x80: return "USB_STATE_CONFIGURING";
    case 0x90: return "USB_STATE_RUNNING";
    case 0xa0: return "USB_STATE_ERROR";
    default: return "UNKNOWN STATE!";
  }
}

inline void print_usb_state_change(uint8_t state)
{
  static uint8_t last_state = 0;
  if (state != last_state)
  {
    DEBUG_PRINT("NOT OK: USB_STATE_RUNNING, state=0x");
    DEBUG_PRINT_HEX(state);
    DEBUG_PRINT(", ");
    DEBUG_PRINTLN(get_usb_state(state));
  }
  last_state = state;
}

void loop()
{
  static bool firstLoop = true;
  if (firstLoop) DEBUG_PRINTLN("loop() begin");
  // Read USB input which updates the state of the in-memory keyboard matrix.
  Usb.Task();

  long loopMillis = millis();
  uint8_t state = Usb.getUsbTaskState();
  print_usb_state_change(state);
  
  if (state != USB_STATE_RUNNING) {
    //if ((loopMillis - lastGoodState) > 5000) {
    if ((loopMillis - lastGoodState) > 10000) { 
      DEBUG_PRINTLN("CPU_RESTART");
      DEBUG_DELAYMS(1000);
      CPU_RESTART;
    }
  } else {
    lastGoodState = loopMillis;
    if (firstBoot != 0) {
      DEBUG_PRINTLN("Prs.setKeyLocks()");
      // Set numlock and capslock on, leave scroll lock off.
      Prs.setKeyLocks(&HidKeyboard, NUMLOCK_STARTUP, CAPSLOCK_STARTUP, SCROLLLOCK_STARTUP);
      firstBoot = 0; 
      gpioSetup();
    }
  }
  if ((loopMillis - lastInterrupted) > 200) {
    onTiColumnChange();
  }
  if (firstLoop) DEBUG_PRINTLN("loop() end");
  firstLoop = false;
}


