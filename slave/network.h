extern uint16_t hardware_address;
extern uint8_t group_index;

void network_init(void);
void network_send(uint8_t* new_data, uint8_t data_length);
