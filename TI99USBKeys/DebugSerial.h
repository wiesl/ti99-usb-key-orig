#ifndef _DEBUG_SERIAL_H
#define _DEBUG_SERIAL_H

#ifndef WAIT_FOR_SERIAL
#define WAIT_FOR_SERIAL false
#endif

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c

#if defined(DEBUG_SERIAL)
#define DEBUG_PRINT(...)   do { if (Serial) { Serial.print(__VA_ARGS__); Serial.send_now(); } } while (0)
#define DEBUG_PRINTLN(...) do { if (Serial) { Serial.println(__VA_ARGS__); Serial.send_now(); } } while (0)
#define DEBUG_PRINT_HEX(hex) do { if (Serial) { Serial.print(hex, HEX); Serial.send_now(); } } while (0)
#define DEBUG_PRINT_HEXLN(hex) do { if (Serial) { Serial.println(hex, HEX); Serial.send_now(); } } while (0)
#define DEBUG_DELAYMS(arg) do { if (Serial) delay(arg); } while (0)
#else
#define DEBUG_PRINT(...)   do {} while (0)
#define DEBUG_PRINTLN(...) do {} while (0)
#define DEBUG_PRINT_HEX(hex) do {} while (0)
#define DEBUG_PRINT_HEXLN(hex) do {} while (0)
#define DEBUG_DELAYMS(arg) do {} while (0)
#endif

void setup_debug_serial()
{
#if defined(DEBUG_SERIAL)
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  if (WAIT_FOR_SERIAL)
  {
    while(!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  }
  else
  {
    // try it several times on bootup
    for(int i = 0; i < 1000; ++i)
    {
      if (Serial)
      {
        DEBUG_PRINT("Serial port initialization time (ms): ");
        DEBUG_PRINTLN(i);
        break;
      }
      delay(1);
    }
  }
#endif
#endif
}

#endif
