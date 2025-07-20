/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 16:46:13
 * 	 Description: 	    This file contains implementation for function for determining sides
 */

#include "side.h"

void determine_side(dice_definition_t *dice_def, int16_t accel_x, int16_t accel_y, int16_t accel_z, side_definition_t *side_def) {
    for (int side = 0; side < dice_def->header.number_of_sides; side++) {

        // Compare vector values
        if (
            (dice_def->sides[side].vector[VEC_X] - dice_def->header.range <= accel_x && dice_def->sides[side].vector[VEC_X] + dice_def->header.range >= accel_x) && 
            (dice_def->sides[side].vector[VEC_Y] - dice_def->header.range <= accel_y && dice_def->sides[side].vector[VEC_Y] + dice_def->header.range >= accel_y) && 
            (dice_def->sides[side].vector[VEC_Z] - dice_def->header.range <= accel_z && dice_def->sides[side].vector[VEC_Z] + dice_def->header.range >= accel_z)
        ) {
            *side_def = dice_def->sides[side];
            return;
        }   
    }
    
    EMPTY_SIDE_DEF(side_def);
}
