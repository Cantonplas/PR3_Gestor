#pragma once
#include <Arduino.h>
#include <atomic>

#define MAX_TASKS 32 
typedef void (*TaskCallback)();

class Scheduler {
private:
    inline static  std::atomic<uint32_t> sys_ticks{0};
    inline static hw_timer_t *timer{nullptr};

    inline static portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
    
    inline static constinit uint32_t active_tasks = 0;  
    inline static constinit uint32_t oneshot_tasks = 0; 

    inline static constinit uint32_t next_run[MAX_TASKS];
    inline static constinit uint32_t periods[MAX_TASKS];
    
    inline static constinit TaskCallback callbacks[MAX_TASKS];

    static void ARDUINO_ISR_ATTR onTimer() {
        sys_ticks++;
    }

public:
    static uint32_t get_global_time()
    {
        return sys_ticks;
    }

    static void start() {
        timer = timerBegin(1000000); 
        timerAttachInterrupt(timer, &onTimer);
        timerAlarm(timer, 1000, true, 0);  //1000us de prescaler da 1ms por cada tick :p
    }

    static int register_task(uint32_t period_ms,TaskCallback cb) {
        if (active_tasks == 0xFFFFFFFF) return -1; 
        portENTER_CRITICAL(&timerMux); 
        int id = __builtin_ctz(~active_tasks); 

        active_tasks |= (1 << id);   
        oneshot_tasks &= ~(1 << id); 
        portEXIT_CRITICAL(&timerMux);
        periods[id] = period_ms;
        next_run[id] = sys_ticks + period_ms;
        callbacks[id] = cb;

        return id;
    }

    static int set_timeout(uint32_t delay_ms,TaskCallback cb) {
        if (active_tasks == 0xFFFFFFFF) return -1;

        portENTER_CRITICAL(&timerMux); 
        int id = __builtin_ctz(~active_tasks); 

        active_tasks |= (1 << id);
        oneshot_tasks |= (1 << id); 
        portEXIT_CRITICAL(&timerMux);  


        periods[id] = delay_ms;
        next_run[id] = sys_ticks + delay_ms;
        callbacks[id] = cb;

        return id;
    }

    static void unregister_task(int id) {
        if (id >= 0 && id < MAX_TASKS) {
          portENTER_CRITICAL(&timerMux); 
            active_tasks &= ~(1 << id);
            callbacks[id] = nullptr;
          portEXIT_CRITICAL(&timerMux);
        }
    }

    static void update() {
        uint32_t current_ticks = sys_ticks; 
        uint32_t pending_map = active_tasks; 

        while (pending_map) {
            int i = __builtin_ctz(pending_map); 
            pending_map &= ~(1 << i); 

            if ((active_tasks & (1 << i)) == 0) {
                continue; 
            }

            if ((int32_t)(current_ticks - next_run[i]) >= 0) {
                next_run[i] = current_ticks + periods[i]; 
                
                if (callbacks[i] != nullptr) {
                    callbacks[i](); 
                }
                
                if (oneshot_tasks & (1 << i)) {
                    portENTER_CRITICAL(&timerMux); 
                    active_tasks &= ~(1 << i);
                    callbacks[i] = nullptr;
                    portEXIT_CRITICAL(&timerMux);
                }
            }
        }
    }
};


