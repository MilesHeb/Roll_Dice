# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

# LVGL 8.3 has no Kconfig entry for the tick expression and defaults to Arduino's millis()
idf_build_set_property(COMPILE_DEFINITIONS "LV_TICK_CUSTOM_SYS_TIME_EXPR=(esp_timer_get_time()/1000LL)" APPEND)

project(Dice_Roll)
