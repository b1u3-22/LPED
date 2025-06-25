/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:32:44
 * 	 Description: 	    This file contains definition of dock structure and
 *                      declarations for dock related functions
 */

#ifndef LIB_DOCK_DOCK_H_
#define LIB_DOCK_DOCK_H_

#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include "../led_defs.h"

/**
 * @brief Pin where the button is connected to when placed in dock
 */
#define DOCK_BUTTON_INT_PIN GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(dockbutton), gpios, 0)

/**
 * @brief Connection pin that is used to determine if dice is in dock or not
 */
#define DOCK_CONN_INT_PIN   GPIO_DT_SPEC_GET_BY_IDX(DT_NODELABEL(inputs), dock_conn_gpios, 0)

/**
 * Pin where the dock LED is connected to when placed in dock
 */
#define DOCK_LED            GPIO_DT_SPEC_GET(DT_NODELABEL(dockled), gpios)


/**
 * @brief Structure representing the dock
 */
typedef struct dock_dev {
    struct k_timer button_long_press_timer;     // Timer used for button long press
    k_timeout_t button_long_press_duration;     // How long does button stay pressed to qualify as long-press
    struct k_timer led_timer;                   // Timer for LED blinking
    struct k_timer led_connection_timer;        // Timer for connection LED blinking
    struct gpio_dt_spec led;                    // Pin where dock LED is connected
    struct k_work *button_long_press_callback;  // Callback that is executed after a button long-press
} dock_dev_t;

/**
 * @brief Initialize the dock structure
 * @param dock_dev                      dock struct to initialize
 * @param button_long_press_duration    duration required for button long press
 * @param button_long_callback          callback executed after long press registered
 */
void dock_init(dock_dev_t *dock_dev, k_timeout_t button_long_press_duration, struct k_work *button_long_callback);

/**
 * @brief Initialize the button pin as interrupt for both RISING and FALLING and register execution callback
 */
void dock_button_int_init(struct gpio_callback *callback_gpio, gpio_callback_handler_t callback);

/**
 * @brief Initialize the connection pin as interrupt for both RISING and FALLING and register execution callback
 */
void dock_conn_int_init(struct gpio_callback *callback_gpio, gpio_callback_handler_t callback);

/**
 * @brief   This function should be ran when the button has been pressed to start the long press timer.
 *          If the timer is already running, this function will reset it
 * @param dock_dev Dock
 */
void dock_start_button_long_press_timer(dock_dev_t *dock_dev);

/**
 * @brief   This function should be ran when the button has been released to stop the press timer. 
 *          If timer is already stopped or finished this function has no effect.
 */
void dock_stop_button_long_press_timer(dock_dev_t *dock_dev);

/**
 * @brief Get current state of button pin
 * @return 0x01 if high, otherwise 0x00
 */
uint8_t dock_get_button_state();

/**
 * @brief Get current state of connection pin
 * @return 0x01 if high, otherwise 0x00
 */
uint8_t dock_get_conn_state();

/**
 * @brief Start the dock LED blinking with given speed
 * @param dock_dev  Dock
 * @param speed     Speed the LED should be blinking
 */
void dock_led_start_blink(dock_dev_t *dock_dev, led_speed_t speed);

/**
 * @brief Immediately stop the dock LED blinking. LED will be turned OFF
 * @param dock_def Dock
 */
void dock_led_stop_blink(dock_dev_t *dock_dev);

/**
 * @brief Start dock LED connection blinking, if already running, it will be reset
 * @param dock_def Dock
 */
void dock_led_start_connection(dock_dev_t *dock_dev);

/**
 * @brief Immediately stop the dock connection blinking. LED will be turned OFF
 * @param dock_def Dock
 */
void dock_led_stop_connection(dock_dev_t *dock_dev);

/**
 * @brief Turn the dock LED off, this will not stop LED blinking
 * @param dock_dev Dock
 */
void dock_led_off(dock_dev_t *dock_dev);

/**
 * @brief Turn the dock LED on, this will not alter LED blinking
 * @param dock_dev Dock
 */
void dock_led_on(dock_dev_t *dock_dev);

#endif // LIB_DOCK_DOCK_H_