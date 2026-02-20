#pragma once
#include <Arduino.h>
#include "Data.hpp"

class Actuators
{
  public:

  static void init()
  {
     neopixelWrite(Pinout::PIN_LED, 0, 0, 0);
  }

  static void set_led_red(bool state)
  {
    if(state == false)
    {
      neopixelWrite(Pinout::PIN_LED, 0, 0, 0);
    }
    else
    {
      neopixelWrite(Pinout::PIN_LED, 255, 0, 0);
    }
  }

  static void set_led_green(bool state)
  {
    if(state == false)
    {
      neopixelWrite(Pinout::PIN_LED, 0, 0, 0);
    }
    else
    {
      neopixelWrite(Pinout::PIN_LED, 0, 255, 0);
    }
  }

  static void set_led_blue(bool state)
  {
    if(state == false)
    {
      neopixelWrite(Pinout::PIN_LED, 0, 0, 0);
    }
    else
    {
      neopixelWrite(Pinout::PIN_LED, 0, 0, 255);
    }
  }
};