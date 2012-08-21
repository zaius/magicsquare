// TODO: separate addressing and content of messages
typedef struct message MESSAGE;
struct message {
  uint16_t source;
  uint16_t destination;
  uint16_t checksum;
  uint8_t message_type;
  uint8_t data_length;
  uint8_t* data;
};

void message_reset_address(uint8_t* data, uint8_t data_length);
void message_assign_address(uint8_t* data, uint8_t data_length);
void message_calibration_mode(uint8_t* data, uint8_t data_length);
void message_set_color(uint8_t* data, uint8_t data_length);
void message_ignore(uint8_t* data, uint8_t data_length);

void message_send(MESSAGE* msg);
void message_receive(MESSAGE* msg);
void message_decode(uint8_t* data, uint8_t data_length);
