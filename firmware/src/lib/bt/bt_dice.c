/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		13-05-2025 16:09:21
 * 	 Modified time: 	13-05-2025 19:01:06
 * 	 Description: 	    This file contains definitions for Bluetooth related function
 *                      declared in the bt_dice header. 
 *                      Additionally, it contains necessary functions and other definitions
 *                      for operating the peripheral mode, such as GATT characteristics 
 */

#include "bt_dice.h"

void broadcast(const uint8_t *message, bt_message_t message_type) 
{
    static uint8_t mfg_data[BR_MFG_SIZE];
    mfg_data[0] = BR_MFG_LSB;
    mfg_data[1] = BR_MFG_MSB;
    mfg_data[2] = id;
    mfg_data[3] = message_type;
    mfg_data[4] = message != NULL ? *message : 0x00;
    mfg_data[5] = 0x00; // Reserved for future use that could require 16-bit numbers in messages

    static const struct bt_data ad[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, strlen(CONFIG_BT_DEVICE_NAME)),
        BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, BR_MFG_SIZE)
    };

    if (bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, ad, ARRAY_SIZE(ad), NULL, 0)) {
        printk("Failed to start broadcasting message: %u with type: %u with id: %u\n", *message, message_type, id);
        return;
    }

    k_msleep(BR_RETRY_MAX_TIME * BR_NUM_OF_RETRIES);  
    bt_le_adv_stop();
    id++;
}

void dice_bt_broadcast(const uint8_t *message, const bt_message_t message_type)
{
    // Enable bluetooth
    if (bt_enable(NULL)) {
        printk("Failed to turn bluetooth on\n");
        return;
    }

    // Broadcast the message in the buffer
    broadcast(message, message_type);

    // Disable bluetooth after message has been broadcasted
    if (bt_disable()) {
        printk("Failed to turn bluetooth off\n");
    }
}

static bt_dice_dev_t *bt_dice_global;

// Custom GATT service
static const struct bt_uuid_128 gatt_dice_svc_uuid =                BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0A0));

// Custom GATT characteristics
static const struct bt_uuid_128 gatt_side_blink_uuid =              BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B0));
static const struct bt_uuid_128 gatt_error_blink_uuid =             BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B1));
static const struct bt_uuid_128 gatt_dice_def_uuid =                BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B2));
static const struct bt_uuid_128 gatt_supported_dice_defs_ids_uuid = BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B3));
static const struct bt_uuid_128 gatt_current_dice_def_id_uuid =     BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B4));
static const struct bt_uuid_128 gatt_selected_dice_def_uuid =       BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B5));
static const struct bt_uuid_128 gatt_dice_update_uuid =             BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B6));
static const struct bt_uuid_128 gatt_accelerometer_uuid =           BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B7));
static const struct bt_uuid_128 gatt_command_uuid =                 BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B8));
static const struct bt_uuid_128 gatt_comm_mode_uuid =               BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0B9));

static const struct bt_uuid_128 gatt_dice_number_uuid =             BT_UUID_INIT_128(BT_UUID_128_ENCODE(0xF9B126C7, 0xECEA, 0x4D1F, 0xA4E2, 0xECC3EB60E0C0));


static uint8_t side_blink;
static uint8_t error_blink;
static uint8_t comm_mode;
static dice_definition_t dice_def;
static uint8_t supported_dice_defs_ids[MAX_DICE_DEFS];
static dice_definition_header_t selected_dice_def;
static uint8_t current_dice_def_id;
static uint8_t new_dice_def_id;
static int16_t acc_values[3];
static uint8_t command_buf;

/* 
    This value is used to notify the app when in connected communication mode. 
    Valid number sides are only uint8_t, the second byte is used for status, such as rolling or unknown.
    For all values @see bt_message_t enum 
*/
static uint8_t dice_number[2];


// Work for the function that runs periodically in the background 
struct k_work dice_bt_state_work;

// Timer for the periodic function
struct k_timer dice_bt_state_timer;

/**
 * @brief Function to get the acceleration and cap values
 */
void dice_bt_state_callback(struct k_work *work) 
{
    if (bt_dice_global->status != bt_status_connectable) {
        k_timer_stop(&dice_bt_state_timer);
        return;
    }

    bt_dice_global->get_acceleration_callback(acc_values);
}

void dice_bt_state_timer_callback() 
{
    k_work_submit(&dice_bt_state_work);
}

/**
 * @brief Function to initialize all variables used in GATT services
 */
void dice_bt_load_data() 
{
    storage_get_supported_dice_ids(supported_dice_defs_ids);
    storage_get_side_blink(&side_blink);
    storage_get_error_blink(&error_blink);
    storage_get_comm_mode(&comm_mode);
    storage_get_current_dice_definition(&dice_def);
    storage_get_dice_definition_header(&supported_dice_defs_ids[0], &selected_dice_def);
    storage_get_current_dice_id(&current_dice_def_id);
    new_dice_def_id = 0;
}

// ============ GATT READ CALLBACKS ============

static ssize_t gatt_read(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(uint8_t));
}

static ssize_t gatt_read_supported_dice_defs_ids(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(uint8_t) * MAX_DICE_DEFS);
}

static ssize_t gatt_read_selected_dice_def(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
	return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(dice_definition_header_t));
}

static ssize_t gatt_read_dice_def(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)  
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(dice_definition_t));
}

static ssize_t gatt_read_dice_update(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(new_dice_def_id));
}

static ssize_t gatt_read_accelerometer(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    bt_dice_global->get_acceleration_callback(acc_values);
    const char *value = attr->user_data;

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(acc_values));
}

static ssize_t gatt_read_comm_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset) 
{
    const char *value = attr->user_data;
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);

    return bt_gatt_attr_read(conn, attr, buf, len, offset, value, sizeof(uint8_t));
}

// ============ GATT WRITE CALLBACKS ============

static ssize_t gatt_write_side_blink(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    if (len < 1) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

    const uint8_t *value = buf;

    // Set side blink to requested value and update it
    storage_set_side_blink(value);
    side_blink = *value >= 1;

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

static ssize_t gatt_write_error_blink(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    if (len < 1) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

    const uint8_t *value = buf;

    // Change error blink to requested value and update it
    storage_set_error_blink(value);
    error_blink = *value >= 1;

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

static ssize_t gatt_write_comm_mode(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    if (len < 1) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

    const uint8_t *value = buf;

    // Change error blink to requested value and update it
    storage_set_comm_mode(value);
    comm_mode = *value >= 1;

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

static ssize_t gatt_write_current_dice_def_id(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    const uint8_t *value = buf;
    // Change the used dice def id to requested one
    storage_set_current_dice_id(value);

    // Update the data and id for currently used dice definition
    storage_get_current_dice_definition(&dice_def);
    current_dice_def_id = *value;

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

static ssize_t gatt_write_selected_dice_def(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    const uint8_t *value = buf;

    // get the requested dice header
    storage_get_dice_definition_header(value, &selected_dice_def);

    // check if it exists
    if (selected_dice_def.id == EMPTY_ID) return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}


static bt_update_header_t *update_params;
static dice_definition_t update_dice_def;
static dice_definition_t *update_dice_def_param;
static side_definition_t *update_side_def;
static ssize_t gatt_write_dice_update(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    const uint8_t *value = buf;

    if (len < sizeof(bt_update_header_t)) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

    update_params = (bt_update_header_t *) value;

    switch (update_params->action)
    {
        case bt_update_action_die_add:
            storage_get_id_for_dice_definition(&new_dice_def_id);

            // Check if new id is available
            if (new_dice_def_id == EMPTY_ID)                                         return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);

            // Check if data contain the dice definition header
            if (len < sizeof(dice_definition_header_t) + sizeof(bt_update_header_t)) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

            // Create new dice definition and copy values into it
            update_dice_def_param = (dice_definition_t *) &value[sizeof(bt_update_header_t)];
            memcpy(&update_dice_def, update_dice_def_param, sizeof(dice_definition_header_t));
            memset(&update_dice_def.sides, 0, sizeof(side_definition_t) * DICE_DEF_SIDES_MAX);

            // Save the dice, and refresh supported dice defs
            storage_add_dice_definition(&update_dice_def);
            storage_get_supported_dice_ids(supported_dice_defs_ids);
            break;

        case bt_update_action_die_delete:
            // Delete the dice definition and update supported ids
            storage_delete_dice_definition(&update_params->dice_id);
            storage_get_supported_dice_ids(supported_dice_defs_ids);
            break;

        case bt_update_action_die_update:
            // Check if data contain the required dice header
            if (len < sizeof(dice_definition_header_t) + sizeof(bt_update_header_t)) return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
            
            // Get the requested dice def to update
            update_dice_def_param = (dice_definition_t *) &value[sizeof(bt_update_header_t)];
            storage_get_dice_definition(&update_dice_def_param->header.id, &update_dice_def);

            // Check if the requested dice def exists
            if (update_dice_def.header.id == EMPTY_ID) return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);

            // Update the dice def values
            memcpy(&update_dice_def, update_dice_def_param, sizeof(dice_definition_header_t));

            // Update the definition
            storage_update_dice_definition(&update_dice_def);

            // Refresh the currently selected dice def if it was updated
            if (update_dice_def.header.id == dice_def.header.id) storage_get_dice_definition(&update_dice_def.header.id, &dice_def);
            break;

        case bt_update_action_side_add:
            // Get the dice definition where to add the side
            storage_get_dice_definition(&update_params->dice_id, &update_dice_def);

            // check if the dice def exists
            if (update_dice_def.header.id == EMPTY_ID)                                   return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);

            // check if the dice def has space for new side def
            if (update_dice_def.header.number_of_sides >= DICE_DEF_SIDES_MAX)            return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);

            // check if data contain the required side definition data
            if (len < sizeof(side_definition_t) + sizeof(bt_update_header_t))            return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

            // Add side to the dice definition, and increment number of sides
            update_side_def = (side_definition_t *) &value[sizeof(bt_update_header_t)];
            update_dice_def.sides[update_dice_def.header.number_of_sides++] = *update_side_def;

            // Update the side definition
            storage_update_dice_definition(&update_dice_def);

            // Refresh the currently used definition if needed 
            if (update_dice_def.header.id == dice_def.header.id) storage_get_dice_definition(&update_dice_def.header.id, &dice_def);
            break;

        case bt_update_action_side_delete:
            // Check if given side index is within bounds
            if (update_params->side_index >= DICE_DEF_SIDES_MAX)                        return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);
            
            // Get requested dice definition
            storage_get_dice_definition(&update_params->dice_id, &update_dice_def);

            // Check if the dice definition exists
            if (update_dice_def.header.id == EMPTY_ID)                                  return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);

            // Check if the side index is within the number of sides of the definition
            if (update_params->side_index > update_dice_def.header.number_of_sides - 1) return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);

            // delete side def from dice def and update by shifting all sides that are after the deleted one to 
            for (int i = update_params->side_index; i < DICE_DEF_SIDES_MAX - 1; i++) {
                update_dice_def.sides[i] = update_dice_def.sides[i + 1];
            }

            // fill the last side def space with zeroes
            memset(&update_dice_def.sides[DICE_DEF_SIDES_MAX - 1], 0, sizeof(side_definition_t)); 

            // decrease number of sides
            update_dice_def.header.number_of_sides--;

            // Update the dice definition
            storage_update_dice_definition(&update_dice_def);

            // Refresh the currently used definition if needed 
            if (update_dice_def.header.id == dice_def.header.id) storage_get_dice_definition(&update_dice_def.header.id, &dice_def);
            break;

        case bt_update_action_side_update:
            // Get requested dice definition
            storage_get_dice_definition(&update_params->dice_id, &update_dice_def);

            // Check if it exists
            if (update_dice_def.header.id == EMPTY_ID)                                   return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);

            // Check if side index is within the number of sides
            if (update_params->side_index > update_dice_def.header.number_of_sides - 1)  return BT_GATT_ERR(BT_ATT_ERR_OUT_OF_RANGE);

            // Check if data contain required side data
            if (len < sizeof(side_definition_t) + sizeof(bt_update_header_t))            return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);

            update_side_def = (side_definition_t *) &value[sizeof(bt_update_header_t)];
            
            // update the side definition, save it and reload it for the current dice definition provided in gatt char ...03
            update_dice_def.sides[update_params->side_index] = *update_side_def;
            storage_update_dice_definition(&update_dice_def);
            
            // Refresh the currently used definition if needed 
            if (update_dice_def.header.id == dice_def.header.id) storage_get_dice_definition(&update_dice_def.header.id, &dice_def);
            break;

        default:
            return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);
    }

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

static ssize_t gatt_write_command(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags) 
{
    const uint8_t *value = buf;

    switch (*value)
    {
        case bt_command_restart:
            //sys_reboot(SYS_REBOOT_COLD);
            break;

        case bt_command_clear_memory:
            // perform storage clear and refresh all data
            storage_clear();
            dice_bt_load_data();
            break;

        case bt_command_disable_dock_disconnect:
            bt_dice_global->set_dock_ignore_callback(true);
            break;

        case bt_command_enable_dock_disconnect:
            bt_dice_global->set_dock_ignore_callback(false);
            break;

        default:
            return BT_GATT_ERR(BT_ATT_ERR_ATTRIBUTE_NOT_FOUND);
    }

    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);
    return len;
}

// ============ GATT CUD CALLBACK ============
static ssize_t gatt_read_cud(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
    const char *name = attr->user_data;
    return bt_gatt_attr_read(conn, attr, buf, len, offset, name, strlen(name));
}


// Define GATT services
BT_GATT_SERVICE_DEFINE(
    dice_svc,
    BT_GATT_PRIMARY_SERVICE(&gatt_dice_svc_uuid),
    BT_GATT_CHARACTERISTIC(
        &gatt_side_blink_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read, gatt_write_side_blink, &side_blink
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Side blink"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_error_blink_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read, gatt_write_error_blink, &error_blink
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Error blink"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_dice_def_uuid.uuid,
        BT_GATT_CHRC_READ, 
        BT_GATT_PERM_READ,
        gatt_read_dice_def, NULL, &dice_def
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Full dice definition"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_supported_dice_defs_ids_uuid.uuid,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        gatt_read_supported_dice_defs_ids, NULL, &supported_dice_defs_ids
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Supported dice definition IDs"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_current_dice_def_id_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read, gatt_write_current_dice_def_id, &current_dice_def_id
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Current dice definition ID"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_selected_dice_def_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read_selected_dice_def, gatt_write_selected_dice_def, &selected_dice_def
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Selected dice definition header"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_dice_update_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read_dice_update, gatt_write_dice_update, &new_dice_def_id
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Side/dice definition updates"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_accelerometer_uuid.uuid,
        BT_GATT_CHRC_READ,
        BT_GATT_PERM_READ,
        gatt_read_accelerometer, NULL, &acc_values
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Acceleration values"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_command_uuid.uuid,
        BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_WRITE,
        NULL, gatt_write_command, &command_buf
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Commands"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_comm_mode_uuid.uuid,
        BT_GATT_CHRC_READ | BT_GATT_CHRC_WRITE,
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
        gatt_read_comm_mode, gatt_write_comm_mode, &comm_mode
    ),
    BT_GATT_DESCRIPTOR(
        BT_UUID_GATT_CUD,
        BT_GATT_PERM_READ,
        gatt_read_cud, NULL, "Communication mode"
    ),

    BT_GATT_CHARACTERISTIC(
        &gatt_dice_number_uuid.uuid,
        BT_GATT_CHRC_INDICATE,
        BT_GATT_PERM_NONE,
        NULL, NULL, &dice_number
    ),
    BT_GATT_CCC(
        NULL, 
        BT_GATT_PERM_READ | BT_GATT_PERM_WRITE
    )
);

void connected(struct bt_conn *connection, uint8_t error) {
    if (error) {
        printk("Failed to connect due to: %u\n", error);
        return;
    }

    bt_dice_global->conn = bt_conn_ref(connection);
    bt_dice_global->status = bt_status_connected;
    printk("Connected\n");

    k_timer_stop(&bt_dice_global->bonding_timeout_timer);
    k_timer_stop(&bt_dice_global->visible_timeout_timer);
    k_timer_start(&bt_dice_global->bonded_timeout_timer, bt_dice_global->bonded_timeout_duration, K_NO_WAIT);

    // Start periodic functions to get capacitor and accelerometer state
    k_timer_start(&dice_bt_state_timer, K_MSEC(CONFIG_LPED_BT_STATE_INTERVAL), K_MSEC(CONFIG_LPED_BT_STATE_INTERVAL));

    k_work_submit(bt_dice_global->connected_work);
}

void disconnected(struct bt_conn *connection, uint8_t error) {
    printk("Disconnected due to: %u\n", error);
    if (bt_dice_global->conn) {
        bt_conn_unref(bt_dice_global->conn);
        bt_dice_global->conn = NULL;
        bt_dice_global->status = bt_status_connectable;
    }

    k_work_submit(bt_dice_global->disconnected_work);

    // stop periodic reading of capacitor and accelerometer
    k_timer_stop(&dice_bt_state_timer);
}

void pairing_confirm(struct bt_conn *connection) {
    printk("Received bonding request\n");
    if (bt_dice_global->status != bt_status_connectable) {
        bt_conn_auth_cancel(connection);
        printk("Not in bonding mode, bonding rejected\n");
        return;
    }

    bt_conn_auth_pairing_confirm(connection);
    printk("Confirmed request\n");
}

void visibility_expired(struct k_timer *timer_id) {
    bt_dice_dev_t *dice = CONTAINER_OF(timer_id, bt_dice_dev_t, visible_timeout_timer);
    k_work_submit(dice->visible_timeout_work);
}

void bonding_expired(struct k_timer *timer_id) {
    bt_dice_dev_t *dice = CONTAINER_OF(timer_id, bt_dice_dev_t, bonding_timeout_timer);
    k_work_submit(dice->bonding_timeout_work);
}

void bonded_expired(struct k_timer *timer_id) {
    bt_dice_dev_t *dice = CONTAINER_OF(timer_id, bt_dice_dev_t, bonded_timeout_timer);
    k_work_submit(dice->bonded_timeout_work);
}

void dice_bt_init(
    bt_dice_dev_t *dice, 
    k_timeout_t visible_timeout, 
    k_timeout_t bonding_timeout, 
    k_timeout_t bonded_timeout, 
    struct k_work *bonding_timeout_work,
    struct k_work *visible_timeout_work,
    struct k_work *bonded_timeout_work,
    struct k_work *connected_work,
    struct k_work *disconnected_work,
    void (*get_acceleration_callback)(int16_t *buffer),
    void (*get_cap_state_callback)(uint8_t *buffer),
    void (*set_dock_ignore_callback)(bool ignore)
)
{
    dice->authentication_callback.pairing_confirm = pairing_confirm;
    dice->connection_callback.connected = connected;
    dice->connection_callback.disconnected = disconnected;

    bt_conn_cb_register(&dice->connection_callback);
    bt_conn_auth_cb_register(&dice->authentication_callback);

    dice->visible_timeout_duration = visible_timeout;
    dice->bonding_timeout_duration = bonding_timeout;
    dice->bonded_timeout_duration = bonded_timeout;
    k_timer_init(&dice->visible_timeout_timer, visibility_expired, NULL);
    k_timer_init(&dice->bonding_timeout_timer, bonding_expired, NULL);
    k_timer_init(&dice->bonded_timeout_timer, bonded_expired, NULL);
    
    dice->status = bt_status_invisible;
    
    dice->bonding_timeout_work = bonding_timeout_work;
    dice->visible_timeout_work = visible_timeout_work;
    dice->bonded_timeout_work = bonded_timeout_work;
    dice->connected_work = connected_work;
    dice->disconnected_work = disconnected_work;

    dice->get_acceleration_callback = get_acceleration_callback;
    dice->get_cap_state_callback = get_cap_state_callback;
    dice->set_dock_ignore_callback = set_dock_ignore_callback;

    dice->dice_number_att = bt_gatt_find_by_uuid(dice_svc.attrs, dice_svc.attr_count, &gatt_dice_number_uuid.uuid);

    bt_dice_global = dice;

    dice_bt_load_data();

    // init timers and workers for periodic functions
    // k_work_init(&dice_bt_state_work, dice_bt_state_callback);
    // k_timer_init(&dice_bt_state_timer, dice_bt_state_timer_callback, NULL);
}

void dice_bt_set_invisible(bt_dice_dev_t *dice)
{
    if (dice->status == bt_status_invisible) return;
    if (bt_le_adv_stop()) {
        printk("Failed to stop advertising\n");
        return;
    }

    dice->status = bt_status_invisible;         // Change status of the die
    k_timer_stop(&dice->bonding_timeout_timer); // Stop timers 
    k_timer_stop(&dice->visible_timeout_timer);

    if (bt_disable()) printk("Failed to turn bluetooth off");
}

void dice_bt_set_visible(bt_dice_dev_t *dice)
{
    if (dice->status == bt_status_invisible) {
        if (bt_enable(NULL)) {
            printk("Failed to turn bluetooth on\n");
            return;
        }
    }

    if (dice->status == bt_status_connectable || dice->status == bt_status_connected) {
        if (bt_le_adv_stop()) {
            printk("Failed to stop connectable advertising\n");
            return;
        }
    }


    // If dice is already visible, just reset the timeout timer
    if (dice->status == bt_status_visible) {
        k_timer_start(&dice->visible_timeout_timer, dice->visible_timeout_duration, K_NO_WAIT);
        return;
    }

    static const struct bt_data data[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, strlen(CONFIG_BT_DEVICE_NAME)),
    };
    if (bt_le_adv_start(BT_LE_ADV_NCONN_IDENTITY, data, ARRAY_SIZE(data), NULL, 0)) {
        printk("Failed to start advertising name\n");
        return;
    }

    dice->status = bt_status_visible;
    k_timer_stop(&dice->bonding_timeout_timer);
    k_timer_start(&dice->visible_timeout_timer, dice->visible_timeout_duration, K_NO_WAIT);
}

void dice_bt_set_bondable(bt_dice_dev_t *dice)
{
    // if device is already bondable, only reset the timer
    if (dice->status == bt_status_connectable || dice->status == bt_status_connected) {
        k_timer_start(&dice->bonding_timeout_timer, dice->bonding_timeout_duration, K_NO_WAIT);
        return;
    }

    if (bt_le_adv_stop()) {
        printk("Failed to stop advertising\n");
        return;
    }

    if (dice->status == bt_status_invisible) {
        if (bt_enable(NULL)) {
            printk("Failed to enable Bluetooth\n");
            return;
        }
    }

    static const struct bt_data data[] = {
        BT_DATA(BT_DATA_NAME_COMPLETE, CONFIG_BT_DEVICE_NAME, strlen(CONFIG_BT_DEVICE_NAME)),
    };
    if (bt_le_adv_start(BT_LE_ADV_CONN, data, ARRAY_SIZE(data), NULL, 0)) {
        printk("Failed to start connectable advertising\n");
        return;
    }

    dice->status = bt_status_connectable;
    k_timer_start(&dice->bonding_timeout_timer, dice->bonding_timeout_duration, K_NO_WAIT);
    k_timer_stop(&dice->visible_timeout_timer);
}

void dice_bt_notify(bt_dice_dev_t *dice, uint8_t *message, const bt_message_t message_type)
{
    if (!dice->conn || dice->status != bt_status_connected || bt_gatt_is_subscribed(dice->conn, dice->dice_number_att, BT_GATT_CCC_INDICATE)) return;

    dice_number[NF_MESSAGE_POS] = message != NULL ? *message : 0x00;
    dice_number[NF_STATUS_POS] = message_type; 

    struct bt_gatt_indicate_params params = {
        .attr = dice->dice_number_att, 
        .data = dice_number, 
        .len  = sizeof(dice_number),
        .func = NULL
    };

    bt_gatt_indicate(dice->conn, &params);
}

void dice_bt_send(bt_dice_dev_t *dice, uint8_t *message, const bt_message_t message_type)
{
    static uint8_t comm_mode;
    storage_get_comm_mode(&comm_mode);

    if (comm_mode)  dice_bt_notify(dice, message, message_type);
    else            dice_bt_broadcast(message, message_type);
}
