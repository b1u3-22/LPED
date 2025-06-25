cmake_minimum_required(VERSION 3.20.0)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/modules/BP-FXLS89xx-Driver)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(lped)

#target_sources(app PRIVATE src/lib/bma400/bma400.c)
target_sources(app PRIVATE src/lib/phy/phy_dice.c)
target_sources(app PRIVATE src/lib/side/side.c)
target_sources(app PRIVATE src/lib/bt/bt_dice.c)
target_sources(app PRIVATE src/lib/dock/dock.c)
target_sources(app PRIVATE src/lib/storage/storage.c)

target_sources(app PRIVATE src/main.c)
