#pragma once
#include <Arduino.h>
#include "Data.hpp"

class Sensors
{   
    public:
    inline static  float distancia_ultra1{999.0f};
    inline static  float distancia_ultra2{999.0f};


    private:
    inline static volatile unsigned long inicioPulso1 = 0;
    inline static volatile unsigned long duracionPulso1 = 0;
    inline static volatile unsigned long inicioPulso2 = 0;
    inline static volatile unsigned long duracionPulso2 = 0;

    static void ARDUINO_ISR_ATTR ecoInterrupcion1() {
    if (digitalRead(Pinout::ECHO_PIN1) == HIGH) {
        inicioPulso1 = micros();
    } 
    else {
        duracionPulso1 = micros() - inicioPulso1;
        distancia_ultra1 = duracionPulso1 * 0.034 / 2.0;
    }
    }

    static void ARDUINO_ISR_ATTR ecoInterrupcion2() {
    if (digitalRead(Pinout::ECHO_PIN2) == HIGH) {
        inicioPulso2 = micros();
    } 
    else {
        duracionPulso2 = micros() - inicioPulso2;
        distancia_ultra2 = duracionPulso2 * 0.034 / 2.0;
    }
    }
    public:
    static void init()
    {
        pinMode(Pinout::TRIG_PIN1,OUTPUT);
        pinMode(Pinout::ECHO_PIN1,INPUT);
        pinMode(Pinout::TRIG_PIN2,OUTPUT);
        pinMode(Pinout::ECHO_PIN2,INPUT);
        attachInterrupt(digitalPinToInterrupt(Pinout::ECHO_PIN1), ecoInterrupcion1, CHANGE);
        attachInterrupt(digitalPinToInterrupt(Pinout::ECHO_PIN2), ecoInterrupcion2, CHANGE);
    }


    static void read_ultrasonido()
    {
        digitalWrite(Pinout::TRIG_PIN1, LOW);
        delayMicroseconds(2);
        digitalWrite(Pinout::TRIG_PIN1, HIGH);
        delayMicroseconds(10);
        digitalWrite(Pinout::TRIG_PIN1, LOW);

        digitalWrite(Pinout::TRIG_PIN2, LOW);
        delayMicroseconds(2);
        digitalWrite(Pinout::TRIG_PIN2, HIGH);
        delayMicroseconds(10);
        digitalWrite(Pinout::TRIG_PIN2, LOW);
        
    }

};