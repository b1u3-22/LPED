/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		22-01-2025 00:34:33
 * 	 Modified time: 	13-05-2025 17:11:11
 * 	 Description: 	    This file contains implementation for functions
 *                      for physical behaviour of the dice
 */

#include "phy_dice.h"

/**
 * @brief Stack used by the LED effects thread. It is reused instead of 
 * recreated each time
 */
K_THREAD_STACK_DEFINE(led_effects_thread_stack, CONFIG_LPED_LED_EFFECTS_STACK_SIZE);

void set_led_color_param(phy_dice_dev_t *dice_dev, const uint8_t *red, const uint8_t *green, const uint8_t *blue) {
    pwm_set_pulse_dt(&dice_dev->led_r, *red * dice_dev->led_r.period / __UINT8_MAX__);
    pwm_set_pulse_dt(&dice_dev->led_g, *green * dice_dev->led_g.period / __UINT8_MAX__);
    pwm_set_pulse_dt(&dice_dev->led_b, *blue * dice_dev->led_b.period / __UINT8_MAX__);
}

void set_led_color_index(phy_dice_dev_t *dice_dev, uint8_t color_index) {
    if (color_index >= COLOR_PALLETE_SIZE) return;

    set_led_color_param(
        dice_dev,
        &pallete[color_index]->red,
        &pallete[color_index]->green,
        &pallete[color_index]->blue
    );
}

void run_fade(phy_dice_dev_t *dice_dev, const color_t * const first_color, const color_t * const second_color, uint16_t fade_duration) {
    color_t c = *first_color;

    if (fade_duration < CONFIG_LPED_LED_FADE_STEP) {
        set_led_color_param(dice_dev, &second_color->red, &second_color->green, &second_color->blue);
        return;
    }

    // calculate steps
    uint16_t steps = fade_duration / CONFIG_LPED_LED_FADE_STEP;

    // calculate diffs
    int16_t red_diff = (second_color->red - c.red) / steps;
    int16_t green_diff = (second_color->green - c.green) / steps;
    int16_t blue_diff = (second_color->blue - c.blue) / steps;

    // rum the fade effect
    for (uint16_t step = 0; step < steps; step++) {
        set_led_color_param(dice_dev, &c.red, &c.green, &c.blue);
        c.red += red_diff;
        c.green += green_diff;
        c.blue += blue_diff;
        k_msleep(CONFIG_LPED_LED_FADE_STEP);
    } 

    // set led to the second color
    set_led_color_param(dice_dev, &second_color->red, &second_color->green, &second_color->blue);
}

void run_animation(void *animation_param, void *dice_dev_param, void *unused2) {
    animation_t *animation = (animation_t *) animation_param;
    phy_dice_dev_t *dice_dev = (phy_dice_dev_t *) dice_dev_param;

    if (animation->number_of_steps == 0) return;

    switch (animation->fade_type) {
        case fade_type_none: 
            set_led_color_index(dice_dev, animation->steps[0].color_index);
            k_msleep((animation->steps[0].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
            break;

        case fade_type_fast: 
            run_fade(dice_dev, &COLOR_OFF, pallete[animation->steps[0].color_index], CONFIG_LPED_LED_FADE_TYPE_FAST_DURATION);
            k_msleep((animation->steps[0].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
            break;

        case fade_type_slow: 
            run_fade(dice_dev, &COLOR_OFF, pallete[animation->steps[0].color_index], CONFIG_LPED_LED_FADE_TYPE_SLOW_DURATION);
            k_msleep((animation->steps[0].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
            break;

        case fade_type_grad: 
            run_fade(dice_dev, &COLOR_OFF, pallete[animation->steps[0].color_index], (animation->steps[0].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION / 2);
            break;

        default: 
            return; // unknown fade type, exit
    }

    // fades/blinking between individual colors
    for (uint8_t step = 0; step < animation->number_of_steps - 1; step++) {
        switch (animation->fade_type) {
            case fade_type_fast: 
                run_fade(dice_dev, 
                    pallete[animation->steps[step].color_index], 
                    pallete[animation->steps[step + 1].color_index],
                    CONFIG_LPED_LED_FADE_TYPE_FAST_DURATION
                );
                k_msleep((animation->steps[step + 1].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
                break;

            case fade_type_slow: 
                run_fade(dice_dev, 
                    pallete[animation->steps[step].color_index], 
                    pallete[animation->steps[step + 1].color_index],
                    CONFIG_LPED_LED_FADE_TYPE_SLOW_DURATION
                );
                k_msleep((animation->steps[step + 1].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
                break;

            case fade_type_grad: 
                run_fade(dice_dev, 
                    pallete[animation->steps[step].color_index], 
                    pallete[animation->steps[step + 1].color_index],
                    ((animation->steps[0].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION + (animation->steps[step + 1].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION) / 2
                );
                break;

            case fade_type_none: 
            default: 
                set_led_color_index(dice_dev, animation->steps[step + 1].color_index);
                k_msleep((animation->steps[step + 1].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION);
                break;
        }
    }

    // implicit led_off at the end
    switch (animation->fade_type) {
        case fade_type_fast: 
            run_fade(dice_dev, pallete[animation->steps[animation->number_of_steps - 1].color_index], &COLOR_OFF, CONFIG_LPED_LED_FADE_TYPE_FAST_DURATION);
            break;

        case fade_type_slow: 
            run_fade(dice_dev, pallete[animation->steps[animation->number_of_steps - 1].color_index], &COLOR_OFF, CONFIG_LPED_LED_FADE_TYPE_SLOW_DURATION);
            break;

        case fade_type_grad: 
            run_fade(dice_dev, pallete[animation->steps[animation->number_of_steps - 1].color_index], &COLOR_OFF, (animation->steps[animation->number_of_steps - 1].duration + 1) * CONFIG_LPED_LED_DURATION_RESOLUTION / 2);
            break;

        default: 
            break;
    }

    set_led_color_index(dice_dev, COLOR_OFF_INDEX);
}

void dice_phy_init(phy_dice_dev_t *dice_dev) {
    static const struct pwm_dt_spec led_r = PWM_DT_SPEC_GET(DT_NODELABEL(diceled_red));
    dice_dev->led_r = led_r;

    static const struct pwm_dt_spec led_g = PWM_DT_SPEC_GET(DT_NODELABEL(diceled_green));
    dice_dev->led_g = led_g;

    static const struct pwm_dt_spec led_b = PWM_DT_SPEC_GET(DT_NODELABEL(diceled_blue));
    dice_dev->led_b = led_b;

    static const struct adc_dt_spec cap = ADC_DT_SPEC_GET(DT_NODELABEL(inputs));
    dice_dev->cap = cap;

    if (!adc_is_ready_dt(&dice_dev->cap)) printf("Dice CAP is not ready\n");
    if (adc_channel_setup_dt(&dice_dev->cap)) printf("Dice CAP failed to setup\n");
}

void dice_led_off(phy_dice_dev_t *dice_dev) {
    k_thread_abort(&dice_dev->led_effects_thread);
    set_led_color_index(dice_dev, COLOR_OFF_INDEX);
}

void dice_led_on_color(phy_dice_dev_t *dice_dev, const color_t *color) {
    k_thread_abort(&dice_dev->led_effects_thread);
    set_led_color_param(dice_dev, &color->red, &color->green, &color->blue);
}

void dice_led_on_params(phy_dice_dev_t *dice_dev, uint8_t *red, uint8_t *green, uint8_t *blue) {
    k_thread_abort(&dice_dev->led_effects_thread);
    set_led_color_param(dice_dev, red, green, blue);
}

void dice_led_on_index(phy_dice_dev_t *dice_dev, const uint8_t *index) {
    k_thread_abort(&dice_dev->led_effects_thread);
    set_led_color_index(dice_dev, *index);
}

void dice_led_start_animation(phy_dice_dev_t *dice_dev, const animation_t *animation) {
    dice_led_off(dice_dev);
    k_thread_create(
        &dice_dev->led_effects_thread,
        led_effects_thread_stack,
        K_THREAD_STACK_SIZEOF(led_effects_thread_stack),
        run_animation,
        (animation_t *) animation, dice_dev, NULL,      // WARNING: casting away const, the run_animation function must never modify the animation
        /* priority*/ 5,
        /* options */ 0,
        /* delay */   K_NO_WAIT  
    );
}

void dice_led_start_error_blink_animation(phy_dice_dev_t *dice_dev) {
    dice_led_start_animation(dice_dev, &animation_error_blink);
}

void dice_led_start_error_solid_animation(phy_dice_dev_t *dice_dev) {
    dice_led_start_animation(dice_dev, &animation_error_solid);
}

void dice_led_start_connected_animation(phy_dice_dev_t *dice_dev) {
    dice_led_start_animation(dice_dev, &animation_bluetooth_blink);
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
