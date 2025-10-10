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
    // reserved for future use
} dock_dev_t;

/**
 * @brief Initialize the dock structure
 * @param dock_dev                      dock struct to initialize
 * @param button_long_press_duration    duration required for button long press
 * @param button_long_callback          callback executed after long press registered
 */
void dock_init(dock_dev_t *dock_dev);

/**
 * @brief Initialize the connection pin as interrupt for both RISING and FALLING and register execution callback
 */
void dock_conn_int_init(struct gpio_callback *callback_gpio, gpio_callback_handler_t callback);

/**
 * @brief Get current state of connection pin
 * @return 0x01 if high, otherwise 0x00
 */
uint8_t dock_get_conn_state();


#endif // LIB_DOCK_DOCK_H_