#pragma once

#include <time.h>

extern "C"
{
#include "ccronexpr/ccronexpr.h"
}

#include "common/IoInterface.h"

#define dtNBR_ALARMS 22

#define USE_SPECIALIST_METHODS // define this for testing

typedef uint8_t CronID_t;
typedef CronID_t CronId;

#define dtINVALID_ALARM_ID 255
#define dtINVALID_TIME (time_t)(-1)

typedef void (*OnTick_t)(void *); // alarm callback function typedef

// class defining an alarm instance, only used by dtAlarmsClass
class CronEventClass
{
public:
  CronEventClass();
  void updateNextTrigger();
  void updatePrevTrigger();
  cron_expr expr;
  OnTick_t onTickHandler;
  iot_tp_dev_t arg;
  time_t nextTrigger;
  time_t previousTrigger;
  bool isEnabled; // the timer is only actioned if isEnabled is true
  bool isOneShot; // the timer will be de-allocated after trigger is processed
};

// class containing the collection of alarms
class CronClass
{
private:
  CronEventClass Alarm[dtNBR_ALARMS];
  uint8_t isServicing;
  uint8_t servicedCronId; // the alarm currently being serviced
  void serviceAlarms();

public:
  CronClass();

  // Function to create alarms and timers with cron
  CronID_t create(char *cronstring, OnTick_t onTickHandler, bool isOneShot);
  CronID_t create(char *cronstring, OnTick_t onTickHandler, iot_tp_dev_t arg, bool isOneShot);
  // isOneShot - trigger once at the given time in the future

  // Function that must be evaluated often (at least once every main loop)
  void delay(unsigned long ms = 0);
  void execute();

  // low level methods
  void enable(CronID_t ID);            // enable the alarm to trigger
  void disable(CronID_t ID);           // prevent the alarm from triggering
  CronID_t getTriggeredCronId() const; // returns the currently triggered  alarm id
  bool getIsServicing() const;         // returns isServicing

  void free(CronID_t ID); // free the id to allow its reuse

  time_t getPrevTrigger(CronID_t ID); // returns the time of the scheduled alarm
  
  int comparetime(time_t time1,time_t time2);
#ifndef USE_SPECIALIST_METHODS
private: // the following methods are for testing and are not documented as part of the standard library
#endif
  uint8_t count() const;                    // returns the number of allocated timers
  time_t getNextTrigger() const;            // returns the time of the next scheduled alarm
  time_t getNextTrigger(CronID_t ID) const; // returns the time of scheduled alarm
  bool isAllocated(CronID_t ID) const;      // returns true if this id is allocated
};

extern CronClass Cron; // make an instance for the user
