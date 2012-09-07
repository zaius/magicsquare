typedef struct color COLOR;
struct color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

void led_init();
void set_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
void led_timer();
void cycle_rgb(uint8_t switch_index);

extern COLOR colors[4];
