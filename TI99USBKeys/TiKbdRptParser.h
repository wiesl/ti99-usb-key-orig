#ifndef _TI_KBD_RPT_PARSER_H
#define _TI_KBD_RPT_PARSER_H

#ifndef USB_USE_OLD_API
// New API (V1.3.0 or newer)
#define USB_HID_CLASS USBHID
#else
// Old API (V1.2.1)
#define USB_HID_CLASS HID
#endif

#include <hidboot.h>

#include "USBCodes.h"

//-----------------------------------------------
// USB Keyboard input handling - mostly taken
//   from the USB Host Boot Keyboard example.

class TiKbdRptParser : public KeyboardReportParser
{
  protected:
    virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
    virtual void OnKeyDown	(uint8_t mod, uint8_t key);
    virtual void OnKeyUp	(uint8_t mod, uint8_t key);

  private:
    void updateModifier(uint8_t mask, uint8_t before, uint8_t after, int* tk);
    boolean handleSimple(uint8_t key, int state);
    boolean handleFunction(uint8_t key, int state);
    boolean handleArrows(uint8_t key, int state);
    boolean handleNumpad(uint8_t key, int state);
    boolean backquote = false;
    boolean backslash = false;
    boolean doublequote = false;
    boolean hyphen = false;
    boolean pipe = false;
    boolean question = false;
    boolean quote = false;
    boolean slash = false;
    boolean tilde = false;
    boolean underscore = false;
    boolean opensquare = false;
    boolean closesquare = false;
    boolean opencurly = false;
    boolean closecurly = false;
    int magic = 0; // If this is set, a magic combo is active. Such as Shift tranformed to Fctn, or magically pressing fctn for a [. 
    // we'll increment magic on keypresses during magic, and then only release finally once magic is unwound.

  public:
    TiKbdRptParser();
    void setKeyLocks(USB_HID_CLASS* hid, boolean numLock, boolean capsLock, boolean scrollLock);
};

TiKbdRptParser::TiKbdRptParser()
{
}

void TiKbdRptParser::setKeyLocks(USB_HID_CLASS* hid, boolean numLock, boolean capsLock, boolean scrollLock)
{ 
  if (numLock) {
    kbdLockingKeys.kbdLeds.bmNumLock = 1;
  }
  if (capsLock) {
    kbdLockingKeys.kbdLeds.bmCapsLock = 1;
    *tk_Alpha = 1;
  }
  if (scrollLock) {
    kbdLockingKeys.kbdLeds.bmScrollLock = 1;
  }
  hid->SetReport(0, 0, 2, 0, 1, &(kbdLockingKeys.bLeds));
}

void TiKbdRptParser::updateModifier(uint8_t mask, uint8_t before, uint8_t after, int* tk)
{
  boolean wasMod = before & mask;
  boolean isMod = after & mask;
  if (wasMod && (!isMod)) {
    tk_release(tk);
  } else if (isMod && (!wasMod)) {
    tk_press(tk);
  }
}

void TiKbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after)
{
  updateModifier(U_LEFTSHIFT | U_RIGHTSHIFT, before, after, tk_Shift);
  updateModifier(U_LEFTALT | U_RIGHTALT, before, after, tk_Fctn);
  updateModifier(U_LEFTCTRL | U_RIGHTCTRL, before, after, tk_Ctrl);
}

#define ISSHIFT(X) ((U_LEFTSHIFT | U_RIGHTSHIFT) & X)
#define ISALT(X) ((U_LEFTALT | U_RIGHTALT) & X)
#define ISCTRL(X) ((U_LEFTCTRL | U_RIGHTCTRL) & X)

void TiKbdRptParser::OnKeyDown(uint8_t mod, uint8_t key)
{
  if (handleSimple(key, 1)) {
    return;
  }
  if (mod == 0 && handleFunction(key, 1)) {
    return;
  }
  if (mod == 0 && handleArrows(key, 1)) {
    return;
  }
  if (mod == 0 && handleNumpad(key, 1)) {
    return;
  }

  if (key == U_HYPHEN && mod == 0) {
    tk_press(tk_Shift);
    tk_press(tk_Slash);
    hyphen = true; magic++;
  } else if (key == U_HYPHEN && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_U);
    underscore = true; magic++;
  } else if (key == U_SLASH && mod == 0) {
    tk_press(tk_Slash);
    slash = true; magic++;
  } else if (key == U_SLASH && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_I);
    question = true; magic++;
  } else if (key == U_BACKSLASH && mod == 0) {
    tk_press(tk_Fctn);
    tk_press(tk_Z);
    backslash = true; magic++;
  } else if (key == U_BACKSLASH && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_A);
    pipe = true; magic++;
  } else if (key == U_BACKQUOTE && mod == 0) {
    tk_press(tk_Fctn);
    tk_press(tk_C);
    backquote = true; magic++;
  } else if (key == U_BACKQUOTE && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_W);
    tilde = true; magic++;
  } else if (key == U_QUOTE && mod == 0) {
    tk_press(tk_Fctn);
    tk_press(tk_O);
    quote = true; magic++;
  } else if (key == U_QUOTE && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_P);
    doublequote = true; magic++;
  } else if (key == U_OPENSQUARE && mod == 0) {
    tk_press(tk_Fctn);
    tk_press(tk_R);
    opensquare = true; magic++;
  } else if (key == U_OPENSQUARE && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_F);
    opencurly = true; magic++;
  } else if (key == U_CLOSESQUARE && mod == 0) {
    tk_press(tk_Fctn);
    tk_press(tk_T);
    closesquare = true; magic++;
  } else if (key == U_CLOSESQUARE && ISSHIFT(mod)) {
    tk_release(tk_Shift);
    tk_press(tk_Fctn);
    tk_press(tk_G);
    closecurly = true; magic++;
  }
}

void TiKbdRptParser::OnKeyUp(uint8_t mod, uint8_t key)
{  
  if (handleSimple(key, 0)) {
    return;
  }
  if (mod == 0 && handleFunction(key, 0)) {
    return;
  }
  if (mod == 0 && handleArrows(key, 0)) {
    return;
  }
  if (mod == 0 && handleNumpad(key, 0)) {
    return;
  }

  if (key == U_HYPHEN) {
    if (underscore) {
      tk_release(tk_U);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      underscore = false; magic--;
    } else {
      tk_release(tk_Slash);
      tk_release(tk_Shift);
      hyphen = false; magic--;
    }
  } else if (key == U_SLASH) {
    if (question) {
      tk_release(tk_I);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      question = false; magic--;
    } else {
      tk_release(tk_Slash);
      slash = false; magic--;
    }
  } else if (key == U_BACKSLASH) {
    if (pipe) {
      tk_release(tk_A);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      pipe = false; magic--;
    } else {
      tk_release(tk_Z);
      tk_release(tk_Fctn);
      backslash = false; magic--;
    }
  } else if (key == U_BACKQUOTE) {
    if (tilde) {
      tk_release(tk_W);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      tilde = false; magic--;
    } else {
      tk_release(tk_C);
      tk_release(tk_Fctn);
      backquote = false; magic--;
    }
  } else if (key == U_QUOTE) {
    if (doublequote) {
      tk_release(tk_P);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      doublequote = false; magic--;
    } else {
      tk_release(tk_O);
      tk_release(tk_Fctn);
      quote = false; magic--;
    }
  } else if (key == U_OPENSQUARE) {
    if (opensquare) {
      tk_release(tk_R);
      tk_release(tk_Fctn);
      opensquare = false; magic--;
    } else {
      tk_release(tk_F);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      opencurly = false; magic--;
    }
  } else if (key == U_CLOSESQUARE) {
    if (closesquare) {
      tk_release(tk_T);
      tk_release(tk_Fctn);
      closesquare = false; magic--;
    } else {
      tk_release(tk_G);
      tk_release(tk_Fctn);
      tk_press(tk_Shift);
      closecurly = false; magic--;
    }
  }

  else if (key == U_CAPSLOCK) {
      *tk_Alpha = kbdLockingKeys.kbdLeds.bmCapsLock;
  }

  else if (key == U_DELETE && ISCTRL(mod) && ISALT(mod)) {
    CPU_RESTART;
  }

  else if (key == U_F10 && ISCTRL(mod) && ISALT(mod)) {
    pinMode(ti_g1,OUTPUT_OPENDRAIN);
    digitalWrite(ti_g1,LOW);
    delay(25);
    pinMode(ti_g1,INPUT_PULLUP);
    digitalWrite(ti_g1,HIGH);
  }
  else if (key == U_F11 && ISCTRL(mod) && ISALT(mod)) {
    pinMode(ti_g1,OUTPUT_OPENDRAIN);
    digitalWrite(ti_g2,LOW);
    delay(25);
    pinMode(ti_g2,INPUT_PULLUP);
    digitalWrite(ti_g2,HIGH);
  }
  else if (key == U_F12 && ISCTRL(mod) && ISALT(mod)) {
    pinMode(ti_g1,OUTPUT_OPENDRAIN);
    digitalWrite(ti_g3,LOW);
    delay(25);
    pinMode(ti_g3,INPUT_PULLUP);
    digitalWrite(ti_g3,HIGH);
  } 
}

#define BCASE(X, Y) case X: if(state) { tk_press(Y); } else { tk_release(Y); } return true
#define FCASE(X, Y) case X: if(state) { tk_press(tk_Fctn); tk_press(Y); magic++; } else { tk_release(Y); tk_release(tk_Fctn); magic--; } return true
#define CCASE(X, Y) case X: if(state) { tk_press(tk_Ctrl); tk_press(Y); magic++; } else { tk_release(Y); tk_release(tk_Ctrl); magic--; } return true
#define SCASE(X, Y) case X: if(state) { tk_press(tk_Shift); tk_press(Y); magic++; } else { tk_release(Y); tk_release(tk_Shift); magic--; } return true

// Handle the keys that are a one to one mapping, with no modifier issues.
boolean TiKbdRptParser::handleSimple(uint8_t key, int state)
{    
  if (state && magic) {
    return false;
  }
  
  switch(key) {
    BCASE(U_NUM1,tk_1);
    BCASE(U_NUM2,tk_2);
    BCASE(U_NUM3,tk_3);
    BCASE(U_NUM4,tk_4);
    BCASE(U_NUM5,tk_5);
    BCASE(U_NUM6,tk_6);
    BCASE(U_NUM7,tk_7);
    BCASE(U_NUM8,tk_8);
    BCASE(U_NUM9,tk_9);
    BCASE(U_NUM0,tk_0);
    BCASE(U_A,tk_A);
    BCASE(U_B,tk_B);
    BCASE(U_C,tk_C);
    BCASE(U_D,tk_D);
    BCASE(U_E,tk_E);
    BCASE(U_F,tk_F);
    BCASE(U_G,tk_G);
    BCASE(U_H,tk_H);
    BCASE(U_I,tk_I);
    BCASE(U_J,tk_J);
    BCASE(U_K,tk_K);
    BCASE(U_L,tk_L);
    BCASE(U_M,tk_M);
    BCASE(U_N,tk_N);
    BCASE(U_O,tk_O);
    BCASE(U_P,tk_P);
    BCASE(U_Q,tk_Q);
    BCASE(U_R,tk_R);
    BCASE(U_S,tk_S);
    BCASE(U_T,tk_T);
    BCASE(U_U,tk_U);
    BCASE(U_V,tk_V);
    BCASE(U_W,tk_W);
    BCASE(U_X,tk_X);
    BCASE(U_Y,tk_Y);
    BCASE(U_Z,tk_Z);
    BCASE(U_COMMA,tk_Comma);
    BCASE(U_PERIOD,tk_Period);
    BCASE(U_EQUAL,tk_Equal);
    BCASE(U_SEMICOLON,tk_Semicolon);
    BCASE(U_SPACE,tk_Space);
    BCASE(U_ENTER,tk_Enter);
  }
  return false;
}

boolean TiKbdRptParser::handleFunction(uint8_t key, int state)
{  
  switch(key) {
    FCASE(U_BACKSPACE,tk_S);
    FCASE(U_F1,tk_1);
    FCASE(U_F2,tk_2);
    FCASE(U_F3,tk_3);
    FCASE(U_F4,tk_4);
    FCASE(U_F5,tk_5);
    FCASE(U_F6,tk_6);
    FCASE(U_F7,tk_7);
    FCASE(U_F8,tk_8);
    FCASE(U_F9,tk_9);
    FCASE(U_F10,tk_0);
    CCASE(U_F11,tk_1);
    CCASE(U_F12,tk_2);
    SCASE(U_NUMPAD_HYPHEN,tk_Slash);
    SCASE(U_NUMPAD_STAR,tk_8);
    BCASE(U_NUMSLASH,tk_Slash);
    SCASE(U_NUMPAD_PLUS,tk_Equal);
    BCASE(U_NUMPAD_ENTER,tk_Enter);
    FCASE(U_BREAK,tk_4);
    CCASE(U_HOME,tk_U);
    CCASE(U_END,tk_V);
    FCASE(U_TAB,tk_7);
    FCASE(U_PGUP,tk_6);
    FCASE(U_PGDN,tk_4);
    FCASE(U_ESC,tk_9);
    FCASE(U_DELETE,tk_1);
    FCASE(U_INSERT,tk_2);
  }
  return false;
}

boolean TiKbdRptParser::handleArrows(uint8_t key, int state)
{
  if (!kbdLockingKeys.kbdLeds.bmScrollLock) {
    switch(key) {
      FCASE(U_LEFTARROW,tk_S);
      FCASE(U_RIGHTARROW,tk_D);
      FCASE(U_UPARROW,tk_E);
      FCASE(U_DOWNARROW,tk_X);     
    }
  } else {
    switch(key) {
      BCASE(U_LEFTARROW,tk_S);
      BCASE(U_RIGHTARROW,tk_D);
      BCASE(U_UPARROW,tk_E);
      BCASE(U_DOWNARROW,tk_X);
    }
  }
  return false;
}

boolean TiKbdRptParser::handleNumpad(uint8_t key, int state)
{
  if (!kbdLockingKeys.kbdLeds.bmNumLock) {
    switch(key) {
      FCASE(U_NUMPAD_PERIOD,tk_1);
      CCASE(U_NUMPAD_1,tk_V);
      FCASE(U_NUMPAD_2,tk_X);
      FCASE(U_NUMPAD_3,tk_4);
      FCASE(U_NUMPAD_4,tk_S);
      BCASE(U_NUMPAD_5,tk_5);
      FCASE(U_NUMPAD_6,tk_D);
      CCASE(U_NUMPAD_7,tk_U);
      FCASE(U_NUMPAD_8,tk_E);
      FCASE(U_NUMPAD_9,tk_6);
      FCASE(U_NUMPAD_0,tk_2);
    }
  } else {
    switch(key) {
      BCASE(U_NUMPAD_PERIOD,tk_Period);
      BCASE(U_NUMPAD_1,tk_1);
      BCASE(U_NUMPAD_2,tk_2);
      BCASE(U_NUMPAD_3,tk_3);
      BCASE(U_NUMPAD_4,tk_4);
      BCASE(U_NUMPAD_5,tk_5);
      BCASE(U_NUMPAD_6,tk_6);
      BCASE(U_NUMPAD_7,tk_7);
      BCASE(U_NUMPAD_8,tk_8);
      BCASE(U_NUMPAD_9,tk_9);
      BCASE(U_NUMPAD_0,tk_0);
    }
  }
  return false;
}

#endif


