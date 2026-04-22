/*
	Button - a small library for Arduino to handle button debouncing
	
	MIT licensed.
*/

#include "Button.h"
#include <Arduino.h>

Button::Button(uint8_t pin, uint16_t debounce_ms, uint16_t avg_count)
:  _pin(pin)
,  _delay(debounce_ms)
,  _state(HIGH)
,  _ignore_until(0)
,  _has_changed(false)
,  _avg_count(avg_count)
{
}

void Button::begin()
{
	pinMode(_pin, INPUT_PULLUP);
}

// 
// public methods
// 

bool Button::read()
{
	uint16_t sum = 0;
	int digRead;

  //Read pin _avg_count times
	for (int i = 0; i < _avg_count; i++) {
		sum += digitalRead(_pin);
	}

  //check if all readings agree. If not, abort
	if (sum == _avg_count) {
		digRead = HIGH;
	} else if (sum == 0) {
		digRead = LOW;
	} else {
		return _state;
	}

	// ignore pin changes until after this delay time
	if (_ignore_until > millis())
	{
		// ignore any changes during this period
	}
	// pin has changed 
	else if (digRead != _state)
	{
		_ignore_until = millis() + _delay;
		_state = !_state;
		_has_changed = true;
	}
	
	return _state;
}

// has the button been toggled from on -> off, or vice versa
bool Button::toggled()
{
	read();
	return has_changed();
}

// mostly internal, tells you if a button has changed after calling the read() function
bool Button::has_changed()
{
	if (_has_changed)
	{
		_has_changed = false;
		return true;
	}
	return false;
}

// has the button gone from off -> on
bool Button::pressed()
{
	return (read() == PRESSED && has_changed());
}

// has the button gone from on -> off
bool Button::released()
{
	return (read() == RELEASED && has_changed());
}
