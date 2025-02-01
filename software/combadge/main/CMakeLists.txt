idf_component_register(SRCS "combadge.cpp"
    REQUIRES driver
    PRIV_REQUIRES "esp_wifi" "esp_timer" "nvs_flash" "esp_driver_touch_sens" "i2speripherals"
    INCLUDE_DIRS ".")
