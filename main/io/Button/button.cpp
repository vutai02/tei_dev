#include "button.h"
#include <esp_timer.h>
#include <smooth/core/logging/log.h>
#include "esp_log.h"
using namespace std;
using namespace std::chrono;
using namespace smooth::core::logging;
static constexpr const char *BUTTON_TAG = "BUTTON_tag";
Button::Button(gpio_num_t io) : Input(io), io(io)
{
    setDebounceTime(DEBOUNCE_MS);
    setSmartTime(SMART_MODE_MS);
    setManualTime(MANUAL_MODE_MS);
    setRestTime(RESET_MODE);
    pressed = true;
    state = 1;
}

Button::Button(gpio_num_t io,
               bool pull_up,
               bool pull_down) : Input(io, pull_up, pull_down), io(io)
{
    setDebounceTime(DEBOUNCE_MS);
    setSmartTime(SMART_MODE_MS);
    setManualTime(MANUAL_MODE_MS);
    setRestTime(RESET_MODE);
    pressed = pull_up ? false : true;
    state = pull_up ? 0 : 1;
}

void Button::setDebounceTime(std::chrono::milliseconds ms)
{
    debounce_time_us = ms.count() * 1000;
}

void Button::setSmartTime(std::chrono::milliseconds ms)
{
    smart_time_us = ms.count() * 1000;
}

void Button::setManualTime(std::chrono::milliseconds ms)
{
    manual_time_us = ms.count() * 1000;
}
void Button::setRestTime(std::chrono::milliseconds ms)
{
    reset_time_us = ms.count() * 1000;
}
void Button::setSmart(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "set_smart");
    smart_cb = f;
}
void Button::setManual(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "setManual");
    manual_cb = f;
}
void Button::setReset(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "setReset");
    reset_cb = f;
}

void Button::setSmartLed(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "set_led_smart");
    smart_led_cb = f;
}
void Button::setManualLed(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "set_led_Manual");
    manual_led_cb = f;
}
void Button::setResetLed(CallbackFunc f)
{
    // Log::info(BUTTON_TAG, "set_Led_Reset");
    reset_led_cb = f;
}

void Button::tick()
{
    if (io > -1)
    {
        // us
        unsigned long now = esp_timer_get_time();
        state = read();
       // Log::info(BUTTON_TAG, "STATE:{0}:{1}", state, io);
        // state =1
        if (state != pressed)
        {
            flag_set_time = true;
            if (!led_smart)
            {
                Log::info(BUTTON_TAG, "Mode_ smart");
                if (smart_cb != NULL)
                    smart_cb(*this);
                led_smart = true;
            }

            else if (!led_manual)
            {
                Log::info(BUTTON_TAG, "Mode_ manual");
                if (manual_cb != NULL)
                    manual_cb(*this);
                led_manual = true;
            }

            else if (!led_reset)
            {
                if (reset_cb != NULL)
                    reset_cb(*this);
                Log::info(BUTTON_TAG, "Mode_ reset");
                led_reset = true;
            }
        }
        // state = 0
        if (state == pressed)
        {
            if (flag_set_time)
            {
                prev_led_time = now;
                flag_set_time = false;
            }
            else if ((now - prev_led_time) > smart_time_us && (now - prev_led_time) < manual_time_us && led_smart)
            {
                Log::info(BUTTON_TAG, "led_ smart");
                if (smart_led_cb != NULL)
                    smart_led_cb(*this);
                led_smart = false;
                led_manual = true;
                led_reset = true;
            }
            else if ((now - prev_led_time) > manual_time_us && (now - prev_led_time) < reset_time_us && led_manual)
            {
                Log::info(BUTTON_TAG, "led_manual");
                if (manual_led_cb != NULL)
                    manual_led_cb(*this);
                led_manual = false;
                led_smart = true;
                led_reset = true;
            }

            else if ((now - prev_led_time) > reset_time_us && led_reset)
            {
                Log::info(BUTTON_TAG, "led_ reset");
                if (reset_led_cb != NULL)
                    reset_led_cb(*this);
                led_reset = false;
                led_manual = true;
                led_smart = true;
            }
        }
    }
}