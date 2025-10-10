/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		26-03-2025 23:10:18
 * 	 Modified time: 	13-05-2025 16:55:31
 * 	 Description: 	    This file contains definition for dice, headers and sides
 */

#ifndef LIB_SIDE_SIDE_DEFS_H_
#define LIB_SIDE_SIDE_DEFS_H_

#include "../color/animation.h"
#include "../color/color_defs.h"

#define ACC_1G      1024    /** Value of acceleration equal to 1g */
#define VEC_X       0       /** Position of the x axis vector component in side_def vector array*/
#define VEC_Y       1       /** Position of the y axis vector component in side_def vector array */
#define VEC_Z       2       /** Position of the z axis vector component in side_def vector array */

#define DICE_DEF_NAME_MAX_LEN 21
#define DICE_DEF_SIDES_MAX 60

#define NUMBER_OF_PREDEFINED_DICE sizeof(supported_dice) / sizeof(dice_definition_t *)
#define EMPTY_SIDE_DEF(side_def) (memset(side_def, 0, sizeof(side_definition_t)))
#define EMPTY_DICE_DEF(dice_def) (memset(dice_def, 0, sizeof(dice_definition_t)))
#define EMPTY_DICE_HEAD(dice_head) (memset(dice_head, 0, sizeof(dice_definition_header_t)))

/**
 * @brief This structure represents one side of the dice
 */
typedef struct side_definition {
    uint8_t number;                     // Number on this side
    animation_t animation;              // animation for this side
    int16_t vector[3];                  // Acceleration vector
} side_definition_t;

/**
 * @brief   Header for dice definition. Dice definition represents the
 *          whole dice (at least the currently used configuration). 
 *          Header contains the general dice information
 */
typedef struct dice_definition_header {
    char name[DICE_DEF_NAME_MAX_LEN];   // Short human-readable name of the definition
    uint8_t id;                         // ID of the definition
    uint8_t number_of_sides;            // Number of sides in this definition
    uint8_t _padding[1];    
    uint16_t range;                     // Range that is used when determining landed side
} dice_definition_header_t;

/**
 * @brief   Full dice definition with a header and array of sides.
 *          Dice definition represent the currently used dice
 */
typedef struct dice_definition {
    dice_definition_header_t header;                // Header with general information
    side_definition_t sides[DICE_DEF_SIDES_MAX];    // Array of sides this definition has
} dice_definition_t;

/**
 * @brief Predefined six-sided dice definition
 */
static const dice_definition_t dice_6_side = {
    .header = {
        .name = "6-sided default\0",
        .id = 1,
        .number_of_sides = 6,
        .range = 250
    },
    .sides = {
        {
            .number = 1,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 1, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
                        .duration = 2
                    }
                }
            },
            .vector = {0, ACC_1G, 0}
        },
        {
            .number = 2,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 3, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
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
            },
            .vector = {0, 0, ACC_1G}
        },
        {
            .number = 3,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 5, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
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
                        .color_index = COLOR_GREEN_INDEX,
                        .duration = 2
                    }
                }
            },
            .vector = {ACC_1G, 0, 0}
        },
        {
            .number = 4,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 7, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
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
                        .color_index = COLOR_BLUE_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_OFF_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_GREEN_INDEX,
                        .duration = 2
                    }
                }
            },
            .vector = {-ACC_1G, 0, 0}
        },
        {
            .number = 5,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 9, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
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
                        .color_index = COLOR_BLUE_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_OFF_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_GREEN_INDEX,
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
            },
            .vector = {0, 0, -ACC_1G}
        },
        {
            .number = 6,
            .animation = {
                .fade_type = fade_type_slow,
                .number_of_steps = 7, 
                .steps = {
                    {
                        .color_index = COLOR_BLUE_INDEX,
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
                        .color_index = COLOR_BLUE_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_OFF_INDEX,
                        .duration = 2
                    },
                    {
                        .color_index = COLOR_GREEN_INDEX,
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
                        .color_index = COLOR_GREEN_INDEX,
                        .duration = 2
                    }
                }
            },
            .vector = {0, -ACC_1G, 0}
        },
    }
};

/**
 * @brief Array of supported predefined dice definitions
 */
static const dice_definition_t *supported_dice[] __attribute__((unused)) = {
    &dice_6_side
};

#endif // LIB_SIDE_SIDE_DEFS_H_