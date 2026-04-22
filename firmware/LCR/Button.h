// Modified version of the Button library that supports reading from a pin multiple times and checking
// for agreement between all readings 

// Author: Michael Adams (<http://www.michael.net.nz>)
// Copyright (C) 2016 Michael D K Adams.
// Released under the MIT license.

#ifndef Button_h
#define Button_h
#include "Arduino.h"

class Button
{
	public:
		Button(uint8_t pin, uint16_t debounce_ms = 100, uint16_t avg_count = 1);
		void begin();
		bool read();
		bool toggled();
		bool pressed();
		bool released();
		bool has_changed();
		
		const static bool PRESSED = LOW;
		const static bool RELEASED = HIGH;
	
	private:
		uint8_t  _pin;
		uint16_t _delay;
		bool     _state;
		uint32_t _ignore_until;
		bool     _has_changed;
		uint16_t _avg_count;
};

#endif
