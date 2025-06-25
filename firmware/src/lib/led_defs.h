#ifndef LIB_LED_DEFS_H_
#define LIB_LED_DEFS_H_

#define LED_SPEED_MAP_LEN sizeof(led_speed_map) / sizeof(led_speed_map[0])

typedef enum {
    led_speed_none      = 0,
    led_speed_slow      = 500,
    led_speed_normal    = 250,
    led_speed_fast      = 100
} led_speed_t;

static const led_speed_t led_speed_map[] = {
    led_speed_none,
    led_speed_slow,
    led_speed_normal,
    led_speed_fast
};

#endif // LIB_LED_DEFS_H_