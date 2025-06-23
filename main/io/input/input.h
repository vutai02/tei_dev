#include <memory>
#include <chrono>
#include <algorithm>
#include <smooth/core/io/Input.h>
#include <smooth/core/logging/log.h>

using namespace std;
using namespace std::chrono;
using namespace smooth::core::logging;

enum Type
{
    PROTOTYPE_ = 0, // input
    PROTOTYPE_2,    // for output 3
    PROTOTYPE_3,
};

static constexpr const char *INPUT_TAG = "INPUT";

class InputAlarm : private smooth::core::io::Input
{

protected:
    typedef void (*CallbackFunc)(InputAlarm &);

    // bool press = false;

    CallbackFunc input = NULL; // what is the name of IO that will be trigger
    CallbackFunc not_input = NULL;
    int64_t is_timer_trigger;
    unsigned long long prev_fire_time = 0;
    bool heal_check_time = false;

public:
    bool status_trigger;
    bool is_trigger = false;
    bool is_not_triger = false; // variable that is pushed to server and know the state of this input
    bool ignore_triger = false;
    gpio_num_t io;
    bool state;
    InputAlarm(gpio_num_t io,
               bool pull_up,
               bool pull_down, uint32_t time_is_fire, bool sta_triger);
   //  InputAlarm() {}
   // explicit InputAlarm(gpio_num_t io);
    void tick();
    void check_trigger_input(CallbackFunc f);
    void check_not_trigger(CallbackFunc f);
   // void set_ire_Time(std::chrono::miliseconds ms);
};