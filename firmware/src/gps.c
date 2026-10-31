



#include "app.h"


int findClosestIndex(int arr[], int size, int target);



int arctanLUT[90] = {0 , 1 , 3 , 5 , 6 , 8 , 10 , 12 , 14 , 15 , 17 , 19 , 21 , 23 , 24 , 26 , 28 , 30 , 32 , 34 , 36 , 38 , 40 , 42 , 44 , 46 , 48 , 50 , 53 , 55 , 57 , 60 , 62 , 64 , 67 , 70 , 72 , 75 , 78 , 80 , 83 , 86 , 90 , 93 , 96 , 99 , 103 , 107 , 111 , 115 , 119 , 123 , 127 , 132 , 137 , 142 , 148 , 153 , 160 , 166 , 173 , 180 , 188 , 196 , 205 , 214 , 224 , 235 , 247 , 260 , 274 , 290 , 307 , 327 , 348 , 373 , 401 , 433 , 470 , 514 , 567 , 631 , 711 , 814 , 951 , 1143 , 1430 , 1908 , 2863 , 5728};

char gps_uartbuf_char_IT[GNSS_UART_BUF_SIZE] = {'A'};
char GPSOutputchar[] = "$PMTK314,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*35\r\n";
char DDlatitudeChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char DDlongitudeChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char latLonRatioChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char deltaLatChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char deltaLonChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char finalBearingChar[] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";
char saveToFlashBuffer[27] = "XXXXXXXXXXXXXXXXXXXXXXXX\r\n";

int latitudeDegNum = 0;
int latitudeMinNum = 0;
int longitudeDegNum = 0;
int longitudeMinNum = 0;

int OLDlatitudeDegNum = 0;
int OLDlongitudeDegNum = 0;

int deltaLat = 0;
int deltaLon = 0;

int latLonRatio = 1;

int prelimArctanOutput = 0;
int finalBearing = 0;






int getlatlon_IT(char latlon[], char receivedRXbuffer[]){
  char *startGngllAddr = NULL;

  char* end_of_array;
  const char gps_gngll[] = "GNGLL";

  int x = 0;
  //copy data from RX buffer passed into this function into our own local buffer, for no real reason I guess
  for(x=0; x<GNSS_UART_BUF_SIZE; x++){
	  gps_uartbuf_char_IT[x] = (char)receivedRXbuffer[x];
  }
  //make sure there is a proper termination character as the last element in the array or the strstr() funciton won't work
  gps_uartbuf_char_IT[GNSS_UART_BUF_SIZE-1] = '\0';
  end_of_array = &gps_uartbuf_char_IT[GNSS_UART_BUF_SIZE-1];
  //look for the string "GNGLL" in the RX buffer, because what follows is the NMEA statement we are looking for!!
  startGngllAddr = strstr(gps_uartbuf_char_IT, gps_gngll);

  if(startGngllAddr == NULL){
	  //DID NOT FIND GPNLL
	  HAL_UART_Transmit(&huart2, GPSOutputchar, 57, 5000);
	  return 0;
  }else if((end_of_array - startGngllAddr) < 50){
	  //WE DID FIND GNGLL, BUT IT STARTED TOO CLOSE TO THE END OF THE ARRAY, AND THE FULL NMEA STATEMENT WOULD HAVE BEEN CUT OFF
	  return 0;
  }else if((startGngllAddr[7] == ',') && (startGngllAddr[8] == ',')){
	  //found GNGLL but no fix. If there is no satellite fix, there will be commas in these locations
	  return 0;
  }else{
	//fix found!!!! extract floats from string
	latlon[0] = *(startGngllAddr+6);
	latlon[1] = *(startGngllAddr+7);
	latlon[2] = *(startGngllAddr+8);
	latlon[3] = *(startGngllAddr+9);
	latlon[4] = *(startGngllAddr+10);
	latlon[5] = *(startGngllAddr+11);
	latlon[6] = *(startGngllAddr+12);
	latlon[7] = *(startGngllAddr+13);
	latlon[8] = *(startGngllAddr+14);
	latlon[9] = *(startGngllAddr+18);
	latlon[10] = *(startGngllAddr+19);
	latlon[11] = *(startGngllAddr+20);
	latlon[12] = *(startGngllAddr+21);
	latlon[13] = *(startGngllAddr+22);
	latlon[14] = *(startGngllAddr+23);
	latlon[15] = *(startGngllAddr+24);
	latlon[16] = *(startGngllAddr+25);
	latlon[17] = *(startGngllAddr+26);
	latlon[18] = *(startGngllAddr+27);
	latlon[19] = *(startGngllAddr+31);
	latlon[20] = *(startGngllAddr+32);
	latlon[21] = *(startGngllAddr+33);
	latlon[22] = *(startGngllAddr+34);
	latlon[23] = *(startGngllAddr+35);
	latlon[24] = *(startGngllAddr+36);
	latlon[25] = *(startGngllAddr+37);
	latlon[26] = *(startGngllAddr+38);
	latlon[27] = *(startGngllAddr+39);
	latlon[28] = *(startGngllAddr+40);
	latlon[29] = '\r';
	latlon[30] = '\n';
	latlon[31] = '\0';

	return 1;
  }
} //end getlatlon function





//this function is called from the main loop when a GNSS UART collection is complete
//this function does the following:
//checks that the GNSS statement is valid,
//converts the ASCII data into decimal degrees lattitude and longitude,
//calculates bearing based on previous and current coordinates
//saves the current coordinates to FRAM memory chip
void todoWhenGnssUartDone(){
	int test_cond = 0;

	test_cond = getlatlon_IT(mylatlon, uartRXbufferIT);
	if(test_cond == 1){

		latitudeDegNum = ((mylatlon[0]-'0')*ONE_DEG_MULT)*10 + ((mylatlon[1]-'0')*ONE_DEG_MULT)*1;
		latitudeMinNum = ((mylatlon[2]-'0')*ONE_DEG_MULT)*10 + ((mylatlon[3]-'0')*ONE_DEG_MULT)*1 + ((mylatlon[5]-'0')*ONE_DEG_MULT)/10 + ((mylatlon[6]-'0')*ONE_DEG_MULT)/100 + ((mylatlon[7]-'0')*ONE_DEG_MULT)/1000 + ((mylatlon[8]-'0')*ONE_DEG_MULT)/10000;
		latitudeDegNum = latitudeDegNum + latitudeMinNum/60;

		snprintf(DDlatitudeChar, sizeof DDlatitudeChar, "%09d",latitudeDegNum);


		longitudeDegNum = ((mylatlon[9]-'0')*ONE_DEG_MULT)*100 + ((mylatlon[10]-'0')*ONE_DEG_MULT)*10 + ((mylatlon[11]-'0')*ONE_DEG_MULT)*1;
		longitudeMinNum = ((mylatlon[12]-'0')*ONE_DEG_MULT)*10 + ((mylatlon[13]-'0')*ONE_DEG_MULT)*1 + ((mylatlon[15]-'0')*ONE_DEG_MULT)/10 + ((mylatlon[16]-'0')*ONE_DEG_MULT)/100 + ((mylatlon[17]-'0')*ONE_DEG_MULT)/1000 + ((mylatlon[18]-'0')*ONE_DEG_MULT)/10000;
		longitudeDegNum = (longitudeDegNum + longitudeMinNum/60)*(-1);

		snprintf(DDlongitudeChar, sizeof DDlongitudeChar, "%09d",longitudeDegNum);

		if (currentview == 2) {

			if(latitudeDegNum < 0){
				displaySmallTextLOC("-", 1, 10+24*3, 208-16);				//display latitude
			}
			displaySmallTextLOC(DDlatitudeChar+2, 2, 10+24*3, 208-16*2);	//display latitude
			displaySmallTextLOC(".", 1, 10+24*3, 208-16*4);					//display latitude
			displaySmallTextLOC(DDlatitudeChar+4, 5, 10+24*3, 208-16*5);	//display latitude


			if(longitudeDegNum < 0){
				displaySmallTextLOC("-", 1, 10+24*2, 208);					//display longitude
			}
			displaySmallTextLOC(DDlongitudeChar+1, 3, 10+24*2, 208-16);		//display longitude
			displaySmallTextLOC(".", 1, 10+24*2, 208-16*4);					//display longitude
			displaySmallTextLOC(DDlongitudeChar+4, 5, 10+24*2, 208-16*5);	//display longitude


			displaySmallTextLOC(mylatlon+19, 2, 20, 208);					//display time
			displaySmallTextLOC(":", 1, 20, 208-16*2);						//display time
			displaySmallTextLOC(mylatlon+21, 2, 20, 208-16*3);				//display time
			displaySmallTextLOC(":", 1, 20, 208-16*5);						//display time
			displaySmallTextLOC(mylatlon+23, 2, 20, 208-16*6);				//display time
		}


		deltaLat = latitudeDegNum - OLDlatitudeDegNum;
		deltaLon = longitudeDegNum - OLDlongitudeDegNum;

		//multiply by 1000 so that I do not lose precision in the subsequent divide operation since i am not using floating point numbers
		deltaLat = deltaLat * 1000;
		deltaLon = deltaLon * 1000;

		snprintf(deltaLatChar, sizeof deltaLatChar, "%09d",deltaLat);
		snprintf(deltaLonChar, sizeof deltaLonChar, "%09d",deltaLon);

		latLonRatio = abs((deltaLat*100)/deltaLon);

		snprintf(latLonRatioChar, sizeof latLonRatioChar, "%09d",latLonRatio);

		//I use a look up table to calculate the arctan of the ratio of latitude to longitude
		//the arctan of each integer degree value up to 90 is stored in the look up table.
		//finding the index of the value in the array closest to my value is therefore approximately the arctan of my value
		prelimArctanOutput = findClosestIndex(arctanLUT, 90, latLonRatio);

		//determine quadrant
		if((deltaLon>=0) && (deltaLat>=0)){
			//quadrant 1
			finalBearing = 90-prelimArctanOutput;

		}else if((deltaLon<=0) && (deltaLat>=0)){
			//quadrant 2
			finalBearing = 270+prelimArctanOutput;

		}else if((deltaLon<=0) && (deltaLat<=0)){
			//quadrant 3
			finalBearing = 270-prelimArctanOutput;

		}else{
			//quadrant 4
			finalBearing = 90+prelimArctanOutput;
		}

		snprintf(finalBearingChar, sizeof finalBearingChar, "%03d",finalBearing);

		OLDlatitudeDegNum = latitudeDegNum;
		OLDlongitudeDegNum = longitudeDegNum;

		displaySmallTextLOC(finalBearingChar, 3, 202, 50);

		//write the latitude, longitude, and time to flash
		//there is really no reason to store these values as ascii instead of just raw integers in memory
		//i just do that because I am lazy and it is easy to display them when they are printed over the USB virtual com port interface
		for(int x=0; x<9; x++){
			saveToFlashBuffer[x] = DDlatitudeChar[x];
			saveToFlashBuffer[x+9] = DDlongitudeChar[x];
		}
		for(int x=0; x<6; x++){
			saveToFlashBuffer[x+18] = mylatlon[x+19];
		}
		flash_write(flash_address_index, (uint8_t*)saveToFlashBuffer, GNSS_DATA_BYTE_LEN);
		flash_address_index = flash_address_index + GNSS_DATA_BYTE_LEN;
		flash_set_index(flash_address_index);
	}
}





//
int findClosestIndex(int arr[], int size, int target){
    // Handle edge cases
    if (size == 0) return -1; // Empty array
    if (size == 1) return 0;  // Only one element

    // If target is smaller than the first element
    if (target <= arr[0]) return 0;
    // If target is larger than the last element
    if (target >= arr[size - 1]) return size - 1;

    // Binary search to find the insertion point
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (arr[mid] == target) {
            return mid; // Exact match
        } else if (arr[mid] < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    // After binary search, 'left' is the insertion point
    // Compare arr[left-1] and arr[left] to find the closest
    if (left == 0) return 0; // Shouldn't happen due to earlier check
    if (left == size) return size - 1; // Shouldn't happen due to earlier check

    // Compare absolute differences
    int diff1 = abs(arr[left - 1] - target);
    int diff2 = abs(arr[left] - target);

    return diff1 <= diff2 ? left - 1 : left;
}


