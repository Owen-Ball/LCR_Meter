// Modified version of the Button library that supports reading from a pin multiple times and checking
// for agreement between all readings 

// Also added functionality for repeated triggers when holding button

// Author: Michael Adams (<http://www.michael.net.nz>)
// Copyright (C) 2016 Michael D K Adams.
// Released under the MIT license.

#ifndef _BUTTON_
#define _BUTTON_
#include "Arduino.h"

class Button
{
	public:
		Button(uint8_t pin, uint16_t debounce_ms = 100, uint16_t avg_count = 1, long unsigned int hold_blank = 0, long unsigned int hold_delay = 0);
		void begin();
		bool read();
		bool toggled();
		bool pressed();
		bool released();
		bool has_changed();
    bool process_hold();
		
		const static bool PRESSED = LOW;
		const static bool RELEASED = HIGH;
	
	private:
		uint8_t  _pin;
		uint16_t _delay;
		bool     _state;
		uint32_t _ignore_until;
		bool     _has_changed;
		uint16_t _avg_count;
    long unsigned int _next_increment;
    long unsigned int _hold_blank;
    long unsigned int _hold_delay;
};


#endif
