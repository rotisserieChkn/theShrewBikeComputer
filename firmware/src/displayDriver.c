

#include "app.h"
#include "uglyFont.h"


// DEFINES FOR DISPLAY PINS
#define DISP_CS_PIN GPIO_PIN_2
#define DISP_CS_BANK GPIOB

#define DISP_RST_PIN GPIO_PIN_12
#define DISP_RST_BANK GPIOB

#define DISP_WR_PIN GPIO_PIN_11
#define DISP_WR_BANK GPIOB



uint16_t halfDisplayBitstream[6250*5] = {0};

//call this function to singnal to the display that a command is being sent
void displaySignalCommand(){
	HAL_GPIO_WritePin(DISP_WR_BANK, DISP_WR_PIN, GPIO_PIN_RESET);
}

//call this function to singnal to the display that data is being sent
void displaySignalData(){
	HAL_GPIO_WritePin(DISP_WR_BANK, DISP_WR_PIN, GPIO_PIN_SET);
}

//activate chip select for display
void displayChipSelect(){
	HAL_GPIO_WritePin(DISP_CS_BANK, DISP_CS_PIN, GPIO_PIN_RESET);
}


void displayChipDeSelect(){
	HAL_GPIO_WritePin(DISP_CS_BANK, DISP_CS_PIN, GPIO_PIN_SET);
}


void displaySendDummyCommand(){
	uint8_t dummy = 0;
	displaySignalCommand();
	displayIssueCommand(0x00, &dummy, 0);
}



void displayHardwareReset(){
	//HARDWARE RESET
	HAL_GPIO_WritePin(DISP_RST_BANK, DISP_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(DISP_RST_BANK, DISP_RST_PIN, GPIO_PIN_RESET);
	HAL_Delay(200);
	HAL_GPIO_WritePin(DISP_RST_BANK, DISP_RST_PIN, GPIO_PIN_SET);
	HAL_Delay(200);
}




void displayIssueCommand(uint8_t command_num, uint8_t param_data[], int num_param){

	uint8_t current_command [1] = {command_num};

	displayChipSelect();
	displaySignalCommand();
	HAL_SPI_Transmit(&hspi2, current_command, 1, 100);
	if(num_param == 0){
		displayChipDeSelect();
		return;
	}
	displaySignalData();
	HAL_SPI_Transmit(&hspi2, param_data, num_param, 100);
	displayChipDeSelect();
	return;
}




void displayInit(){

	uint8_t param_data_array[5] = {0};

	displayChipDeSelect();
	displayHardwareReset();

	//exit sleep mode (SLPOUT)
	displayIssueCommand(0x11, param_data_array, 0);

	//enter normal on mode (NORON)
	displayIssueCommand(0x13, param_data_array, 0);

	// turn the display on (DISPON)
	displayIssueCommand(0x29, param_data_array, 0);

	//set brightness (WRDISBV)
	param_data_array[0] = 0xFF;
	displayIssueCommand(0x51, param_data_array, 1);

	//set the pixel data format to 5-6-5
	param_data_array[0] = 0x05;
	displayIssueCommand(0x3A, param_data_array, 1);

}


//tell the display what rectangular sub-section of the display any subsequent data should be written to
void displaySetSubArea(uint16_t lowRow, uint16_t highRow, uint16_t lowCol, uint16_t highCol){

	uint8_t colParams[4] = {(uint8_t)lowCol>>8, (uint8_t)lowCol&0x00FF, (uint8_t)highCol>>8, (uint8_t)highCol&0x00FF};
	uint8_t rowParams[4] = {(uint8_t)lowRow>>8, (uint8_t)lowRow&0x00FF, (uint8_t)highRow>>8, (uint8_t)highRow&0x00FF};

	displayIssueCommand(0x2A, colParams, 4);
	displayIssueCommand(0x2B, rowParams, 4);
}



//clear the ENTIRE display area
void displayTotalClear(){

	displaySetSubArea(0, 250, 0, 250);

	displayChipSelect();
	displaySignalCommand();
	// Send command indicating writing display data (0x2C)
	uint8_t myCommand[1] = {0x2C};
	HAL_SPI_Transmit(&hspi2, myCommand, 1, 100);
	displaySignalData();


	// write the actual display data

	//init array of all 0xFFFF
    for (int i = 0; i < 6250; i++) {
    	halfDisplayBitstream[i] = 0xFFFF;
    }

    //transmit the buffer of all 1's 10 times to cover all the pixels in the entire display
	for(int y=0; y<10; y++){
		HAL_SPI_Transmit(&hspi2, (uint8_t *)halfDisplayBitstream, 6250*2, 10000);
	}

	// write dummy command to signal the end of display data transfer
	displaySendDummyCommand();

	//deselect the display SPI bus
	displayChipDeSelect();

}




//function used to display the large numbers in the middle of the display
void LCDbigtext(char *oledtext, int len, int invert){

	uint8_t *bigbuf = NULL;

	//set sub-area for area of first big letter
	 uint16_t ylow = 160;
	 uint16_t yhigh = 223;
	 uint16_t xlow = 60;
	 uint16_t xhigh = 171;

	//for each letter
	for(int n=0;n<len;n++){

		if(oledtext[n] == '0'){
			bigbuf = &big0;
		}else if(oledtext[n] == '1'){
			bigbuf = &big1;
		}else if(oledtext[n] == '2'){
			bigbuf = &big2;
		}else if(oledtext[n] == '3'){
			bigbuf = &big3;
		}else if(oledtext[n] == '4'){
			bigbuf = &big4;
		}else if(oledtext[n] == '5'){
			bigbuf = &big5;
		}else if(oledtext[n] == '6'){
			bigbuf = &big6;
		}else if(oledtext[n] == '7'){
			bigbuf = &big7;
		}else if(oledtext[n] == '8'){
			bigbuf = &big8;
		}else if(oledtext[n] == '9'){
			bigbuf = &big9;
		}else{
			bigbuf = &bigSpace;
		}

		displaySetSubArea(xlow, xhigh, ylow, yhigh);
		displayWritePixelBuf(bigbuf, (xhigh+1-xlow)*(yhigh+1-ylow));
		//move sub area to the next big letter area
		ylow = ylow-64;
		yhigh = yhigh-64;
		}

}


//displya a dot on the display to represent the decimal point for the large numbers I write to the center of the display
void decimalDot(){

	displaySetSubArea(55, 64, 95, 104);
	displayChipSelect();
	displaySignalCommand();

	// Send command indicating writing display data
	uint8_t myCommand[1] = {0x2C};
	HAL_SPI_Transmit(&hspi2, myCommand, 1, 100);

	displaySignalData();


	// write the actual display data
	uint16_t allZeros[81] = {0};
	HAL_SPI_Transmit(&hspi2, (uint8_t *)allZeros, 81*2, 100);

	// write dummy command to signal the end of display data transfer
	displaySendDummyCommand();

	//Unselect the display SPI bus
	displayChipDeSelect();
}





//clear only the portion of the display that I write the large numbers to
void LCDclrbigtext(){

	uint8_t *bigbuf = NULL;

	//set sub-area for area of first big letter
	 uint16_t ylow = 160;
	 uint16_t yhigh = 223;
	 uint16_t xlow = 60;
	 uint16_t xhigh = 171;

	 bigbuf = &bigSpace;

	for(int n=0;n<3;n++){
		displaySetSubArea(xlow, xhigh, ylow, yhigh);
		displayWritePixelBuf(bigbuf, (xhigh+1-xlow)*(yhigh+1-ylow));
		//move sub area to the next big letter area
		ylow = ylow-64;
		yhigh = yhigh-64;
	}

}








//write small text to the display in a specified location
void displaySmallTextLOC(char *oledtext, int len, uint16_t yinit, uint16_t xinit){

	uint8_t *bigbuf = NULL;

	 uint16_t ylow;
	 uint16_t yhigh;
	 uint16_t xlow;
	 uint16_t xhigh;


      ylow = xinit;
	  yhigh = xinit + 15;
	  xlow = yinit;
	  xhigh = yinit+23;

	//for each letter
	for(int n=0;n<len;n++){

		if(oledtext[n] == '0'){
			bigbuf = &number0;
		}else if(oledtext[n] == '1'){
			bigbuf = &number1;
		}else if(oledtext[n] == '2'){
			bigbuf = &number2;
		}else if(oledtext[n] == '3'){
			bigbuf = &number3;
		}else if(oledtext[n] == '4'){
			bigbuf = &number4;
		}else if(oledtext[n] == '5'){
			bigbuf = &number5;
		}else if(oledtext[n] == '6'){
			bigbuf = &number6;
		}else if(oledtext[n] == '7'){
			bigbuf = &number7;
		}else if(oledtext[n] == '8'){
			bigbuf = &number8;
		}else if(oledtext[n] == '9'){
			bigbuf = &number9;
		}else if(oledtext[n] == 'A'){
			bigbuf = &letterA;
		}else if(oledtext[n] == 'B'){
			bigbuf = &letterB;
		}else if(oledtext[n] == 'C'){
			bigbuf = &letterC;
		}else if(oledtext[n] == 'D'){
			bigbuf = &letterD;
		}else if(oledtext[n] == 'E'){
			bigbuf = &letterE;
		}else if(oledtext[n] == 'F'){
			bigbuf = &letterF;
		}else if(oledtext[n] == 'G'){
			bigbuf = &letterG;
		}else if(oledtext[n] == 'H'){
			bigbuf = &letterH;
		}else if(oledtext[n] == 'I'){
			bigbuf = &letterI;
		}else if(oledtext[n] == 'J'){
			bigbuf = &letterJ;
		}else if(oledtext[n] == 'K'){
			bigbuf = &letterK;
		}else if(oledtext[n] == 'L'){
			bigbuf = &letterL;
		}else if(oledtext[n] == 'M'){
			bigbuf = &letterM;
		}else if(oledtext[n] == 'N'){
			bigbuf = &letterN;
		}else if(oledtext[n] == 'O'){
			bigbuf = &letterO;
		}else if(oledtext[n] == 'P'){
			bigbuf = &letterP;
		}else if(oledtext[n] == 'Q'){
			bigbuf = &letterQ;
		}else if(oledtext[n] == 'R'){
			bigbuf = &letterR;
		}else if(oledtext[n] == 'S'){
			bigbuf = &letterS;
		}else if(oledtext[n] == 'T'){
			bigbuf = &letterT;
		}else if(oledtext[n] == 'U'){
			bigbuf = &letterU;
		}else if(oledtext[n] == 'V'){
			bigbuf = &letterV;
		}else if(oledtext[n] == 'W'){
			bigbuf = &letterW;
		}else if(oledtext[n] == 'X'){
			bigbuf = &letterX;
		}else if(oledtext[n] == 'Y'){
			bigbuf = &letterY;
		}else if(oledtext[n] == 'Z'){
			bigbuf = &letterZ;
		}else if(oledtext[n] == '.'){
			bigbuf = &dot;
		}else if(oledtext[n] == ':'){
			bigbuf = &colon;
		}else if(oledtext[n] == '-'){
			bigbuf = &minus;
		}else{
			bigbuf = &blankSpace;
		}

		displaySetSubArea(xlow, xhigh, ylow, yhigh);
		displayWritePixelBuf(bigbuf, (xhigh+1-xlow)*(yhigh+1-ylow));
		ylow = ylow-16;
		yhigh = yhigh-16;
		}

}














//the pixel buf sent to this function holds 1 bit per pixel
//meaning it can only hold "black and white" data, just to save storage data on the mcu
//but, the display is a color display, expecting several bits per pixel, depending on the initialization setting
//this converts the one bit per pixel data sent to this function to the many bits per pixel that the display is expecting, and send it over the SPI interface
//this funciton assumes that the display sub-area has already been set using the displaySetSubArea function
void displayWritePixelBuf(uint8_t pixelBuf[], int numPixels){


	if(numPixels > 6250*5){
		return;
	}

	uint16_t letterColor = 0x0000;
	uint16_t backgroundColor = 0xFFFF;
	uint16_t whatever[1] = {0};

	uint8_t currentPixelColor = 0;

	int bitcounter = 0;
	int bytecounter = 0;

	displayChipSelect();
	displaySignalCommand();



	// Send command indicating writing display data
	uint8_t myCommand[1] = {0x2C};
	HAL_SPI_Transmit(&hspi2, myCommand, 1, 100);

	displaySignalData();

	// build the bitstream from the array containing one bit per pixel
	for(int y=0; y<numPixels; y++){

		currentPixelColor = pixelBuf[bytecounter] & (0b10000000 >> bitcounter);

		if (currentPixelColor == 0){
			halfDisplayBitstream[y] = letterColor;
		}else{halfDisplayBitstream[y] = backgroundColor;}

		bitcounter++;
		if(bitcounter == 8){
			bytecounter++;
			bitcounter = 0;
		}
	}

	//actually transmit the pixel bitstream
	HAL_SPI_Transmit(&hspi2, (uint8_t *)halfDisplayBitstream, numPixels*2, 100);

	// write dummy command to signal the end of display data transfer
	displaySendDummyCommand();

	//Unselect the display SPI bus
	displayChipDeSelect();
}





