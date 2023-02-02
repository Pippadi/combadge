bool setupSpkI2s() {
    const i2s_config_t spkCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(BITS_PER_SAMPLE),

        // .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = BUF_LEN,
    };

    i2s_pin_config_t spkPins = {
        .bck_io_num = SPK_BCLK,
        .ws_io_num = SPK_WS,
        .data_out_num = SPK_DATA,
    };

    esp_err_t err;
    err = i2s_driver_install(SPK_PORT, &spkCfg, 0, NULL);
    if (err != ESP_OK) {
        return false;
    }
    err = i2s_set_pin(SPK_PORT, &spkPins);
    if (err != ESP_OK) {
        return false;
    }

    return true;
}
