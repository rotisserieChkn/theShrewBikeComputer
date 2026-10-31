#include "app.h"









//huge wall of global variables, poorly organized in no meaningful fashion, sorry
char tempchar[7] = { 'X' };
char odometerchar[7] = { 'X' };
char mylatlon[GNSS_DATA_BYTE_LEN] = "11111111111111111111111111111\r\n";
char menu0_erase[] = " ERASE ALL";
char menu1_print[] = " PRINT";
char menu2_exit[] = " EXIT";
char readout_buf[500] = { 'X' };
int total_transmitted = 0;
char speedchar[9] = { 'X' };
float current_speed = 1.23456;
float myambienttemp = 0;
float odometer = 0;
uint8_t menucounter = 2;
uint8_t menuopen = 0;
uint32_t flash_address_index = FLASH_START_INDEX;
uint8_t currentview = 0;
const uint32_t numClicks = 0x0A;
int gpsDelayCounter = 0;
char gpgllOnlyPls[] = "$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35\r\n";
char uartRXbufferIT[GNSS_UART_BUF_SIZE] = "";
char aboutToPrintMessage[] = "PRINTING GPS COORDINATES\r\n";
char numCoordsMessage[] = "Number of points to print: ";
char numCoordsChar[] = "XXXX";
char newline[] = "\r\n";
volatile int buttonLeftFlag = 0;
volatile int buttonRightFlag = 0;
volatile int gnssUartDoneFlag = 0;
volatile int wheelCounterFullFlag = 0;
uint32_t sessionMilliseconds = 0;
uint32_t sessionHours = 0;
uint32_t sessionMinutes = 0;
char sessionHoursChar[] = "00";
char sessionMinutesChar[] = "00";
char combinedTimeChar[] = "00:00";
volatile uint32_t last_button_interrupt_time = 0;


void bikeComputerInit(){
	
	//read the last saved address index value
	//the value at this address holds the value of the highest memory address written to in the FRAM chip
	flash_address_index = flash_readindex();

	//initialize some peripheral chips, with delays in between just for good measure
	displayInit();
	HAL_Delay(100);

	temp_setup();
	HAL_Delay(100);

	initGNSSmodule();
	HAL_Delay(100);

	//start timers
	HAL_TIM_Base_Start(&htim2);

	//start LPTIM to count pulses. some custom setup is necessary so I wrote a quick function for this
	initLPTIM1custom();

	//display a welcome message
	displaySplashScreen();
	//this is the odometer reading, the default first view
	displaySmallTextLOC("MILES", 5, 20, 208);

	//start the uartbuf gps NMEA collection interrupt
	HAL_UART_Receive_IT(&huart2, uartRXbufferIT, GNSS_UART_BUF_SIZE);

}





void bikeCOmputerMainLoop(){
			//check to see if any buttons have been pressed and act accordingly

////////////////////////////////////////IF BUTTON 1 IS PRESSED//////////////////////////////////////////////////////////

		//this button (right) opens the menu and makes selections when the menu is open
		if (buttonRightFlag == 1) {
			if (menuopen == 0) {
				//tell everyone the menu is now open, reset the menu counter, and make sure nothing else updates the display
				menuopen = 1;
				menucounter = 2;
				currentview = 100;
				//display the menu options to the user
				displayTotalClear();
				displaySmallTextLOC(menu0_erase, sizeof(menu0_erase), 10, 208);
				displaySmallTextLOC(menu1_print, sizeof(menu1_print), 10 + 24,
						208);
				displaySmallTextLOC(menu2_exit, sizeof(menu2_exit), 10 + 24 * 2,
						208);
				//highlight the first default selection
				displaySmallTextLOC("-", 1, 10 + 24 * 2, 208);
			} else {
				// otherwise, the menu is already open and user is making a selection
				switch (menucounter) {
				case 0:
					//erase all glps data saved to the FRAM
					//this basically boils down to just resetting the FRAM index counter to 0, we don't really need to erase all the data written to the chip
					flash_set_index(FLASH_START_INDEX);
					flash_address_index = FLASH_START_INDEX;
					displayTotalClear();
					displaySmallTextLOC("ERASED", 5, 10, 208);
					for (int po = 0; po < 60000000; po++) {
						asm("nop");
					}
					displayTotalClear();
					break;

				case 1:
					//print out gps over serial
					//print out all memory contents
					snprintf(numCoordsChar, 5, "%04d",
							(flash_readindex() - FLASH_START_INDEX)
									/ GNSS_DATA_BYTE_LEN);
					CDC_Transmit_FS(aboutToPrintMessage, 26);
					nonHALdelay(10000000);
					CDC_Transmit_FS(numCoordsMessage, 27);
					nonHALdelay(10000000);
					CDC_Transmit_FS(numCoordsChar, 4);
					nonHALdelay(10000000);
					CDC_Transmit_FS(newline, 2);
					nonHALdelay(10000000);
					while (total_transmitted
							< (flash_address_index - FLASH_START_INDEX)) {
						flash_read(total_transmitted + FLASH_START_INDEX,
								readout_buf, 100);
						CDC_Transmit_FS(readout_buf, 100);
						total_transmitted = total_transmitted + 100;
					}
					total_transmitted = 0;
					break;

				case 2:
					//do nothing and exit menu
					displayTotalClear();
					displaySmallTextLOC("EXIT", 4, 10, 208);
					for (int po = 0; po < 60000000; po++) {
						asm("nop");
					}
					displayTotalClear();
					break;
				}				//end switch case

				//at this point the user has made a menu selection, it has been carried out, and we need to close the menu and update the display
				menuopen = 0;
				currentview = 0;
			}
			//un-press the right button
			buttonRightFlag = 0;

////////////////////////////////////////IF BUTTON 2 IS PRESSED//////////////////////////////////////////////////////////

			// this button (left) changes the main view or cycles which menu selection is selected in menu mode
		} else if (buttonLeftFlag == 1) {
			if (menuopen == 0) {
				//if the menu is closed, user is just cycling to the next view page
				displayTotalClear();
				currentview++;
				switch (currentview) {
				case 0:
					//this is the odometer reading
					displaySmallTextLOC("MILES", 5, 20, 208);
					break;

				case 1:
					//this is the speed reading
					displaySmallTextLOC("MPH", 3, 20, 208);
					break;

				case 2:
					//this is the gps reading
					displaySmallTextLOC("GPS", 3, 20, 208);
					break;

				case 3:
					//this is the temperature reading
					displaySmallTextLOC("TEMP CELSIUS", 12, 20, 208);
					break;

				case 4:
					//wrap around to the start, the odometer reading
					currentview = 0;
					displaySmallTextLOC("MILES", 5, 20, 208);
					break;
				}
			} else {
				//if the menu is open, the user is changing the highlighted menu option
				menucounter++;
				if (menucounter == 3) {
					menucounter = 0;
				}
				//erase current selection
				for (int b = 0; b < 3; b++) {
					displaySmallTextLOC(" ", 1, 10, 208);
					displaySmallTextLOC(" ", 1, 10 + 24, 208);
					displaySmallTextLOC(" ", 1, 10 + 24 * 2, 208);
				}

				switch (menucounter) {
				case 0:
					displaySmallTextLOC("-", 1, 10, 208);
					break;
				case 1:
					displaySmallTextLOC("-", 1, 10 + 24, 208);
					break;
				case 2:
					displaySmallTextLOC("-", 1, 10 + 24 * 2, 208);
					break;
				}				//end switch menu counter
			}
			//reset the left button flag to get ready for the next left button press whenever the user decides to do so
			buttonLeftFlag = 0;
		}				//end button handling stuff


		//if the menu is not open:
		if (menuopen == 0) {
			// look for a new GPS location every once in a while
			//this is kinda not great, the way we measure the passage of time is just by burning cycles in this while loop
			//most of the time we are not really doing anything so it should be pretty consistent timing from measurement to measurement
			if (gpsDelayCounter == 10000000) {
				gpsDelayCounter = 0;
				//start looking for gps uart comms, read temperature data, and update the session timer display
				//GNSS UART is interrupt driven, so all this function does is kick off the collection of UART data
				//whenever the collection is complete, the callback will set a flag and be handled lower in this main while loop
				//callback is defined in the gnssUartISR.c file
				HAL_UART_Receive_IT(&huart2, uartRXbufferIT, GNSS_UART_BUF_SIZE);

				//oh yeah also get the temperature reading from the temp sensor ic
				myambienttemp = get_ambient();
				snprintf(tempchar, sizeof tempchar, "%d%02d\n", (int) myambienttemp, (int) (myambienttemp * 100) - (int) myambienttemp * 100);
				//display the temperature data if that view is active
				if (currentview == 3) {
					LCDclrbigtext();
					LCDbigtext(tempchar, 3, 0);
					displaySmallTextLOC("TEMP CELSIUS", 12, 20, 208);
					decimalDot();
				}

				//update the session counter
				//this keeps track of how long the bike computer has been on since being powered on
				sessionMilliseconds = HAL_GetTick();
				sessionHours = sessionMilliseconds / 3600000;
				sessionMinutes = (sessionMilliseconds - (sessionHours * 3600000)) / 60000;

				snprintf(sessionHoursChar, sizeof sessionHoursChar, "%02d",	sessionHours);
				snprintf(sessionMinutesChar, sizeof sessionMinutesChar, "%02d",	sessionMinutes);

				combinedTimeChar[0] = sessionHoursChar[0];
				combinedTimeChar[1] = sessionHoursChar[1];
				combinedTimeChar[2] = ':';
				combinedTimeChar[3] = sessionMinutesChar[0];
				combinedTimeChar[4] = sessionMinutesChar[1];

				displaySmallTextLOC(combinedTimeChar, 5, 202, 208);

				// just do the normal routine stuff every loop. meaning, check to see if there are any flags set or any work to be done as indicated by interrupt flags
			} else {
				gpsDelayCounter++;

				//check if there is a complete GNSS capture ready for us
				if (gnssUartDoneFlag == 1) {
					//reset the flag
					gnssUartDoneFlag = 0;
					//this next function is a big one
					todoWhenGnssUartDone();
				}

				//check if the wheel has rotated the required number of times
				if (wheelCounterFullFlag == 1) {
					//reset the flag
					wheelCounterFullFlag = 0;
					todoWhenCounterFull();
				}
			}
		}				//end if menu closed
	
	
}




void nonHALdelay(int numNOP) {
	for (int po = 0; po < numNOP; po++) {
		asm("nop");
	}
}

void initGNSSmodule() {
	//set gnss module to only output GPGLL statements
	HAL_UART_Transmit(&huart2, gpgllOnlyPls, 57, 5000);
	nonHALdelay(500000);
	HAL_UART_Transmit(&huart2, gpgllOnlyPls, 57, 5000);
}

void initLPTIM1custom() {
	//start LPTIM to count pulses. some custom setup is necessary
	HAL_LPTIM_Counter_Start_IT(&hlptim1, numClicks);
	nonHALdelay(500);
	LPTIM1->CR |= 0b000;
	nonHALdelay(500);
	LPTIM1->CR |= 0b001;
	nonHALdelay(500);
	LPTIM1->ARR = numClicks;
	nonHALdelay(500);
	LPTIM1->CMP = numClicks - 1;
	nonHALdelay(500);
	LPTIM1->CR |= 0b101;
	nonHALdelay(500);
}

void displaySplashScreen() {
	displayTotalClear();
	displaySmallTextLOC("HELLO     ", 10, 50, 208);
	//while(1){}
	HAL_Delay(2000);
	displayTotalClear();
}

/////////////ISR Callback Functions///////////////////////////////
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	//clear interrupt flag
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);

	uint32_t now = HAL_GetTick();

	if ((now - last_button_interrupt_time) > 300) { // 50 ms debounce

		last_button_interrupt_time = now;

		if (GPIO_Pin == GPIO_PIN_13) {
			buttonRightFlag = 1;
		}

		if (GPIO_Pin == GPIO_PIN_14) {
			buttonLeftFlag = 1;
		}
	}
} //end button press ISR

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {

	gnssUartDoneFlag = 1;

} //End GNSS UART Done ISR

// Callback: timer has rolled over, meaning
//we have traveled  10995mm
//how much time has elapsed since last interrupt?
void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim) {

	if (hlptim == &hlptim1) {

		wheelCounterFullFlag = 1;

		//we have traveled  10995mm, add that to odometer
		//how much time has elapsed since last interrupt?
		//use that to calculate speed
		current_speed = 4181543624.0 / ((float) (TIM2->CNT));
		odometer = odometer + 0.0068319763;

		TIM2->CNT = 0x0;
		LPTIM1->ICR = 0xFF;
	}
}





