#include "input.h"  
#include <esp_timer.h>

InputAlarm::InputAlarm(gpio_num_t io,
                       bool pull_up,
                       bool pull_down, uint32_t time_is_fire, bool sta_trigger) : Input(io, pull_up, pull_down), io(io)
{
#define TIMER_TRIGGER std::chrono::milliseconds(time_is_fire)
    // set_Fire_Time(TIMER_TRIGGER);
    status_trigger = sta_trigger;
}   
    
void InputAlarm::check_trigger_input(CallbackFunc f)
{
    input = f;
}

void InputAlarm::check_not_trigger(CallbackFunc f)
{
    not_input = f;
}
// void InputAlarm::set_Fire_Time(std::chrono::milliseconds ms)
// {
//     is_timer_trigger = ms.count() * 1000;
// }

void InputAlarm::tick()
{
    if (io > -1)
    {
       bool new_state = read(); 
    if (new_state != state )
    {    
        state = new_state;
        if(state == false)
        {
            if(input != NULL)
            input(*this);
            Log::info(INPUT_TAG, "input:{}",io ,state);
        }
        else if(state == true)
        {   
            if(not_input != NULL)
            not_input(*this);
            Log::info(INPUT_TAG, "not_input:{}",io ,state);
        }
    }
}
}
