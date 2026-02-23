#pragma once

#include <WiFi.h>
#include <ArduinoJson.h>
#include <PicoMQTT.h>
#include "Data.hpp"

extern void ErrorHandler(String s);

/* ---------- WIFI ---------- */
static constexpr char* AP_SSID = "RAFAGUAPO";
static constexpr char* AP_PASS = "12345678";

class Comms
{
  public:

  enum class Robot_id: uint8_t 
  {
    Robot1 = 0,
    Robot2 =1
  };

  enum class Request_type :uint8_t 
  {
    None =0,
    Requested =1,
    Accepted = 2
  };

  private: 
    inline static std::array<Request_type,2> robot_requests{Request_type::None,Request_type::None};
    inline static std::array<uint32_t,2> best_robot_time{0,0};

  inline static PicoMQTT::Server mqtt;
  inline static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
  inline static bool Wifi_error{false};

  inline static String auth_topic_1 = String("gestor/0/autorizacion");
  inline static String auth_topic_2 = String("gestor/1/autorizacion");
  inline static String end_topic_1 = String("gestor/0/finalizado");
  inline static String end_topic_2 = String("gestor/1/finalizado");

  public:
  
  static void init()
  {
    WiFi.mode(WIFI_AP);
    Wifi_error = WiFi.softAP(AP_SSID, AP_PASS);
    
    String topic_solicitud = String("vehiculo/+/solicitud");
    String topic_salida = String("vehiculo/+/salida");

    mqtt.subscribe(topic_solicitud, [](const char *payload) {
      String message = payload;
      static JsonDocument dic;         
      DeserializationError errores = deserializeJson(dic, message);

      if (errores) {
          ErrorHandler("Diccionario JSON no ha podido ser creado");
          return;
      }

      Robot_id robot = static_cast<Robot_id>(dic["id_device"].as<uint8_t>());
      portENTER_CRITICAL(&timerMux);
      set_requested(robot);
      portEXIT_CRITICAL(&timerMux);
    });

    mqtt.subscribe(topic_salida, [](const char *payload) {
      String message = payload;
      static JsonDocument dic;         
      DeserializationError errores = deserializeJson(dic, message);

      if (errores) {
          ErrorHandler("Diccionario JSON no ha podido ser creado");
          return;
      }

      Robot_id robot = static_cast<Robot_id>(dic["id_device"].as<uint8_t>());
      uint32_t time_in_ms = dic["time"];
      portENTER_CRITICAL(&timerMux);
      compare_time(robot,time_in_ms);
      portEXIT_CRITICAL(&timerMux);
    });
    mqtt.begin();
  }

  static bool is_connected(){
    return Wifi_error;
  }
  
  static void update(void* parameters)
  {
    while(1){
    mqtt.loop();
    delay(50);
    }
  }


  static const Request_type& get_state(Robot_id robot)
  {
    return robot_requests[static_cast<size_t>(robot)];
  }

  static void set_done(Robot_id robot)
  {
      robot_requests[static_cast<size_t>(robot)] = Request_type::None;
      send_finish_robot(robot);

  }

  static void set_accepted(Robot_id robot)
  {
      robot_requests[static_cast<size_t>(robot)] = Request_type::Accepted;
      send_auth_robot(robot);

  }

  static auto& get_times()
  {
    return best_robot_time;
  }

private:

  static void set_requested(Robot_id robot)
  {
    robot_requests[static_cast<size_t>(robot)] = Request_type::Requested;
  }

  static void compare_time(Robot_id robot, const uint32_t time)
  {
    if(time < best_robot_time[static_cast<size_t>(robot)])
    {
      best_robot_time[static_cast<size_t>(robot)] = time;
      return;
    }
  }

  static void send_auth_robot(Robot_id robot)
  {
    static JsonDocument jsonBuffer; 
    jsonBuffer["id_device"] = static_cast<uint8_t>(robot); 
    jsonBuffer["auth"] = true;
    
    char payload[128];
    serializeJson(jsonBuffer, payload);
    
    if(robot == Robot_id::Robot1)
    {
    mqtt.publish(auth_topic_1, payload);
    }
    else
    {
      mqtt.publish(auth_topic_2, payload);
    }
  }

  static void send_finish_robot(Robot_id robot) //Hasta que haya sensores con un timeout
  {
    static JsonDocument jsonBuffer; 
    jsonBuffer["id_device"] = static_cast<uint8_t>(robot); 
    jsonBuffer["end"] = true;
    
    char payload[128];
    serializeJson(jsonBuffer, payload);
    
    if(robot == Robot_id::Robot1)
    {
    mqtt.publish(end_topic_1, payload);
    }
    else
    {
      mqtt.publish(end_topic_2, payload);
    }
   }
    


};