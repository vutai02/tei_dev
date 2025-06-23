#pragma once

#include "storage/nvs/StorageNvsE.h"
#include "ConfigConstant.h"
using namespace fireAlarm::storage;
using namespace smooth::core::logging;
class ObjectValueTimerNow
{
public:
    ObjectValueTimerNow() = default;

    ObjectValueTimerNow(int32_t &timer_output, int32_t &timer_feedback, int32_t &timer_trigger_fire)
    {

        if (StorageNvsE::instance().read(fireAlarm::common::TIMER_FEEDBACK, &feedback))
        {
            if (feedback == 0)
            {
                timer_feedback = 5;
            }
            else
            {
                timer_feedback = feedback / 1000;
            }
        }
        if (StorageNvsE::instance().read(fireAlarm::common::TIMER_OUTPUT, &output))
        {
            if (output == 0)
            {
                timer_output = 120;
            }
            else
            {
                timer_output = output / 1000;
            }
        }
        if (StorageNvsE::instance().read(fireAlarm::common::TIMER_TRIGGER, &trigger))
        {
            if (trigger == 0)
            {
                timer_trigger_fire = 20000;
            }
            else
            {
                timer_trigger_fire = trigger;
            }
        }
    }

private:
    int32_t output = 0;
    int32_t feedback = 0;
    int32_t trigger = 0;
};