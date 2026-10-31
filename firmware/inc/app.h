




#include "main.h"

void nonHALdelay(int numNOP);
void initGNSSmodule();
void initLPTIM1custom();
void displaySplashScreen();
void bikeComputerInit();
void bikeCOmputerMainLoop();

#define WHEEL_DIAMETER_MM 700.0
#define FLASH_START_INDEX 100
#define GNSS_UART_BUF_SIZE 150
#define GNSS_DATA_BYTE_LEN 26
#define ONE_DEG_MULT 100000


//a bunch of global variables LOL
extern char tempchar[7];
extern char odometerchar[7];
extern char mylatlon[GNSS_DATA_BYTE_LEN];
extern char menu0_erase[];
extern char menu1_print[];
extern char menu2_exit[];
extern char readout_buf[500];
extern int total_transmitted;
extern char speedchar[9];
extern float current_speed;
extern float myambienttemp;
extern float odometer;
extern uint8_t menucounter;
extern uint8_t menuopen;
extern uint32_t flash_address_index;
extern uint8_t currentview;
extern const uint32_t numClicks;
extern int gpsDelayCounter;
extern char gpgllOnlyPls[];
extern char uartRXbufferIT[GNSS_UART_BUF_SIZE];
extern char aboutToPrintMessage[];
extern char numCoordsMessage[];
extern char numCoordsChar[];
extern char newline[];
extern volatile int buttonLeftFlag;
extern volatile int buttonRightFlag;
extern volatile int gnssUartDoneFlag;
extern volatile int wheelCounterFullFlag;
extern uint32_t sessionMilliseconds;
extern uint32_t sessionHours;
extern uint32_t sessionMinutes;
extern char sessionHoursChar[];
extern char sessionMinutesChar[];
extern char combinedTimeChar[];
extern volatile uint32_t last_button_interrupt_time;


//these are what should be generated within the main.c file from the CubeMX code generation
extern I2C_HandleTypeDef hi2c2;
extern LPTIM_HandleTypeDef hlptim1;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart2;
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;




