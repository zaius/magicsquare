extern uint16_t hardware_address;

void network_init(void);
void network_send(uint8_t* new_data, uint8_t data_length);

// Circular buffer for storing packet
typedef struct buffer BUFFER;
struct buffer {
  uint8_t data[MAX_PACKET_SIZE];
  uint8_t read_index;
  uint8_t length;
};
