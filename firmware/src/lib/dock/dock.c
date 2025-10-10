/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:33:09
 * 	 Description: 	    This file contains definitions for dock related functions
 */

#include "dock.h"
void dock_init(dock_dev_t *dock) {
    // reserved for future use
}

void dock_conn_int_init(struct gpio_callback *callback_gpio, gpio_callback_handler_t callback) {
    static const struct gpio_dt_spec int_pin = DOCK_CONN_INT_PIN;
    

    if (!gpio_is_ready_dt(&int_pin)) {
        printf("Interrupt pin DOCK_CONN is not ready\n");
        return;
    }

    if (gpio_pin_configure_dt(&int_pin, GPIO_INPUT)) {
        printf("Failed to configure DOCK_CONN pin\n");
        return;
    }

    if (gpio_pin_interrupt_configure_dt(&int_pin, GPIO_INT_EDGE_BOTH)) {
        printf("Failed to set interrupt type on pin DOCK_CONN\n");
        return;
    }

    gpio_init_callback(callback_gpio, callback, BIT(int_pin.pin));
    if (gpio_add_callback(int_pin.port, callback_gpio)) {
        printf("Could not add callback to pin DOCK_CONN\n");
        return;
    }

    printf("DOCK_CONN interrupt inizialized on pin: %d\n", int_pin.pin);
}

uint8_t dock_get_conn_state() {
    static const struct gpio_dt_spec dock_button_pin = DOCK_CONN_INT_PIN;
    return gpio_pin_get_dt(&dock_button_pin);
}