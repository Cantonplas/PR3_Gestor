#pragma once
#include "Board.hpp"

void setup() {
  Serial.begin(115200);
  Board::start();
  Comms::init();
  Sensors::init();
  Scheduler::start();
  xTaskCreate(
    Comms::update,         
   "mqtt_update",       
    8192,             
    NULL,           
    1,                
    NULL              
  );
}

void loop() {
  Scheduler::update();
}


void ErrorHandler(String s)
{
  while(1){
      Serial.println("Error handler called...");
      Serial.print("Error is:");
      Serial.println(s);
      delay(500);
  };
}