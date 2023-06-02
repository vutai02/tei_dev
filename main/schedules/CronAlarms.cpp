#include "CronAlarms.h"
#include <cstring>
#include <sstream>
#include <algorithm>
#include <functional>
#include <iostream>

extern "C"
{
#include "ccronexpr/ccronexpr.h"
}

//**************************************************************
//* Cron Event Class Constructor

CronEventClass::CronEventClass()
{
  memset(&expr, 0, sizeof(expr));
  onTickHandler = NULL; // prevent a callback until this pointer is explicitly set
  memset(&arg, 0, sizeof(arg));
  nextTrigger = 0;
  isEnabled = isOneShot = false;
}

//**************************************************************
//* Cron Event Class Methods

void CronEventClass::updateNextTrigger()
{
  if (isEnabled) {
    time_t timenow = time(nullptr);
    if (onTickHandler != NULL && nextTrigger <= timenow) {
      // update alarm if next trigger is not yet in the future
      nextTrigger = cron_next(&expr, timenow);
    }
  }
}

void CronEventClass::updatePrevTrigger()
{
  if (isEnabled) {
    time_t timenow = time(nullptr);
    if (previousTrigger <= timenow) {
      // update alarm if previous trigger is not yet in the future
      previousTrigger = cron_prev(&expr, timenow);
    }
  }
}

//**************************************************************
//* Cron Class Public Methods

CronClass::CronClass()
{
  isServicing = false;
  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    free(id); // ensure all Alarms are cleared and available for allocation
  }
}

void CronClass::enable(CronID_t ID)
{
  if (isAllocated(ID)) {
    Alarm[ID].isEnabled = true;
    Alarm[ID].updateNextTrigger();
    Alarm[ID].updatePrevTrigger();
  }
}

void CronClass::disable(CronID_t ID)
{
  if (isAllocated(ID)) {
    Alarm[ID].isEnabled = false;
  }
}

void CronClass::free(CronID_t ID)
{
  if (isAllocated(ID)) {
    memset(&(Alarm[ID].expr), 0, sizeof(Alarm[ID].expr));
    Alarm[ID].onTickHandler = NULL;
    memset(&(Alarm[ID].arg), 0, sizeof(Alarm[ID].arg));
    Alarm[ID].nextTrigger = 0;
    Alarm[ID].isEnabled = false;
    Alarm[ID].isOneShot = false;
  }
}

// returns the number of allocated timers
uint8_t CronClass::count() const
{
  uint8_t c = 0;
  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (isAllocated(id)) c++;
  }
  return c;
}

// returns true if this id is allocated
bool CronClass::isAllocated(CronID_t ID) const
{
  return (ID < dtNBR_ALARMS && Alarm[ID].onTickHandler != NULL);
}

// returns the currently triggered alarm id
// returns dtINVALID_ALARM_ID if not invoked from within an alarm handler
CronID_t CronClass::getTriggeredCronId() const
{
  if (isServicing) {
    return servicedCronId; // new private data member used instead of local loop variable i in serviceAlarms();
  }
  else {
    return dtINVALID_ALARM_ID; // valid ids only available when servicing a callback
  }
}

int CronClass::comparetime(time_t time1,time_t time2) {
  return difftime(time1,time2) > 0.0 ? 1 : -1; 
} 

// following functions are not Alarm ID specific.
void CronClass::delay(unsigned long ms)
{
  // unsigned long start = millis();
  // do {
  //   serviceAlarms();
  //   std::this_thread::sleep_for(std::chrono::milliseconds(1));
  // } while (millis() - start  <= ms);
}

void CronClass::execute()
{
  serviceAlarms();
}

//returns isServicing
bool CronClass::getIsServicing() const
{
  return isServicing;
}

//***********************************************************
//* Private Methods

void CronClass::serviceAlarms()
{
  if (!isServicing) {
    isServicing = true;
    for (servicedCronId = 0; servicedCronId < dtNBR_ALARMS; servicedCronId++) {
      if (Alarm[servicedCronId].isEnabled && (time(nullptr) >= Alarm[servicedCronId].nextTrigger)) {
        iot_tp_dev_t arg = Alarm[servicedCronId].arg;
        OnTick_t TickHandler = Alarm[servicedCronId].onTickHandler;
        if (Alarm[servicedCronId].isOneShot) {
          free(servicedCronId); // free the ID if mode is OnShot
        }
        else {
          Alarm[servicedCronId].updateNextTrigger();
        }
        if (TickHandler != NULL) {
          (*TickHandler)(&arg); // call the handler
        }
      }
      if (Alarm[servicedCronId].isEnabled && (time(nullptr) >= Alarm[servicedCronId].previousTrigger)) {
        Alarm[servicedCronId].updatePrevTrigger();
      }
    }
    isServicing = false;
  }
}

// returns the absolute time of the next scheduled alarm, or 0 if none
time_t CronClass::getNextTrigger() const
{
  time_t nextTrigger = 0;

  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (isAllocated(id)) {
      if (nextTrigger == 0) {
        nextTrigger = Alarm[id].nextTrigger;
      }
      else if (Alarm[id].nextTrigger < nextTrigger){
        nextTrigger = Alarm[id].nextTrigger;
      }
    }
  }
  return nextTrigger;
}

time_t CronClass::getNextTrigger(CronID_t ID) const
{
  if (isAllocated(ID)){
    return Alarm[ID].nextTrigger;
  }
  else {
    return 0;
  }
}

time_t CronClass::getPrevTrigger(CronID_t ID) 
{
  if (isAllocated(ID)){
    enable(ID);
    return Alarm[ID].previousTrigger;
  }
  else {
    return 0;
  }
}

// attempt to create a cron alarm and return CronID if successful
CronID_t CronClass::create(char *cronstring, OnTick_t onTickHandler, bool isOneShot)
{
  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (!isAllocated(id)) {
      // here if there is an Alarm id that is not allocated
      const char *err = NULL;
      memset(&(Alarm[id].expr), 0, sizeof(Alarm[id].expr));
      cron_parse_expr(cronstring, &(Alarm[id].expr), &err);
      if (err) {
        memset(&(Alarm[id].expr), 0, sizeof(Alarm[id].expr));
        return dtINVALID_ALARM_ID;
      }
      Alarm[id].onTickHandler = onTickHandler;
      Alarm[id].isOneShot = isOneShot;
      enable(id);
      return id; // alarm created ok
    }
  }
  return dtINVALID_ALARM_ID; // no IDs available or time is invalid
}

CronID_t CronClass::create(char *cronstring, OnTick_t onTickHandler, iot_tp_dev_t arg_tp, bool isOneShot)
{
  for (uint8_t id = 0; id < dtNBR_ALARMS; id++) {
    if (!isAllocated(id)) {
      // here if there is an Alarm id that is not allocated
      const char *err = NULL;
      memset(&(Alarm[id].expr), 0, sizeof(Alarm[id].expr));
      cron_parse_expr(cronstring, &(Alarm[id].expr), &err);
      if (err) {
        memset(&(Alarm[id].expr), 0, sizeof(Alarm[id].expr));
        return dtINVALID_ALARM_ID;
      }
      Alarm[id].onTickHandler = onTickHandler;
      Alarm[id].arg.tp = (int)arg_tp.tp;
      Alarm[id].arg.id = id;
      Alarm[id].arg.value = arg_tp.value;
      Alarm[id].isOneShot = isOneShot;
      enable(id);
      return id; // alarm created ok
    }
  }
  return dtINVALID_ALARM_ID; // no IDs available or time is invalid
}
// make one instance for the user to use
CronClass Cron = CronClass();
