

void flash_select();
void flash_deselect();
void flash_wren();
int flash_readsr();
void flash_wait_for_ready();
void flash_write(uint32_t start_address, uint8_t writebuf[], int buf_len);
void flash_read(uint32_t start_address, uint8_t myflashreadbuf[], int buf_len);
void flash_set_index(uint32_t index_val);
uint32_t flash_readindex();

