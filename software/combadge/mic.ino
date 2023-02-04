bool setupMicI2s() {
    i2s_config_t conf = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = i2s_bits_per_sample_t(BITS_PER_SAMPLE),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
        .dma_buf_count = 8,
        .dma_buf_len = BUF_LEN,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t pins = {
        .bck_io_num = MIC_BCLK,
        .ws_io_num = MIC_WS,
        .data_in_num = MIC_SD,
    };

    esp_err_t err = i2s_driver_install(MIC_PORT, &conf, 0, NULL);
    if (err != ESP_OK)
        return false;

    err = i2s_set_pin(MIC_PORT, &pins);
    return err == ESP_OK;
}
