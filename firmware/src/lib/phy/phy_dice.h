/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:10:40
 * 	 Description: 	    This file contains declarations for functions 
 *                      that can be used to control the physical behaviour
 *                      of the dice (such as LED blinking), and definition
 *                      for necessary structure
 */

#ifndef LIB_DICE_DICE_H_
#define LIB_DICE_DICE_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/pwm.h>
#include "../color/animation.h"
#include "../color/animation_defs.h"

/**
 * @brief Structure representing the physical dice
 */
typedef struct phy_dice_dev {
    struct pwm_dt_spec led_r;               // Pin where the dice red LED is connected
    struct pwm_dt_spec led_g;               // Pin where the dice green LED is connected
    struct pwm_dt_spec led_b;               // Pin where the dice blue LED is connected
    struct adc_dt_spec cap;                 // Pin used for capacitor state reading

    struct k_thread led_effects_thread;     // Thread that is running all LED effects
} phy_dice_dev_t;

/**
 * @brief   Initialize the physical dice: init timers, led and cap pins
 */
void dice_phy_init(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED off, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_off(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED on, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_on_color(phy_dice_dev_t *dice_dev, const color_t *color);

/**
 * @brief Turn the dice LED on, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_on_params(phy_dice_dev_t *dice_dev, uint8_t *red, uint8_t *green, uint8_t *blue);

/**
 * @brief Turn the dice LED on, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_on_index(phy_dice_dev_t *dice_dev, const uint8_t *index);

/**
 * @brief Run given animation immediately. Note that this will stop ongoing animations
 * @param   dice_dev Dice
 * @param   animation animation that should be run         
 */
void dice_led_start_animation(phy_dice_dev_t *dice_dev, const animation_t *animation);

/**
 * @brief Start error blinking the dice LED
 * @param dice_dev Dice
 */
void dice_led_start_error_blink_animation(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED on for #ERROR_TIME then turn it off
 * @param dice_def Dice
 */
void dice_led_start_error_solid_animation(phy_dice_dev_t *dice_dev);

void dice_led_start_connected_animation(phy_dice_dev_t *dice_dev);

/**
 * @brief   Get current capacitor state of charge as unsigned eight bit integer,
 *          255 represents voltage 3.6V
 * @param   dice_dev    Dice
 * @param   cap_state   Where to save the read cap state
 */
void dice_get_cap_state(phy_dice_dev_t *dice_dev, uint8_t *cap_state);

#endif // LIB_DICE_DICE_H_