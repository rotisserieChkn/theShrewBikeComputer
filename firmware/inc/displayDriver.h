/*
 * displayDriver.c
 *
 *  Created on: Jun 18, 2023
 *      Author: nicho
 */


extern void displaySignalCommand();
extern void displaySignalData();
extern void displayHardwareReset();
extern void displayIssueCommand(uint8_t command_num, uint8_t param_data[], int num_param);
extern void displayInit();
extern void displaySetSubArea(uint16_t lowRow, uint16_t highRow, uint16_t lowCol, uint16_t highCol);
extern void displayTotalClear();
extern void LCDbigtext(char *oledtext, int len, int invert);
extern void displaySmallTextLOC(char *oledtext, int len, uint16_t yinit, uint16_t xinit);
extern void decimalDot();
extern void LCDclrbigtext();
extern void displayWritePixelBuf(uint8_t pixelBuf[], int numPixels);
extern void displayChipSelect();
extern void displayChipDeSelect();
extern void displayDummyCommand();
