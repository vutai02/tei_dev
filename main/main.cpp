#include "App.h"
#include <smooth/core/SystemStatistics.h>

using namespace iotTouch;

extern "C"
{
  void app_main()
  {
    App app{};
    app.start();
  }
}
