/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:11:11
 * 	 Description: 	    This file contains implementation for functions
 *                      for physical behaviour of the dice
 */

#include "phy_dice.h"

void dice_led_timer_finished(struct k_timer *timer_id) {
    phy_dice_dev_t *dice = CONTAINER_OF(timer_id, phy_dice_dev_t, led_blink_timer);
    gpio_pin_toggle_dt(&dice->led);
}

void dice_error_timer_finished(struct k_timer *timer_id) {
    phy_dice_dev_t *dice = CONTAINER_OF(timer_id, phy_dice_dev_t, led_error_timer);
    dice_led_stop_blink(dice);
}

void dice_side_timer_finished(struct k_timer *timer_id) {
    phy_dice_dev_t *dice = CONTAINER_OF(timer_id, phy_dice_dev_t, led_side_timer);
    dice_led_stop_blink(dice);
}

void dice_phy_init(phy_dice_dev_t *dice_dev) {
    k_timer_init(&dice_dev->led_blink_timer, dice_led_timer_finished, NULL);
    k_timer_init(&dice_dev->led_error_timer, dice_error_timer_finished, NULL);
    k_timer_init(&dice_dev->led_side_timer, dice_side_timer_finished, NULL);

    static const struct gpio_dt_spec led = DICE_LED;
    dice_dev->led = led;

    if (!gpio_is_ready_dt(&dice_dev->led)) printf("Dice LED is not ready\n");
    if (gpio_pin_configure_dt(&dice_dev->led, GPIO_OUTPUT_LOW)) printf("Dice LED failed to setup\n");

    static const struct adc_dt_spec cap = DICE_CAP;
    dice_dev->cap = cap;

    if (!adc_is_ready_dt(&dice_dev->cap)) printf("Dice CAP is not ready\n");
    if (adc_channel_setup_dt(&dice_dev->cap)) printf("Dice CAP failed to setup\n");
}

void dice_led_start_blink(phy_dice_dev_t *dice_dev, led_speed_t speed) {
    k_timer_start(&dice_dev->led_blink_timer, K_MSEC(speed), K_MSEC(speed));
}

void dice_led_stop_blink(phy_dice_dev_t *dice_dev) {
    k_timer_stop(&dice_dev->led_blink_timer);
    dice_led_off(dice_dev);
}

void dice_led_off(phy_dice_dev_t *dice_dev) {
    gpio_pin_set_dt(&dice_dev->led, 0);
}

void dice_led_on(phy_dice_dev_t *dice_dev) {
    gpio_pin_set_dt(&dice_dev->led, 1);
}

void dice_led_start_error_blink(phy_dice_dev_t *dice_dev) {
    dice_led_start_blink(dice_dev, led_speed_normal);
    k_timer_start(&dice_dev->led_error_timer, ERROR_TIME, K_NO_WAIT);
}

void dice_led_start_error_solid(phy_dice_dev_t *dice_dev) {
    dice_led_on(dice_dev);
    k_timer_start(&dice_dev->led_error_timer, ERROR_TIME, K_NO_WAIT);
}

void dice_led_stop_error(phy_dice_dev_t *dice_dev) {
    k_timer_stop(&dice_dev->led_error_timer);
    dice_led_off(dice_dev);
}

void dice_led_start_side(phy_dice_dev_t *dice_dev, uint8_t *side_number, uint8_t side_blink_mode) {
    if (side_blink_mode == led_speed_none || side_blink_mode > LED_SPEED_MAP_LEN) return;

    dice_led_start_blink(dice_dev, led_speed_map[side_blink_mode]);
    k_timer_start(&dice_dev->led_side_timer, K_MSEC(led_speed_map[side_blink_mode] * 2 * *side_number), K_NO_WAIT);
}

void dice_led_stop_side(phy_dice_dev_t *dice_dev) {
    k_timer_stop(&dice_dev->led_side_timer);
    dice_led_stop_blink(dice_dev);
}

void dice_get_cap_state(phy_dice_dev_t *dice_dev, uint8_t *cap_state) {
    uint16_t buffer;
    struct adc_sequence sequence = {
        .buffer = &buffer,
        .buffer_size = sizeof(buffer)
    };

    if (adc_sequence_init_dt(&dice_dev->cap, &sequence)) {
        printf("Failed to init read sequence\n");
        *cap_state = 0;
    }

    if (adc_read_dt(&dice_dev->cap, &sequence)) {
        printf("Failed to read from Dice CAP\n");
        *cap_state = 0;
    }

    *cap_state = ((uint8_t) (buffer >> (sequence.resolution - 8)));
}
