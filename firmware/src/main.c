#include <zephyr/kernel.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/dfu/mcuboot.h>
#include <zephyr/sys/poweroff.h>

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
#include <libfxls89xx.h>
#endif

#include "lib/side/side.h"
#include "lib/bt/bt_dice.h"
#include "lib/phy/phy_dice.h"
#include "lib/dock/dock.h"
#include "lib/storage/storage.h"
#include "lib/side/side_defs.h"

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
static const struct device *acc = DEVICE_DT_GET_ANY(nxp_fxls89xx);

// helper function to configure accelerometer into interrupt state
void acc_configure_int() {
	fxls89xx_configure_sdcd(acc, fxls89xx_sdcd_reference_update_always, fxls89xx_sdcd_input_mode_standard, -100, 100);
	fxls89xx_configure_sdcd_within(acc, false, true, true, true, 1, fxls89xx_sdcd_counter_decrement, fxls89xx_sdcd_logic_mode_and);

	fxls89xx_set_auto_timeout(acc, 2);
	fxls89xx_enable_auto_source(acc, fxls89xx_aslp_source_sdcd_wt, true);
	fxls89xx_enable_sdcd(acc, true);
	fxls89xx_interrupt_enable(acc, fxls89xx_interrupt_source_wake_out, true);
}

static struct gpio_callback acc_int1_callback_gpio;
//static struct gpio_callback int2_callback_gpio;
#endif

static struct gpio_callback int_dock_conn_callback_gpio;

static struct dock_dev dock;
static struct phy_dice_dev phy_dice;
static struct bt_dice_dev bt_dice;

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
static uint8_t cap_state_interval_counter = 0;

// debounce timers for interrupts
static uint32_t acc_int1_debounce = 0;
static uint32_t acc_int2_debounce = 0;
#endif
static uint32_t dock_con_debounce = 0;

// worker threads invoked from ISRs
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
struct k_work acc_int1_work;
struct k_work acc_int2_work;
#endif
struct k_work dock_btn_work;
struct k_work dock_con_work;
struct k_work dice_visible_timeout_work;
struct k_work dice_connectable_timeout_work;
struct k_work dice_connected_timeout_work;
struct k_work dice_connected_work;
struct k_work dice_disconnected_work;

static uint8_t ignore_dock_disconnect = false;

static uint32_t last_boot_time;

static uint8_t comm_mode;

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
static side_definition_t side_def;
static dice_definition_t dice_def;
static uint8_t side_blink;
static uint8_t error_blink;
static uint8_t cap_state;

void acc_int1_callback(struct k_work *work) {
	storage_get_comm_mode(&comm_mode);

	if (fxls89xx_get_int_pin_state(acc, fxls89xx_interrupt_pin_int1)) {
		storage_get_current_dice_definition(&dice_def);

		determine_side(
			&dice_def,
			fxls89xx_get_acceleration(acc, fxls89xx_axis_x),
			fxls89xx_get_acceleration(acc, fxls89xx_axis_y),
			fxls89xx_get_acceleration(acc, fxls89xx_axis_z),
			&side_def
		);

		printk("Landed on number: %u\n", side_def.number);

		storage_get_side_blink(&side_blink);
		storage_get_error_blink(&error_blink);

		if (side_blink || error_blink) 	dice_led_off(&phy_dice);

		if (side_def.number != 0)   {
			if (side_blink) dice_led_start_animation(&phy_dice, &side_def.animation);

			if (comm_mode) 	dice_bt_set_dice_number(&bt_dice, &side_def.number, bt_message_dice_number);
			else 			dice_bt_broadcast(&side_def.number, bt_message_dice_number);
		}

		else {
			if (error_blink) dice_led_start_error_solid_animation(&phy_dice);

			if (comm_mode) 	dice_bt_set_dice_number(&bt_dice, NULL, bt_message_unknown);
			else 			dice_bt_broadcast(NULL, bt_message_unknown);
		}

		if (!IS_ENABLED(CONFIG_LPED_CAP_STATE_IN_ROLLING_MSG)) {
			dice_get_cap_state(&phy_dice, &cap_state);

#ifdef CONFIG_LPED_CAP_STATE_INTERVAL
			if (++cap_state_interval_counter >=x CONFIG_LPED_CAP_STATE_INTERVAL) {
				cap_state_interval_counter = 0;
				dice_get_cap_state(&phy_dice, &cap_state);

				if (comm_mode) 	dice_bt_set_cap_state(&bt_dice, &cap_state);
				else 			dice_bt_broadcast(&cap_state, bt_message_cap_state);
			}
#endif
		}
	}

	// Interrupt event started (movement started)
	else {
		if (IS_ENABLED(CONFIG_LPED_CAP_STATE_IN_ROLLING_MSG)) {
			dice_get_cap_state(&phy_dice, &cap_state);

			if (comm_mode) {
				dice_bt_set_cap_state(&bt_dice, &cap_state);
				dice_bt_set_dice_number(&bt_dice, NULL, bt_message_rolling);
			}

			else {
				dice_bt_broadcast(&cap_state, bt_message_rolling);
			}
		}

		else {
			cap_state_interval_counter = 0;
			if (comm_mode) 	dice_bt_set_dice_number(&bt_dice, NULL, bt_message_rolling);
			else 			dice_bt_broadcast(NULL, bt_message_rolling);
		}
	}
}

void acc_int2_callback(struct k_work *work) {
}
#endif

void dock_con_callback(struct k_work *work) {
	if (ignore_dock_disconnect) return;
	k_msleep(10);
	storage_get_comm_mode(&comm_mode);

	// Rising edge
	if (dock_get_conn_state()) {
		printk("Charging dock connected\n");
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
		fxls89xx_set_mode(acc, fxls89xx_sys_mode_standby);
#endif

		if (bt_dice.status != bt_status_connected) dice_led_on_color(&phy_dice, &COLOR_BLUE_BRIGHT);
		dice_bt_set_connectable(&bt_dice);
	}

	// Falling edge
	else {
		printk("Charging dock disconnected\n");
		if (!comm_mode) {
			dice_bt_set_invisible(&bt_dice);
			dice_led_off(&phy_dice);
		}

		else {
			if (bt_dice.status != bt_status_connected) dice_led_on_color(&phy_dice, &COLOR_BLUE_BRIGHT);
			dice_bt_set_connectable(&bt_dice);
		}

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
		acc_configure_int();
		fxls89xx_set_mode(acc, fxls89xx_sys_mode_active);
#endif
	}
}       

void bt_dice_get_acceleration_values_callback(int16_t *accel_values) {
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
	fxls89xx_interrupt_enable(acc, fxls89xx_interrupt_source_wake_out, false);
    fxls89xx_set_int2_function(acc, fxls89xx_int2_function_trigger);
    fxls89xx_trigger_read(acc);
	while (!fxls89xx_data_ready(acc)) k_msleep(5);
    accel_values[0] = fxls89xx_get_acceleration(acc, fxls89xx_axis_x);
    accel_values[1] = fxls89xx_get_acceleration(acc, fxls89xx_axis_y);
    accel_values[2] = fxls89xx_get_acceleration(acc, fxls89xx_axis_z);
    fxls89xx_set_int2_function(acc, fxls89xx_int2_function_interrupt);
#endif
}

void bt_dice_get_cap_state_callback(uint8_t *cap_state) {
	dice_get_cap_state(&phy_dice, cap_state);
}

void bt_dice_set_ignore_dock_connection_callback(bool ignore) {
	ignore_dock_disconnect = ignore;
	if (!ignore_dock_disconnect) k_work_submit(&dock_con_work);
}

void bt_dice_set_animation_callback(const animation_t *animation) {
	dice_led_start_animation(&phy_dice, animation);
}

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
void acc_int1_isr(const struct device *dev, struct gpio_callback *callback_gpio, uint32_t pins) {
	if (
		acc_int1_debounce + CONFIG_LPED_ACC_DEBOUNCE_TIME > k_uptime_get_32() || 
		last_boot_time + CONFIG_LPED_ACC_BOOT_DURATION > k_uptime_get_32()
	) {
		return;
	}
	k_work_submit(&acc_int1_work);
	acc_int1_debounce = k_uptime_get_32();
}

void acc_int2_isr(const struct device *dev, struct gpio_callback *callback_gpio, uint32_t pins) {
	if (
		acc_int2_debounce + CONFIG_LPED_ACC_DEBOUNCE_TIME > k_uptime_get_32() || 
		last_boot_time + CONFIG_LPED_ACC_BOOT_DURATION > k_uptime_get_32()
	) {
		return;
	}
	acc_int2_debounce = k_uptime_get_32();
}
#endif

void dock_con_isr(const struct device *dev, struct gpio_callback *callback_gpio, uint32_t pins) {
	if (dock_con_debounce + CONFIG_LPED_DOCK_DEBOUNCE_TIME > k_uptime_get_32()) return;
	k_work_submit(&dock_con_work);
	dock_con_debounce = k_uptime_get_32();
}

// Workers for handling bluetooth interaction - Pairing button long press, visibility timedout and connectable timedout
void dice_connectable_timeout_callback(struct k_work *work) 
{
	dice_bt_set_invisible(&bt_dice);
	dice_led_off(&phy_dice);
}

void dice_connected_timeout_callback(struct k_work *work) 
{
	ignore_dock_disconnect = false;
	dice_bt_set_invisible(&bt_dice);
	dice_led_off(&phy_dice);
}

void dice_connected_callback(struct k_work *work) 
{
	dice_led_start_connected_animation(&phy_dice);
}

void dice_disconnected_callback(struct k_work *work) 
{
	// Return this to default value in case that the client
	// application doesn't do it, which would require additional
	// restart of the die
	ignore_dock_disconnect = false;


	dice_bt_set_connectable(&bt_dice);
	dice_led_on_color(&phy_dice, &COLOR_BLUE_BRIGHT);
}

int main(void)
{
	storage_init();

	last_boot_time = k_uptime_get_32();

	if (IS_ENABLED(CONFIG_LPED_FACTORY_RESET_AT_STARTUP)) storage_clear();

	dice_bt_init(
		&bt_dice, 
		K_SECONDS(CONFIG_LPED_CONNECTABLE_TIMEOUT),
		K_SECONDS(CONFIG_LPED_CONNECTED_TIMEOUT),
		&dice_connectable_timeout_work,
		&dice_connected_timeout_work,  
		&dice_connected_work, 
		&dice_disconnected_work,
		bt_dice_get_acceleration_values_callback,
		bt_dice_get_cap_state_callback,
		bt_dice_set_ignore_dock_connection_callback,
		bt_dice_set_animation_callback
	);

	dice_phy_init(&phy_dice);

	dock_init(&dock);

#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
	k_work_init(&acc_int1_work, acc_int1_callback);
	k_work_init(&acc_int2_work, acc_int2_callback);
#endif
	k_work_init(&dock_con_work, dock_con_callback);
	k_work_init(&dice_connectable_timeout_work, dice_connectable_timeout_callback);
	k_work_init(&dice_connected_timeout_work, dice_connected_timeout_callback);
	k_work_init(&dice_connected_work, dice_connected_callback);
	k_work_init(&dice_disconnected_work, dice_disconnected_callback);

	dock_conn_int_init(&int_dock_conn_callback_gpio, dock_con_isr);

	// Accelerometer setup
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
	fxls89xx_set_range(acc, fxls89xx_range_2g);
	fxls89xx_set_mode(acc, fxls89xx_sys_mode_standby);
	fxls89xx_configure_state(acc, fxls89xx_state_wake, fxls89xx_odr_3_125Hz, fxls89xx_power_mode_low);
	fxls89xx_configure_state(acc, fxls89xx_state_sleep, fxls89xx_odr_6_25Hz, fxls89xx_power_mode_low);
	acc_configure_int();
	fxls89xx_interrupt_bind(acc, fxls89xx_interrupt_source_wake_out, fxls89xx_interrupt_pin_int1, GPIO_INT_EDGE_BOTH, &acc_int1_callback_gpio, acc_int1_isr);
#endif

	dice_led_off(&phy_dice);

	k_msleep(CONFIG_LPED_ACC_BOOT_DURATION);

	if (IS_ENABLED(CONFIG_LPED_FORCE_CONNECTABLE_AT_STARTUP)) {
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
		fxls89xx_set_mode(acc, fxls89xx_sys_mode_standby);
#endif
		dice_bt_set_connectable(&bt_dice);
		dice_led_on_color(&phy_dice, &COLOR_BLUE_BRIGHT);
	}

	else {
		if (IS_ENABLED(CONFIG_LPED_IGNORE_DOCK_AT_STARTUP)) {
#ifndef CONFIG_LPED_DEBUG_DISABLE_ACC
			fxls89xx_set_mode(acc, fxls89xx_sys_mode_active);
#endif
			storage_get_comm_mode(&comm_mode);
			if (comm_mode) {
				dice_bt_set_connectable(&bt_dice);
				dice_led_on_color(&phy_dice, &COLOR_BLUE_BRIGHT);
			}
		}

		else {
			k_work_submit(&dock_con_work);
		}
 	}

	printk("LPED running\n");

	// run debug animation if acc is disabled
#ifdef CONFIG_LPED_BOOT_ANIMATION
	k_msleep(CONFIG_LPED_ACC_BOOT_DURATION * 2); // Give space to all worker threads that could turn the LED back off prematurely
	dice_led_start_animation(&phy_dice, &animation_boot);
#endif

	return 0;
}