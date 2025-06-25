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
#include "../led_defs.h"

#define ERROR_TIME K_MSEC(2000)

/**
 * @brief Pin where dice LED is connected
 */
#define DICE_LED    GPIO_DT_SPEC_GET(DT_NODELABEL(diceled), gpios)

/**
 * @brief Pin where capacitor is connected
 */
#define DICE_CAP    ADC_DT_SPEC_GET(DT_NODELABEL(inputs))

/**
 * @brief Structure representing the physical dice
 */
typedef struct phy_dice_dev {
    struct k_timer led_blink_timer; // Timer used for LED blinking
    struct k_timer led_error_timer; // Timer used for ERROR blink
    struct k_timer led_side_timer;  // Timer used for normal side blinking
    struct gpio_dt_spec led;        // Pin where the dice LED is connected
    struct adc_dt_spec cap;         // Pin used for capacitor state reading
} phy_dice_dev_t;

/**
 * @brief   Initialize the physical dice: init timers, led and cap pins
 */
void dice_phy_init(phy_dice_dev_t *dice_dev);

/**
 * @brief Start blinking the dice LED with given speed
 * @param dice_dev  Dice
 * @param speed     How fast the LED should blink
 */
void dice_led_start_blink(phy_dice_dev_t *dice_dev, led_speed_t speed);

/**
 * @brief   Immediately stop the dice LED blinking, if no blinking is running, this function has no effect
 *          The LED will be turned OFF
 * @param dice_dev  Dice
 */
void dice_led_stop_blink(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED off, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_off(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED on, note this will not stop ongoing blinking
 * @param dice_dev Dice
 */
void dice_led_on(phy_dice_dev_t *dice_dev);

/**
 * @brief Start error blinking the dice LED
 * @param dice_dev Dice
 */
void dice_led_start_error_blink(phy_dice_dev_t *dice_dev);

/**
 * @brief Turn the dice LED on for #ERROR_TIME then turn it off
 * @param dice_def Dice
 */
void dice_led_start_error_solid(phy_dice_dev_t *dice_dev);

/**
 * @brief   Stop ongoing error blinking, if no blinking is running, this function has no effect
 *          The LED will be turned OFF
 * @param dice_dev 
 */
void dice_led_stop_error(phy_dice_dev_t *dice_dev);

/**
 * @brief Start blinking the dice LED based on the given side number and led mode
 * @param dice_dev          Dice
 * @param side_number       Number on the side
 * @param side_blink_mode   Sides blink mode
 */
void dice_led_start_side(phy_dice_dev_t *dice_dev, uint8_t *side_number, uint8_t side_blink_mode);

/**
 * @brief   Stop ongoing side blinking, if no blinking is running, this function has no effect
 *          The LED will be turned OFF
 * @param dice_dev 
 */
void dice_led_stop_side(phy_dice_dev_t *dice_dev);

/**
 * @brief   Get current capacitor state of charge as unsigned eight bit integer,
 *          255 represents voltage 3.6V
 * @param   dice_dev    Dice
 * @param   cap_state   Where to save the read cap state
 */
void dice_get_cap_state(phy_dice_dev_t *dice_dev, uint8_t *cap_state);

#endif // LIB_DICE_DICE_H_