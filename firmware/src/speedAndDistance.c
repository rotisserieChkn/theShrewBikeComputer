/*
 * speedAndDistance.c
 *
 *  Created on: Aug 13, 2025
 *      Author: nicho
 */


#include "app.h"


void todoWhenCounterFull(){

	if (currentview == 0) {
		LCDclrbigtext();
		snprintf(odometerchar, sizeof odometerchar, "%02d%d\n",	(int) odometer,	(int) (odometer * 100) - (int) odometer * 100);
		LCDbigtext(odometerchar, 3, 0);
		displaySmallTextLOC("MILES", 5, 20, 208);
		decimalDot();
	}
	if (currentview == 1) {
		LCDclrbigtext();
		snprintf(speedchar, sizeof speedchar, "%02d%d\n", (int) current_speed, (int) (current_speed * 100) - (int) current_speed * 100);
		LCDbigtext(speedchar, 3, 0);
		displaySmallTextLOC("MPH", 3, 20, 208);
		decimalDot();
	}
}
