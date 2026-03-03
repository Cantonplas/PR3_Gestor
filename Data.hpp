#pragma once


enum class General_states: uint8_t
{
  Connecting =0,
  Operational =1,
  Fault=2
};

enum class Operational_states: uint8_t
{
  Junction_ready=0,
  Junction_busy=1
};

namespace Pinout
{
  static constexpr uint8_t PIN_LED = RGB_BUILTIN;
  //Ultrasonidos:
  static constexpr uint8_t TRIG_PIN1 =17;
  static constexpr uint8_t ECHO_PIN1 =18;
  static constexpr uint8_t TRIG_PIN2 =15;
  static constexpr uint8_t ECHO_PIN2 =16;
};


