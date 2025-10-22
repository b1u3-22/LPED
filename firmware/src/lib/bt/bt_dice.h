/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		15-02-2025 23:25:37
 * 	 Modified time: 	13-05-2025 18:19:33
 * 	 Description: 	    This file contains definition for bluetooth dice structure
 *                      and other necessary structures or enums.
 *                      IT also contains declarations for operating the bluetooth functions of the dice
 */

#ifndef LIB_BT_BT_DICE_H_
#define LIB_BT_BT_DICE_H_

#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/bas.h>
#include <zephyr/sys/reboot.h>

#include <libfxls89xx.h>


#include "../dock/dock.h"
#include "../storage/storage.h"
#include "../side/side.h"

#define BR_NUM_OF_RETRIES           3       // Broadcaster number of retries for one message 
#define BR_RETRY_MAX_TIME           150     // Maximum retry time in milliseconds for broadcaster
#define BR_MFG_LSB                  0x1F    // Manufacturer identifier LSB    
#define BR_MFG_MSB                  0x01    // Manufacturer identifier MSB
#define BR_MFG_SIZE                 6       // Total length of manufacturer data

/**
 * @brief ID used in broadcast messages
 */
static uint8_t id __attribute__((unused)) = 0;

/**
 * @brief Enum defining broadcast message types
 */
typedef enum {
    bt_message_dice_number  = 0x01, // Landed number
    bt_message_cap_state    = 0x02, // Capacitor state (not to be used when in connected communication mode as status identifier)
    bt_message_rolling      = 0x03, // Dice is rolling
    bt_message_unknown      = 0x04  // Dice landed on non-valid number (e.g. landed on edge or outside of the RANGE)
} bt_message_t;

/**
 * @brief Enum defining bluetooth states of the dice
 */
typedef enum {
    bt_status_invisible,    // Dice is invisible, BT turned off
    bt_status_connectable,  // Dice is visible and connectable
    bt_status_connected     // Dice is visible and connected
} bt_status_t;

/**
 * @brief Enum defining available commands
 */
typedef enum {
    bt_command_restart,                 // Cold-reboot
    bt_command_clear_memory,            // Factory reset (storage clear)
    bt_command_disable_dock_disconnect, // Ignore dock disconnect/connect detection
    bt_command_enable_dock_disconnect   // Enable dock disconnect/connect detection 
} bt_command_t;

/**
 * @brief Enum defining available update actions
 */
typedef enum {
    bt_update_action_die_add,       // Add new dice definition
    bt_update_action_die_delete,    // Delete existing dice definition
    bt_update_action_die_update,    // Update existing dice definition
    bt_update_action_side_add,      // Add new side to existing dice definition
    bt_update_action_side_delete,   // Delete existing side from existing dice definition
    bt_update_action_side_update    // Update existing side in existing dice definition
} bt_update_action_t;

/**
 * @brief   Header used in the update action characteristic. This header denotes 
 *          what action is being done. It also contains necessary informations for
 *          about dice definition, however these can be 0, depending on the action
 */
typedef struct bt_update_header {
    uint8_t action;     // What action is being done
    uint8_t dice_id;    // ID of dice definition to delete/update or where to delete/update/add side definition
    uint8_t side_index; // Index of side definition inside dice definition, only required when deleteing or updating side
} bt_update_header_t;

/**
 * @brief Structure representing the dice from the bluetooth side, containing necessary callbacks, works and timers
 */
typedef struct bt_dice_dev {
    bt_status_t status;                                 // Current bluetooth status 

    k_timeout_t connectable_timeout_duration;                   // Time before transitioning from bondable state back to invisible
    k_timeout_t connected_timeout_duration;                    // Time before transitioning from connected state back to invisible 
    
    struct k_timer connectable_timeout_timer;                   // Timer used for connectable timeout
    struct k_timer visible_timeout_timer;                   // Timer used for visible timeout
    struct k_timer connected_timeout_timer;                    // Timer used for connected timeout 

    struct k_work *connectable_timeout_work;                    // Work for connectable timeout callback
    struct k_work *connected_timeout_work;                     // Work for connected timeout callback
    struct k_work *connected_work;                          // Work for connected callback
    struct k_work *disconnected_work;                       // Work for disconnected callback

    struct bt_conn *conn;                                   // Bluetooth connection structure 
    struct bt_conn_cb connection_callback;                  // Bluetooth connection callback

    void (*get_acceleration_callback)(int16_t *buffer);     // Callback for getting acceleration values
    void (*get_cap_state_callback)(uint8_t *buffer);        // Callback for getting capacitor state value
    void (*set_dock_ignore_callback)(bool ignore);          // Callback for changing the dock connection detection
    void (*set_animation_callback)(const animation_t *animation); // Callback for manually starting animations

    struct bt_gatt_attr *dice_number_att;                   // Dice number attribute
} bt_dice_dev_t;

/**
 * @brief Send given message with given type in non-connectable advertising, this automatically increments the message ID
 * @param message       Message to send
 * @param message_type  Type of the message
 */
void dice_bt_broadcast(const uint8_t *message, const bt_message_t message_type);

/**
 * @brief Initialize the bluetooth dice structure
 * @param dice                          Dice to initialize
 * @param connectable_timeout               Time before connectable timeout
 * @param connected_timeout                Time before connected timeout
 * @param connectable_timeout_work          Work where to submit on connectable timeout
 * @param connected_timeout_work           Work where to submit on connected timeout
 * @param connected_work                Work where to submit when connected
 * @param disconnected_work             Work where to submit when disconnected
 * @param get_acceleration_callback     Callback for getting acceleration values
 * @param get_cap_state_callback        Callback for getting capacitor state value
 * @param set_dock_ignore_callback      Callback for changing dock connection detection
 */
void dice_bt_init(
    bt_dice_dev_t *dice, 
    k_timeout_t connectable_timeout, 
    k_timeout_t connected_timeout, 
    struct k_work *connectable_timeout_work,
    struct k_work *connected_timeout_work,
    struct k_work *connected_work,
    struct k_work *disconnected_work,
    void (*get_acceleration_callback)(int16_t *buffer),
    void (*get_cap_state_callback)(uint8_t *buffer),
    void (*set_dock_ignore_callback)(bool ignore),
    void (*set_animation_callback)(const animation_t *animation)
);

/**
 * @brief   Change the dice bluetooth state to invisible,
 *          disconnecting from any connected devices and 
 *          turning the Bluetooth module OFF
 * @param dice Dice
 */
void dice_bt_set_invisible(bt_dice_dev_t *dice);

/**
 * @brief   Change the dice bluetooth state to visible,
 *          turning the Bluetooth module ON and
 *          starting connectable advertising.
 *          If already connectable, this will reset the 
 *          connectable timeout timer
 * @param dice Dice
 */
void dice_bt_set_connectable(bt_dice_dev_t *dice);

void dice_bt_set_dice_number(bt_dice_dev_t *dice, uint8_t *number, bt_message_t status);
void dice_bt_set_cap_state(bt_dice_dev_t *dice, uint8_t *state);

#endif // LIB_BT_BT_DICE_H_