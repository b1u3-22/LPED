/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:33:09
 * 	 Description: 	    This file contains definitions for dock related functions
 */

#include "dock.h"

void dock_led_timer_finished(struct k_timer *timer_id) {
    dock_dev_t *dock = CONTAINER_OF(timer_id, dock_dev_t, led_timer);
    gpio_pin_toggle_dt(&dock->led);
}

void dock_led_connection_timer_finished(struct k_timer *timer_id) {
    dock_dev_t *dock = CONTAINER_OF(timer_id, dock_dev_t, led_connection_timer);
    dock_led_stop_blink(dock);
}

void button_long_press(struct k_timer *timer_id) {
    dock_dev_t *dock = CONTAINER_OF(timer_id, dock_dev_t, button_long_press_timer);
    k_work_submit(dock->button_long_press_callback);
}

void dock_init(dock_dev_t *dock, k_timeout_t button_long_press_duration, struct k_work *button_long_callback) {
    k_timer_init(&dock->button_long_press_timer, button_long_press, NULL);
    dock->button_long_press_duration = button_long_press_duration;

    static const struct gpio_dt_spec led = DOCK_LED;
    dock->led = led;
    dock->button_long_press_callback = button_long_callback;

    if (!gpio_is_ready_dt(&dock->led)) printf("Dock LED device not read\n");
    if (gpio_pin_configure_dt(&dock->led, GPIO_OUTPUT_LOW)) printf("Dock LED setup failed\n");

    k_timer_init(&dock->led_timer, dock_led_timer_finished, NULL);
    k_timer_init(&dock->led_connection_timer, dock_led_connection_timer_finished, NULL);
}

void dock_button_int_init(struct gpio_callback *callback_gpio, gpio_callback_handler_t callback) {
    static const struct gpio_dt_spec int_pin = DOCK_BUTTON_INT_PIN;
    

    if (!gpio_is_ready_dt(&int_pin)) {
        printf("Interrupt pin DOCK_BUTTON is not ready\n");
        return;
    }

    if (gpio_pin_configure_dt(&int_pin, GPIO_INPUT)) {
        printf("Failed to configure DOCK_BUTTON pin\n");
        return;
    }

    if (gpio_pin_interrupt_configure_dt(&int_pin, GPIO_INT_EDGE_BOTH)) {
        printf("Failed to set interrupt type on pin DOCK_BUTTON\n");
        return;
    }

    gpio_init_callback(callback_gpio, callback, BIT(int_pin.pin));
    if (gpio_add_callback(int_pin.port, callback_gpio)) {
        printf("Could not add callback to pin DOCK_BUTTON\n");
        return;
    }

    printf("DOCK_BUTTON interrupt inizialized on pin: %d\n", int_pin.pin);
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

void dock_start_button_long_press_timer(dock_dev_t *dock) {
    
    // Not needed, since starting already started timer is permitted
    //// Check if timer is already running
    // if (k_timer_status_get(&dock->pair_timer) < 0) {
    //     // if it is, stop it first
    //     dock_stop_pair_timer(dock);
    // }


    k_timer_start(&dock->button_long_press_timer, dock->button_long_press_duration, K_NO_WAIT);
}

void dock_stop_button_long_press_timer(dock_dev_t *dock) {
    // If timer is already stop, don't do anything
    // if (k_timer_status_get(&dock->pair_timer) >= 0) return;
    k_timer_stop(&dock->button_long_press_timer);
}

uint8_t dock_get_button_state() {
    static const struct gpio_dt_spec dock_button_pin = DOCK_BUTTON_INT_PIN;
    return gpio_pin_get_dt(&dock_button_pin);
}

uint8_t dock_get_conn_state() {
    static const struct gpio_dt_spec dock_button_pin = DOCK_CONN_INT_PIN;
    return gpio_pin_get_dt(&dock_button_pin);
}

void dock_led_start_blink(dock_dev_t *dock, led_speed_t speed)
{
    k_timer_start(&dock->led_timer, K_MSEC(speed), K_MSEC(speed));
}

void dock_led_stop_blink(dock_dev_t *dock) {
    k_timer_stop(&dock->led_timer);
    dock_led_off(dock);
}

void dock_led_start_connection(dock_dev_t *dock_dev) {
    dock_led_start_blink(dock_dev, led_speed_fast);
    k_timer_start(&dock_dev->led_connection_timer, K_MSEC(led_speed_fast * 2 *2), K_NO_WAIT);
}

void dock_led_stop_connection(dock_dev_t *dock_dev) {
    k_timer_stop(&dock_dev->led_connection_timer);
    dock_led_stop_blink(dock_dev);
}

void dock_led_off(dock_dev_t *dock) {
    gpio_pin_set_dt(&dock->led, 0);
}

void dock_led_on(dock_dev_t *dock) {
    gpio_pin_set_dt(&dock->led, 1);
}