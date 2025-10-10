#ifndef ANIMATION_H_
#define ANIMATION_H_

typedef enum fade_type {
    fade_type_none = 0x00,
    fade_type_fast = 0x01,
    fade_type_slow = 0x02,
    fade_type_grad = 0x03
} fade_type_t;

typedef struct animation_step {
    uint8_t color_index      : 4;    /* Index of the color in the color pallete*/
    uint8_t duration         : 4;    /* Color duration as multiples of configured color duration step */
} animation_step_t;

typedef struct animation {
    uint8_t fade_type       : 2;
    uint8_t number_of_steps : 6;
    animation_step_t steps[20];
} animation_t;

#endif // ANIMATION_H_