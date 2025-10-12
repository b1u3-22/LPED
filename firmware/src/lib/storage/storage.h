/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		24-03-2025 20:49:10
 * 	 Modified time: 	13-05-2025 17:40:49
 * 	 Description: 	    This file contains function for operating the flash storage
 *                      This includes CRUD operations on dice definitions and get/set
 *                      for blinking configuration
 */


#ifndef LIB_STORAGE_STORAGE_H_
#define LIB_STORAGE_STORAGE_H_

#include <zephyr/kernel.h>
#include <zephyr/fs/nvs.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/device.h>
#include "../side/side_defs.h"
#include "../side/side.h"

// Addresses
// TODO: Create new "FLAGS" field that would contain all three - side and error blinking and communication mode settings
#define SIDE_BLINK_ID           0x01
#define ERROR_BLINK_ID          0x02
#define COMM_MODE_ID            0x03
#define SUPPORTED_DICE_IDS_ID   0x04
#define CURRENT_DICE_ID         0x05
#define DICE_DEF_OFFSET         10

// Default values
#define SIDE_BLINK_DEFAULT_VAL      1
#define ERROR_BLINK_DEFAULT_VAL     1
#define COMM_MODE_DEFAULT_VAL       1

// Constants
#define EMPTY_ID        0
#define MAX_DICE_DEFS   10

/**
 * @brief Initialize storage and mount filesystem
 * @return 
 */
uint8_t storage_init();

/**
 * @brief Clear all data from flash storage and reinitialize
 */
void storage_clear();

/**
 * @brief   Get IDs from all existing saved dice definitions
 *          This will also write all predefined definitions IDs in case that the array
 *          does not yet exist
 * @param   ids array where to save the supported IDs
 */
void storage_get_supported_dice_ids(uint8_t *ids);

/**
 * @brief   Set supported IDs to new array
 */
void storage_set_supported_dice_ids(uint8_t *ids);

/**
 * @brief   Deletes dice definition with given ID 
 *          and removes the ID from array of supported IDs
 */
void storage_delete_dice_definition(uint8_t *id);

/**
 * @brief   Save new definition to persistent storage
 *          This will also generate ID for given definition
 *          and save its ID to supported definitions IDs
 * @param new_dice new dice definition that should be saved
 */
void storage_add_dice_definition(dice_definition_t *new_dice);

/**
 * @brief   Update information of given dice definition if it exists
 *          in array of supported dice definitions (has to be added first)
 * @param   dice updated dice definition
 */
void storage_update_dice_definition(dice_definition_t *dice);

/**
 * @brief   Get array of dice definition headers from all supported dice definitions
 * @param   supported_dice array where to save found headers
 */
void storage_get_supported_dice_headers(dice_definition_header_t *supported_dice);

/**
 * @brief   Get single dice definition header for given dice definition ID
 *          This function will also search in predefined profiles the given dice def
 *          is not found in flash and save it before returning
 * @param   header header where to save the found header if found, otherwise it will be set to empty header
 */
void storage_get_dice_definition_header(const uint8_t *id, dice_definition_header_t *header);

/**
 * @brief   ID and position in the id array is almost the same, the id is just +1 than it's position
 *          the id array could be saved as bit map, but for easier manipulation (e.g. sending 
 *          via Bluetooth) it's an array of uint8_t with length of UINT8_MAX)
 * @param   new_id id where to save non-taken id if available, EMPTY_ID otherwise
 */
void storage_get_id_for_dice_definition(uint8_t *new_id);

/**
 * @brief   Get currently selected dice definition from flash
 *          if no definition is selected (the value has not been written yet),
 *          it selects the default six-sided profile
 * @param   current_id  id of currently selected one, if none selected,
 *                      six-sided definition will be selected
 */
void storage_get_current_dice_id(uint8_t *current_id);

/**
 * @brief   Select dice definition with given ID as currently used
 *          if the given ID is not found in the supported definition, 
 *          the selected definition does not change
 * @param   id ID of profile to use
 */
void storage_set_current_dice_id(const uint8_t *id);

/**
 * @brief   Get currently selected dice definition, selects six-sided def if none selected
 * @param   new_dice_def where the currently used one, or six-sided if none used
 */
void storage_get_current_dice_definition(dice_definition_t *new_dice_def);

/**
 * @brief   Get dice definition with given ID, this function will also search 
 *          the predefined profiles if the definition is not found in flash
 * @param   id          ID of definition to get
 * @param   dice_def    definition where to save the found one or empty definition if none found
 */
void storage_get_dice_definition(const uint8_t *id, dice_definition_t *dice_def);

/**
 * @brief   Get current setting for side number blinking
 *          if this value wasn't set before, it will be set to enabled (true)
 * @param   side_blink where to save the value from flash
 */
void storage_get_side_blink(uint8_t *side_blink);

/**
 * @brief   Set current setting for side number blinking to given value
 * @param   side_blink new value for side number blinking
 */
void storage_set_side_blink(const uint8_t *side_blink);

/**
 * @brief   Get current setting for error blinking (invalid side)
 *          if this value wasn't set before, it will be set to enabled (true)
 * @param   error_blink where to save the value from flash
 */
void storage_get_error_blink(uint8_t *error_blink);

/**
 * @brief   Set current setting for error to given value
 * @param   side_blink new value for error blinking
 */
void storage_set_error_blink(const uint8_t *error_blink);

/**
 * @brief   Get current setting for communication mode
 *          if this value wasn't set before, it will be set to connection (true)
 * @param   error_blink where to save the value from flash
 */
void storage_get_comm_mode(uint8_t *comm_mode);

/**
 * @brief   Set current setting for communication mode to given value
 * @param   side_blink new value for communication mode 
 */
void storage_set_comm_mode(const uint8_t *comm_mode);

#endif // LIB_STORAGE_STORAGE_H_