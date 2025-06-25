/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:41:46
 * 	 Description: 	    This file contains declaration for function that can be used to determine which side is facing up
 */

#ifndef LIB_SIDE_SIDE_H_
#define LIB_SIDE_SIDE_H_

#include <zephyr/kernel.h>
#include "side_defs.h"

/**
 * @brief   Determine which side is currently facing up based on given dice definition
 *          This function compares the given acceleration values with vector saved in each side
 *          and then also check if the acceleration is close to 1g
 * @param   dice_def    dice definition where to search for appropriate side
 * @param   accel_x     acceleration value for x axis
 * @param   accel_y     acceleration value for y axis
 * @param   accel_z     acceleration value for z axis
 * @param   side_def    where to save matching side or empty if no side matches the vector
 */
void determine_side(dice_definition_t *dice_def, int16_t accel_x, int16_t accel_y, int16_t accel_z, side_definition_t *side_def);

#endif // LIB_SIDE_SIDE_H_