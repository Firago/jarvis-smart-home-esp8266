/*
  CountdownTimer.h - small util library which allows to create 
  a countdown timer and check if it is already expired
  Created by Dmytro Firago, September 27, 2016.
*/

#ifndef CountdownTimer_h
#define CountdownTimer_h

#include "Arduino.h"

class Timer {
	
  private:
  
	long _expireTime;
	
  public:
  
	Timer(long period);
	
	static Timer* start(long period);
	
	boolean expired();
};

#endif