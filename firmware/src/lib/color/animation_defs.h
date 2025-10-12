#ifndef LIB_COLOR_ANIMATION_DEFS_H_
#define LIB_COLOR_ANIMATION_DEFS_H_

#include "animation.h"
#include "color_defs.h"

static const animation_t animation_error_blink = {
    .number_of_steps = 9, 
    .fade_type = fade_type_fast,
    .steps = {
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_OFF_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_OFF_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_OFF_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_OFF_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 2
        }
    }
};

static const animation_t animation_error_solid = {
    .number_of_steps = 2,
    .fade_type = fade_type_slow, 
    .steps = {
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 15
        },
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 15
        }
    }
};

static const animation_t animation_bluetooth_blink = {
    .number_of_steps = 3,
    .fade_type = fade_type_fast, 
    .steps = {
        {
            .color_index = COLOR_BLUE_INDEX,
            .duration = 2
        },
        {
            .color_index = COLOR_OFF_INDEX,
            .duration = 1
        },
        {
            .color_index = COLOR_BLUE_INDEX,
            .duration = 2
        }
    }
};

static const animation_t animation_debug_startup = {
    .number_of_steps = 3,
    .fade_type = fade_type_grad,
    .steps = {
        {
            .color_index = COLOR_RED_INDEX,
            .duration = 9
        },
        {
            .color_index = COLOR_GREEN_INDEX,
            .duration = 9
        },
        {
            .color_index = COLOR_BLUE_INDEX,
            .duration = 9
        }
    }
};

#endif // LIB_COLOR_ANIMATION_DEFS_H_