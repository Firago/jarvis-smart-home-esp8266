#include "CountdownTimer.h"

Timer::Timer(long period) {
  _expireTime = millis() + period;
}

Timer* Timer::start(long period) {
  return new Timer(period);
}

boolean Timer::expired() {
  return millis() > _expireTime;
}