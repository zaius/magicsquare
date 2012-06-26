// TODO: separate addressing and content of messages
// TODO: have a message type identifier
// TODO: separate color messages and button press messages
typedef struct message MESSAGE;
struct message {
  uint8_t source;
  uint8_t destination;
  uint8_t square_index;
  uint8_t color;
};

void message_encode(MESSAGE* msg);
void message_decode(uint8_t* data, uint8_t data_length);
