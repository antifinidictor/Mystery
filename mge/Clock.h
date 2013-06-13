/*
 * Clock.h
 * Defines the standard clock class.
 */

#ifndef CLOCK_H
#define CLOCK_H

#include "SDL.h"
#include "mge/defs.h"

 /*
  * Clock
  * A class managing time for the game.
  */
class Clock {
private:
	//Instance variables
	uint   m_uiCurTime,		//Current game time
		   m_uiActTime,		//Actual SDL ticks when CurTime was last updated
		   m_uiTotalTicks;	//Total times update was called (used for average)
	float  m_fRate;			//Rate at which in-game time flows

public:
	//Constructor(s)/Destructor
	Clock() {
		m_uiCurTime = 0;
		m_uiActTime = SDL_GetTicks();
		m_fRate = 1;
		m_uiTotalTicks = 1;	//Otherwise divide by 0 error, I think
	}
	virtual ~Clock() {}	/* Nothing to clear at the moment. */

	//General Methods
	void update() {
		//Update current time: SDL ticks - previous ticks = new ticks, * rate = speed time flows
		m_uiCurTime += (uint)((SDL_GetTicks() - m_uiActTime) * m_fRate);
		m_uiActTime = SDL_GetTicks();
		m_uiTotalTicks++;
	}

	//Accessor Methods
	void setRate(float fRate)				{ m_fRate = fRate; }
	uint getTime() 							{ return m_uiCurTime; }
	uint getTimePassed( uint uiPrevTime )	{ return m_uiCurTime - uiPrevTime; }
	uint getAverage()						{ return m_uiCurTime / m_uiTotalTicks; }
};

#endif
