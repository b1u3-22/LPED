# LPED

Highly customizable Low-power Electronic Playing Dice

> This project started as a bachelors thesis, the original repository can be found [here](https://git.fit.vutbr.cz/xsedla2e/BP-LPED)

## Features

* Separate charging station for charging the dice via pogo-pins
* Changing the dice shape with custom sleeves 
* The dice can be configured to support any shape with up to 60 sides
* Up to 10 profiles can be saved onboard 

### Hardware 

* The dice uses a nRF52810 microcontroller with FXLS8974CF accelerometer
* Instead of battery it uses hybrid super-capacitor with 20 F capacity that provides up to 8 hours of game time with charging time of 15 minutes
* The 3D models of the charging dock have been optimized for 3D printing and they do not require any supports
	* However, one heat inserts and 4 magnets (6x6x2mm) are necessary for the assembly

### Firmware 

* The dice transmits the landed numbers via BLE broadcast advertisement
	* To ensure they get to the client application, they are sent multiple times with unique identifier
* For configuration it is necessary to connect to the dice
	* It provides all the necessary parameters via characteristics in custom GATT service
* It saves the whole configuration in the FLASH memory of the dice, making it independent on the used client application
* Each side can have its own physical position on the dice and its own number, making it possible to create dice with custom non-standard shapes

## Project structure 
* `/hardware` - schematics, 3D models and PCB design files for dice, charger and programming adapter
	* The schematics and PCB designs were made in KiCad 
	* The printable models were made in FreeCAD 
* `/firmware` - Zephyr firmware running on the dice

## Versioning info 
* Repository uses `major.minor` format
	* `major` is incremented with each major hardware revision or significant firmware update that causes the implementation to be incompatible with previous version
	* `minor` is used for firmware fixes and patches that don't affect backwards compatibility
	* Branch names are created based on major revisions 
* PCBs have their own version code on them that follows standard `major.minor` format
	* `major` is used for drastic changes to the boards, such as MCU swap or change of pinout
	* `minor` is used for hardware fixes and patches