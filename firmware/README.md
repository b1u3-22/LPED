# Firmware for LPED
* Zephyr firmware running on the die
* The `src/lib` contains helper function and structure definitions for controlling the die, dock and determining the current number on top
* A FXLS8974 driver is required to be placed in `modules` and it is available at: https://git.fit.vutbr.cz/xsedla2e/BP-FXLS89xx-Driver

## `src/lib` structure
1. `bma400`
    * Older driver for BMA400 accelerometer
    * Not currently used
2. `bt`
    * Contains structure for Bluetooth aspects of the die and functions to send messages and change the Bluetooth states
    * `bt_dice.h` also contains definitions for message types
3. `dock`
    * Contains structure and functions for controlling the charging dock - LED and button handling
4. `phy`
    * Contains structure for physical aspects of the die - controlling the LED and getting the capacitor state
5. `side`
    * Contains array with all supported sides and their vector values and function to determine the current side
6. `led_defs.h`
    * LED speed definitions