#pragma once
#include <Arduino.h>
#include "StateMachine.hpp"
#include "Data.hpp"
#include "Actuators.hpp"
#include "MQTT-WIFI.hpp"

class Board
{
  inline static bool busy_junction_flag{false};

  static void control_loop()
  {
    if(busy_junction_flag == true)
    {
      return;
    }

    if(Comms::get_state(Comms::Robot_id::Robot1) == Comms::Request_type ::Requested)
      {
        Comms::set_accepted(Comms::Robot_id::Robot1);
        busy_junction_flag = true;
        Scheduler::set_timeout(5000,[](){
          Comms::set_done(Comms::Robot_id::Robot1);
          busy_junction_flag = false;
        });
        return;
      }

      if(Comms::get_state(Comms::Robot_id::Robot2) == Comms::Request_type ::Requested)
      {
        Comms::set_accepted(Comms::Robot_id::Robot2);
        busy_junction_flag = true;
        Scheduler::set_timeout(5000,[](){
          Comms::set_done(Comms::Robot_id::Robot2);
          busy_junction_flag= false;
        });
        return;
      }

      static bool priority_robot1 = false;
      if((Comms::get_state(Comms::Robot_id::Robot1) == Comms::Request_type ::Requested) &&  
          (Comms::get_state(Comms::Robot_id::Robot2)== Comms::Request_type ::Requested))
      {
        if(priority_robot1)
        {
          Comms::set_accepted(Comms::Robot_id::Robot1);
          busy_junction_flag = true;
          Scheduler::set_timeout(5000,[](){
          Comms::set_done(Comms::Robot_id::Robot1);
          busy_junction_flag = false;
          });
        }
        else{
          Comms::set_accepted(Comms::Robot_id::Robot2);
          busy_junction_flag= true;
          Scheduler::set_timeout(5000,[](){
          Comms::set_done(Comms::Robot_id::Robot2);
          busy_junction_flag= false;
          });
        }
        priority_robot1 = !priority_robot1;
      }
  }

  /*State Machine declaration*/
  static inline constexpr auto connecting_state = make_state(General_states::Connecting,
        Transition<General_states>{General_states::Operational, []() { return Comms::is_connected(); }}
    );

  static inline constexpr auto operational_state = make_state(General_states::Operational,
        Transition<General_states>{General_states::Fault, []() { return !Comms::is_connected(); }}
    );

  static inline constexpr auto fault_state = make_state(General_states::Fault);

  static inline constexpr auto junction_ready_state = make_state(Operational_states::Junction_ready,
        Transition<Operational_states>{Operational_states::Junction_busy, []() { 
          return (Comms::get_state(Comms::Robot_id::Robot1) == Comms::Request_type ::Accepted) ||  
          (Comms::get_state(Comms::Robot_id::Robot2)== Comms::Request_type ::Accepted);
        }}
    );

  static inline constexpr auto junction_busy_state = make_state(Operational_states::Junction_busy,
        Transition<Operational_states>{Operational_states::Junction_ready, []() { 
          return (Comms::get_state(Comms::Robot_id::Robot1) == Comms::Request_type ::None) &&  
          (Comms::get_state(Comms::Robot_id::Robot2)== Comms::Request_type ::None); 
          }}
    );

  static inline constinit auto Nested_state_machine = [junction_ready_state,junction_busy_state]()consteval{
    auto sm = make_state_machine(Operational_states::Junction_ready, junction_ready_state,junction_busy_state);
    using namespace std::chrono_literals;

    /*--------Junction Ready----------*/

    sm.add_enter_action([](){
      Actuators::set_led_blue(true);
    },junction_ready_state);


    sm.add_cyclic_action([](){
      Board::control_loop();
    },10ms,junction_ready_state);
    

    /*--------Junction Busy----------*/

    // sm.add_enter_action([](){
    //   Actuators::levantar_barrera?
    // },junction_busy_state);

    sm.add_cyclic_action([](){
      static bool toggle = true;
      Actuators::set_led_blue(toggle);
      toggle =!toggle;
    },250ms,junction_ready_state);

    return sm;
  }();

  static inline constinit auto State_machine = [connecting_state,operational_state,fault_state]()consteval{
    auto sm = make_state_machine(General_states::Connecting, connecting_state,operational_state,fault_state);
    using namespace std::chrono_literals;

    sm.add_cyclic_action([](){
      static bool toggle = true;
      Actuators::set_led_green(toggle);
      toggle = !toggle;
    },250ms,connecting_state);

    sm.add_enter_action([](){
      Actuators::set_led_red(true);
    },fault_state);

    sm.add_state_machine(Nested_state_machine,operational_state);

    return sm;
  }();

  public: 
  static void start()
  {
    State_machine.start();

    Scheduler::register_task(10,[](){
      State_machine.check_transitions();
    });

  }

};