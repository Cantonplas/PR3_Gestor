#include "MQTT-WIFI.hpp"
#include "Board.hpp"

void Comms::set_done(Robot_id robot)
  {
      portENTER_CRITICAL(&timerMux);
      robot_requests[static_cast<size_t>(robot)] = Request_type::None;
      portEXIT_CRITICAL(&timerMux);
      Board::set_junction_busy(false);
      send_finish_robot(robot);
  }