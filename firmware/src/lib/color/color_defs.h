#ifndef COLOR_DEFS_H_
#define COLOR_DEFS_H_

#include <zephyr/kernel.h>
#include "color.h"

#define COLOR_OFF_INDEX     0
#define COLOR_RED_INDEX     2
#define COLOR_GREEN_INDEX   10
#define COLOR_BLUE_INDEX    14

#define COLOR_PALLETE_SIZE  (sizeof(pallete) / sizeof(color_t *))

static const color_t COLOR_OFF                  = { 0x00, 0x00, 0x00 };
static const color_t COLOR_WHITE                = { 0xFF, 0xFF, 0xFF };
static const color_t COLOR_RED_BRIGHT           = { 0xFF, 0x00, 0x00 };
static const color_t COLOR_RED_DIM              = { 0x80, 0x00, 0x00 };
static const color_t COLOR_ORANGE_BRIGHT        = { 0xFF, 0x80, 0x00 };
static const color_t COLOR_ORANGE_DIM           = { 0x80, 0x40, 0x00 };
static const color_t COLOR_YELLOW_BRIGHT        = { 0xFF, 0xFF, 0x00 };
static const color_t COLOR_YELLOW_DIM           = { 0x80, 0x80, 0x00 };
static const color_t COLOR_CHARTREUSE_BRIGHT    = { 0x80, 0xFF, 0x00 };
static const color_t COLOR_CHARTREUSE_DIM       = { 0x40, 0x80, 0x00 };
static const color_t COLOR_GREEN_BRIGHT         = { 0x00, 0xFF, 0x00 };
static const color_t COLOR_GREEN_DIM            = { 0x00, 0x80, 0x00 };
static const color_t COLOR_CYAN_BRIGHT          = { 0x00, 0xFF, 0xFF };
static const color_t COLOR_CYAN_DIM             = { 0x00, 0x80, 0x80 };
static const color_t COLOR_BLUE_BRIGHT          = { 0x00, 0x00, 0xFF };
static const color_t COLOR_BLUE_DIM             = { 0x00, 0x00, 0x80 };

static const color_t * const pallete[16] = {
    &COLOR_OFF,
    &COLOR_WHITE,
    &COLOR_RED_BRIGHT,
    &COLOR_RED_DIM,
    &COLOR_ORANGE_BRIGHT,
    &COLOR_ORANGE_DIM,
    &COLOR_YELLOW_BRIGHT,
    &COLOR_YELLOW_DIM,
    &COLOR_CHARTREUSE_BRIGHT,
    &COLOR_CHARTREUSE_DIM,
    &COLOR_GREEN_BRIGHT,
    &COLOR_GREEN_DIM,
    &COLOR_CYAN_BRIGHT,
    &COLOR_CYAN_DIM,
    &COLOR_BLUE_BRIGHT,
    &COLOR_BLUE_DIM 
};


#endif // COLOR_DEFS_H_