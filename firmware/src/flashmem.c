
#include "app.h"


#define FLASH_READ 0x03
#define FLASH_WRIT 0x02
#define FLASH_WREN 0x06
#define FLASH_RDSR 0x05





//activate the chip select pin of the fram module
//all the "nop" commands are just to add a timing delay without the need to call the HAL_Delay() function that relies on the SysTick timer
void flash_select(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}


//deactivate the chip select pin of the fram module
//all the "nop" commands are just to add a timing delay without the need to call the HAL_Delay() function that relies on the SysTick timer
void flash_deselect(){
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
}



//send the WREN command to the fram
void flash_wren(){
	uint8_t flashquickbuf1[1] = {FLASH_WREN};
	flash_select();
	HAL_SPI_Transmit(&hspi1, flashquickbuf1, 1, 100);
	flash_deselect();
}




//read the status register of the fram chip
int flash_readsr(){
	uint8_t flashquickbuf2[1] = {FLASH_RDSR};
	uint8_t flashquickread1[1] = {0};
	flash_select();
	HAL_SPI_Transmit(&hspi1, flashquickbuf2, 1, 100);
	HAL_SPI_Receive(&hspi1, flashquickread1, 1, 100);
	flash_deselect();
	return flashquickread1[0];
}




//repeatedly read the status register until the fram indicates that it is ready
//this will block infinitely if the fram never becomes ready
void flash_wait_for_ready(){
	uint8_t flash_test_byte = 0XFF;
	int condition = 1;
	while(condition){
		flash_test_byte = flash_readsr();
		if(flash_test_byte & 0b00000010){
			condition = 0;
		}else{
			condition = 1;
		}
	}
}



//write data to the fram starting at a specified memory address
//right now this function will only write 100 or less memory locations at a time
void flash_write(uint32_t start_address, uint8_t writebuf[], int buf_len){
	if(buf_len > 100){
		//we simply cannot handle more data than 100 bytes to write at one time. sorry. good day.
		return;
	}

	uint8_t flash_outbuf[110] = {0};
	flash_outbuf[0] = FLASH_WRIT;
	flash_outbuf[1] = (uint8_t)(start_address >> 8*2);
	flash_outbuf[2] = (uint8_t)(start_address >> 8*1);
	flash_outbuf[3] = (uint8_t)(start_address >> 8*0);

	for(int x=0; x<buf_len; x++){
		flash_outbuf[4+x] = writebuf[x];
	}

	flash_wren();
	flash_wait_for_ready();
	flash_select();
	HAL_SPI_Transmit(&hspi1, flash_outbuf, buf_len+4, 1000);
	flash_deselect();
}



//read fram data starting at a specified memory address, store in a buffer that is passed to this function
//only reading 100 values or less at a time is supported
void flash_read(uint32_t start_address, uint8_t myflashreadbuf[], int buf_len){

	if(buf_len > 100){
		//we simply cannot handle more data than 100 bytes to write at one time. sorry. good day.
		return;
	}

	uint8_t flash_outbuf[110] = {0};
	flash_outbuf[0] = FLASH_READ;
	flash_outbuf[1] = (uint8_t)(start_address >> 8*2);
	flash_outbuf[2] = (uint8_t)(start_address >> 8*1);
	flash_outbuf[3] = (uint8_t)(start_address >> 8*0);


	flash_select();
	HAL_SPI_Transmit(&hspi1, flash_outbuf, 4, 1000);
	HAL_SPI_Receive(&hspi1, myflashreadbuf, buf_len, 1000);
	flash_deselect();
}





//write a value stored at address 0x00 in the fram, I use this as a pointer that keeps track of the last memory location written to in the fram
void flash_set_index(uint32_t index_val){

	uint8_t myBuf[3] = {0};

	myBuf[0] = (uint8_t)(index_val >> 16);
	myBuf[1] = (uint8_t)(index_val >> 8);
	myBuf[2] = (uint8_t)(index_val >> 0);

	flash_write(0, myBuf, 3);
}



//read the value stored at address 0x00 in the fram
uint32_t flash_readindex(){

	uint8_t flashInBuf[10] = {0};

	flash_read(0, flashInBuf, 3);

	return (((uint32_t)flashInBuf[0])<<16) + ((uint32_t)(flashInBuf[1])<<8) + ((uint32_t)(flashInBuf[2])<<0);
}





