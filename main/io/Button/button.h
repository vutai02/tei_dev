#include <memory>
#include <chrono>
#include <algorithm>
#include <smooth/core/io/Input.h>

#define DEBOUNCE_MS std::chrono::milliseconds(50)
#define SMART_MODE_MS std::chrono::milliseconds(5000)
#define MANUAL_MODE_MS std::chrono::milliseconds(10000)
#define RESET_MODE std::chrono::milliseconds(15000)
#define SINGLE_CLICK 1
#define DOUBLE_CLICK 2
#define TRIPLE_CLICK 3
#define LONG_CLICK 4

class Button : private smooth::core::io::Input
{
protected:
    gpio_num_t io;
    bool prev_state;
    bool state = true;
    bool pressed = false;
    unsigned long down_ms;
    unsigned long prev_led_time = 0;
    int64_t debounce_time_us;
    int64_t smart_time_us;
    int64_t manual_time_us;
    int64_t reset_time_us;
    unsigned long down_time_ms = 0;
    bool pressed_triggered = false;

    typedef void (*CallbackFunc)(Button &);

    CallbackFunc smart_cb = NULL;
    CallbackFunc manual_cb = NULL;
    CallbackFunc reset_cb = NULL;
    CallbackFunc smart_led_cb = NULL;
    CallbackFunc manual_led_cb = NULL;
    CallbackFunc reset_led_cb = NULL;

public:
    void tick();
    explicit Button(gpio_num_t io);
    Button(gpio_num_t io,
           bool pull_up,
           bool pull_down);

    void setDebounceTime(std::chrono::milliseconds ms);
    void setSmartTime(std::chrono::milliseconds ms);
    void setManualTime(std::chrono::milliseconds ms);
    void setRestTime(std::chrono::milliseconds ms);

    void setSmart(CallbackFunc f);
    void setManual(CallbackFunc f);
    void setReset(CallbackFunc f);

    void setSmartLed(CallbackFunc f);
    void setManualLed(CallbackFunc f);
    void setResetLed(CallbackFunc f);

    // bool flag_led = true;
    bool led_smart = true;
    bool led_manual = true;
    bool led_reset = true;
    bool flag_set_time = true;
};