

#include "app.h"


uint16_t TEMP_ADDR = 0b0000000010010000; // Use 8-bit address
uint8_t AMB_ADDR[1] = {0b00000000};
uint8_t RET_TEMP[2] = {0};
uint8_t CONFIG_BUFFER[2] = {0b00000001, 0b01100000};
uint16_t current_temp = 0;
float ftemp = 0;
int isneg = 0;
HAL_StatusTypeDef ret;


void temp_setup();
float get_ambient();




float get_ambient(){
	HAL_I2C_Master_Transmit(&hi2c2, TEMP_ADDR, AMB_ADDR, 1, 1000);
	HAL_I2C_Master_Receive(&hi2c2, TEMP_ADDR, RET_TEMP, 2, 1000);
	current_temp = (RET_TEMP[0] << 8) | RET_TEMP[1];

	if(current_temp & 0b1000000000000000){
		//temp is negative
		isneg = 1;
		current_temp = (~current_temp) + 1;
	}else{
		isneg = 0;
	}
	ftemp = 0;
	if(current_temp & 0b0100000000000000){
		ftemp = ftemp + 64;
	}
	if(current_temp & 0b0010000000000000){
		ftemp = ftemp + 32;
		}
	if(current_temp & 0b0001000000000000){
		ftemp = ftemp + 16;
		}
	if(current_temp & 0b0000100000000000){
		ftemp = ftemp + 8;
		}
	if(current_temp & 0b0000010000000000){
		ftemp = ftemp + 4;
		}
	if(current_temp & 0b0000001000000000){
		ftemp = ftemp + 2;
		}
	if(current_temp & 0b0000000100000000){
		ftemp = ftemp + 1;
		}
	if(current_temp & 0b0000000010000000){
		ftemp = ftemp + 0.5;
		}
	if(current_temp & 0b0000000001000000){
		ftemp = ftemp + 0.25;
		}
	if(current_temp & 0b0000000000100000){
		ftemp = ftemp + 0.125;
		}
	if(current_temp & 0b0000000000010000){
		ftemp = ftemp + .0625;
		}
	return ftemp;
}


void temp_setup(){
	ret = HAL_I2C_Master_Transmit(&hi2c2, 0b0000000010010000 , CONFIG_BUFFER, 2, 10000);
}


