/**
 * 	 Author: 			Jiří Sedlák (xsedla2e)
 * 	 Create Time: 		25-03-2025 12:55:39
 * 	 Modified time: 	13-05-2025 16:42:36
 * 	 Description: 	    Implementation of storage functions
 */

#include "storage.h"

static struct nvs_fs file_system;


uint8_t storage_init()
{
    struct flash_pages_info info;
    
    file_system.flash_device = FIXED_PARTITION_DEVICE(storage_partition);
    if (!device_is_ready(file_system.flash_device)) {
        printk("Storage is not ready\n");
        return 1;
    }

    file_system.offset = FIXED_PARTITION_OFFSET(storage_partition);
    if (flash_get_page_info_by_offs(file_system.flash_device, file_system.offset, &info)) {
        printk("Failed to get page info\n");
        return 1;
    }

    file_system.sector_size = info.size;
    file_system.sector_count = 3U;

    if (nvs_mount(&file_system)) {
        printk("Failed to mount file system\n");
        return 1;
    }

    return 0;
}

void storage_clear()
{
    nvs_clear(&file_system);
    storage_init();
}

void storage_get_supported_dice_ids(uint8_t *ids)
{
    if (nvs_read(&file_system, SUPPORTED_DICE_IDS_ID, ids, MAX_DICE_DEFS) > 0) return;
    else {
        // Copy default from side_defs
        memset(ids, 0, MAX_DICE_DEFS);
        for (uint8_t dice_def_i = 0; dice_def_i < NUMBER_OF_PREDEFINED_DICE && dice_def_i < MAX_DICE_DEFS; dice_def_i++) {
            ids[dice_def_i] = supported_dice[dice_def_i]->header.id;
        }
        nvs_write(&file_system, SUPPORTED_DICE_IDS_ID, ids, MAX_DICE_DEFS);
    }
}

void storage_set_supported_dice_ids(uint8_t *ids)
{
    nvs_write(&file_system, SUPPORTED_DICE_IDS_ID, ids, MAX_DICE_DEFS);
}

void storage_delete_dice_definition(uint8_t *id) 
{
    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    // Find the id, and if found, delete the definition and id from array
    for (uint8_t i = 0; i < sizeof(ids); i++) {
        if (ids[i] == *id) {
            nvs_delete(&file_system, *id + DICE_DEF_OFFSET);
            ids[i] = 0;
            storage_set_supported_dice_ids(ids);
            return;
        }
    }
}

void storage_add_dice_definition(dice_definition_t *new_dice)
{
    // try to get new id, if no id is available, exit
    uint8_t id;
    storage_get_id_for_dice_definition(&id);
    if (id == EMPTY_ID) return;

    if (new_dice->header.number_of_sides > DICE_DEF_SIDES_MAX) return;

    // save newly generated id
    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);
    ids[id - 1] = id;
    storage_set_supported_dice_ids(ids);
    
    // save dice definition
    new_dice->header.id = id;
    nvs_write(&file_system, id + DICE_DEF_OFFSET, new_dice, sizeof(dice_definition_t));
}

void storage_update_dice_definition(dice_definition_t *dice)
{
    // do not save defs with empty ids
    if (dice->header.id == EMPTY_ID) return;    

    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    // check if dice_defintion is saved and update if yes
    for (uint8_t id = 0; id < sizeof(ids); id++) {
        if (ids[id] == dice->header.id) {
            nvs_write(&file_system, dice->header.id + DICE_DEF_OFFSET, dice, sizeof(dice_definition_t));
            return;
        }
    }
}

void storage_get_supported_dice_headers(dice_definition_header_t *supported_dice)
{
    uint8_t output_list_pos = 0;
    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    for (int id_index = 0; id_index < MAX_DICE_DEFS; id_index++) {
        if (ids[id_index] == EMPTY_ID) continue; // Skip empty ids
        nvs_read(&file_system, ids[id_index] + DICE_DEF_OFFSET, &supported_dice[output_list_pos++], sizeof(dice_definition_header_t));
    }
}

void storage_get_dice_definition_header(const uint8_t *id, dice_definition_header_t *header)
{
    if (*id == EMPTY_ID) {
        EMPTY_DICE_HEAD(header);
        return;
    };

    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    for (uint8_t id_index = 0; id_index < sizeof(ids); id_index++) {
        if (ids[id_index] == *id) {
            if (nvs_read(&file_system, *id + DICE_DEF_OFFSET, header, sizeof(dice_definition_header_t)) > 0) return;
            else {
                // If id is not found in flash, try to find it in predefined profiles
                for (uint8_t i = 0; i < NUMBER_OF_PREDEFINED_DICE; i++) {
                    if (supported_dice[i]->header.id == *id) {
                        // If found, save it to flash 
                        nvs_write(&file_system, *id + DICE_DEF_OFFSET, supported_dice[i], sizeof(dice_definition_t));

                        header->id = *id;
                        header->number_of_sides = supported_dice[i]->header.number_of_sides;
                        memcpy(header->name, supported_dice[i]->header.name, DICE_DEF_NAME_MAX_LEN);
                        return;
                    }
                }
            }
        }
    }
    
    EMPTY_DICE_HEAD(header);
}

void storage_get_id_for_dice_definition(uint8_t *new_id)
{
    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    // First, find the minimum id number
    for (uint8_t id_pos = 0; id_pos < MAX_DICE_DEFS; id_pos++) {
        if (ids[id_pos] == EMPTY_ID) {
            *new_id = id_pos + 1;
            return;
        }
    }

    *new_id = EMPTY_ID;
}

void storage_get_current_dice_id(uint8_t *current_id)
{
    if (nvs_read(&file_system, CURRENT_DICE_ID, current_id, sizeof(uint8_t)) > 0) return;
    else {
        *current_id = dice_6_side.header.id;
        nvs_write(&file_system, CURRENT_DICE_ID, current_id, sizeof(uint8_t));
    }
}

void storage_set_current_dice_id(const uint8_t *id)
{
    if (*id == EMPTY_ID) return;

    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    // check if given id exists in supported ids
    for (int id_pos = 0; id_pos < sizeof(ids); id_pos++) {
        if (ids[id_pos] == *id) {
            nvs_write(&file_system, CURRENT_DICE_ID, id, sizeof(uint8_t));
            return;
        }
    }
}

void storage_get_current_dice_definition(dice_definition_t *new_dice_def)
{
    uint8_t current_id;
    storage_get_current_dice_id(&current_id);
    storage_get_dice_definition(&current_id, new_dice_def);
}

void storage_get_dice_definition(const uint8_t *id, dice_definition_t *dice_def)
{
    uint8_t ids[MAX_DICE_DEFS];
    storage_get_supported_dice_ids(ids);

    for (uint8_t i = 0; i < MAX_DICE_DEFS; i++) {
        if (ids[i] == *id) {
            if (nvs_read(&file_system, *id + DICE_DEF_OFFSET, dice_def, sizeof(dice_definition_t)) > 0) return;

            // if not found in flash, look in predefined dice
            for (uint8_t o = 0; o < NUMBER_OF_PREDEFINED_DICE; o++) {
                if (supported_dice[o]->header.id == *id) {
                    // if found, save it to flash too
                    *dice_def = *supported_dice[o];
                    nvs_write(&file_system, *id + DICE_DEF_OFFSET, dice_def, sizeof(dice_definition_t));
                    return;
                }
            }
        }
    }
    // if not predefined or in flash, reset it
    EMPTY_DICE_DEF(dice_def);
}

void storage_get_side_blink(uint8_t *side_blink)
{
    if (nvs_read(&file_system, SIDE_BLINK_ID, side_blink, sizeof(uint8_t)) > 0) return;
    else {
        *side_blink = SIDE_BLINK_DEFAULT_VAL;
        nvs_write(&file_system, SIDE_BLINK_ID, side_blink, sizeof(uint8_t));
    }
}

void storage_set_side_blink(const uint8_t *side_blink)
{
    nvs_write(&file_system, SIDE_BLINK_ID, side_blink, sizeof(uint8_t));
}


void storage_get_error_blink(uint8_t *error_blink)
{
    if (nvs_read(&file_system, ERROR_BLINK_ID, error_blink, sizeof(uint8_t)) > 0) return;
    else {
        *error_blink = ERROR_BLINK_DEFAULT_VAL;
        nvs_write(&file_system, ERROR_BLINK_ID, error_blink, sizeof(uint8_t));
    }
}

void storage_set_error_blink(const uint8_t *error_blink)
{
    nvs_write(&file_system, ERROR_BLINK_ID, error_blink, sizeof(uint8_t));
}

void storage_get_comm_mode(uint8_t *comm_mode) 
{
    if (nvs_read(&file_system, COMM_MODE_ID, comm_mode, sizeof(uint8_t)) > 0) return;
    else {
        *comm_mode = ERROR_BLINK_DEFAULT_VAL;
        nvs_write(&file_system, COMM_MODE_ID, comm_mode, sizeof(uint8_t));
    }
}

void storage_set_comm_mode(const uint8_t *comm_mode)
{
    nvs_write(&file_system, COMM_MODE_ID, comm_mode, sizeof(uint8_t));
}
