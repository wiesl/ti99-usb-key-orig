#define DEBUG_USB_PRINTING
#define DEBUG_PRINTING

#include <hidboot.h>
#include <usbhub.h>
#include <usbhid.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

#ifdef DEBUG_PRINTING
#define DEBUG_PRINT(s)    do { Serial.print(s); Serial.send_now(); } while( false )
#define DEBUG_PRINTLN(s)    do { Serial.println(s); Serial.send_now(); } while( false )
#define DEBUG_PRINT_HEX(hex)    do { Serial.print(hex, HEX); Serial.send_now(); } while( false )
#else
#define DEBUG_PRINT(s)        /* Don't do anything in release builds */
#define DEBUG_PRINTLN(s)        /* Don't do anything in release builds */
#define DEBUG_PRINT_HEX(hex)  /* Don't do anything in release builds */
#endif

// Reboot support
#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

#define NUMLOCK_STARTUP true
#define CAPSLOCK_STARTUP true
#define SCROLLLOCK_STARTUP false

#include "TiPins.h"

#include "TiVirtualState.h"

#include "TiScan.h"

#include "TiKbdRptParser.h"

USB     Usb;
USBHub     Hub(&Usb);
HIDBoot<USB_HID_PROTOCOL_KEYBOARD>    HidKeyboard(&Usb);

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
  delay(10000);
#endif
  
  DEBUG_PRINTLN("setup() begin");
  //initialLEDblink();

  lastGoodState = millis();
  firstBoot = 1;
  
  initData();

  DEBUG_PRINTLN("Usb.Init()");
  // Wait for keyboard to be up
//  while (Usb.Init() == -1)
//    delay( 5000 );
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

void loop()
{
  if (firstBoot) DEBUG_PRINTLN("loop() begin");
  // Read USB input which updates the state of the in-memory keyboard matrix.
  Usb.Task();

  long loopMillis = millis();
  uint8_t state = Usb.getUsbTaskState();
  
  if (state != USB_STATE_RUNNING) {
    DEBUG_PRINTLN("NOT OK: USB_STATE_RUNNING");
    if ((loopMillis - lastGoodState) > 5000) {
      DEBUG_PRINTLN("CPU_RESTART");
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
  if (firstBoot) DEBUG_PRINTLN("loop() end");
}


