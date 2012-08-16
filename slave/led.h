typedef struct color COLOR;
struct color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

void led_init();
void led_timer();

extern COLOR colors[4];
