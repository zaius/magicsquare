#define MASTER_ADDRESS 0xaaaa
typedef struct message_header MESSAGE_HEADER;
struct message_header {
  uint16_t source;
  uint16_t destination;
  // uint16_t checksum;
  uint8_t message_type;
};

#define MESSAGE_TYPE_COLOR_CHANGE 1
typedef struct message_color_change MESSAGE_COLOR_CHANGE;
struct message_color_change {
  uint8_t switch_index;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

#define MESSAGE_TYPE_SWITCH_CHANGE 2
typedef struct message_switch_change MESSAGE_SWITCH_CHANGE;
struct message_switch_change {
  uint8_t switch_index;
  uint8_t switch_state;
};


void message_send(MESSAGE_SWITCH_CHANGE* switch_change);
void message_receive(uint8_t* data, uint8_t data_length);
