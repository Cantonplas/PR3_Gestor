#pragma once
#include "Board.hpp"

void setup() {
  Serial.begin(115200);
  Board::start();
  Comms::init();
  Scheduler::start();
  xTaskCreate(
    Comms::update,         
   "mqtt_update",       
    2048,             
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
  Serial.print(s);
  while(1){
      Serial.print("Error handler called...");
      delay(500);
  };
}