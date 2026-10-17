idf_component_register(SRCS "serial_link.c" "cpu_usage.c" "roll_cache.c" "dice_ui.c" "dice_tasks.c" "pseudo_rng.c" "true_rng.c" "main.c" "display.c" "rm67162.c"
                       PRIV_REQUIRES esp_driver_spi esp_driver_gpio esp_timer esp_hw_support bootloader_support esp_driver_usb_serial_jtag
                       INCLUDE_DIRS ".")
