bool setupMicI2s() {
    i2s_config_t conf = {};
    conf.mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX);
    conf.sample_rate = SAMPLE_RATE;
    conf.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
    conf.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
    conf.communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB);
    conf.dma_buf_count = 8;
    conf.dma_buf_len = BUF_LEN;
    conf.use_apll = false;
    conf.tx_desc_auto_clear = false;
    conf.fixed_mclk = 0;

    i2s_pin_config_t pins = {};
    pins.bck_io_num = MIC_BCLK;
    pins.ws_io_num = MIC_WS;
    pins.data_in_num = MIC_SD;
    pins.data_out_num = -1;

    esp_err_t err = i2s_driver_install(MIC_PORT, &conf, 0, NULL);
    if (err != ESP_OK)
        return false;

    err = i2s_set_pin(MIC_PORT, &pins);
    return err == ESP_OK;
}
