#include <driver/i2s.h>
#include <soc/i2s_reg.h>
#include <WiFi.h>

#define BUF_LEN 2048
#define SAMPLE_RATE 44100
#define I2S_PORT I2S_NUM_0

// Change me
#define DATA_PIN 7
#define BCLK_PIN 4
#define WS_PIN 17

// Change me
#define PC_IP IPAddress(192, 168, 1, 100)
#define SSID "YourSSID"
#define PASSWORD "YourPassword"

#define LISTEN_PORT 1592

WiFiClient client;

void setup() {
    Serial.begin(115200);

    const i2s_config_t micCfg = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        // My microphone is configured to send on the right channel.
        // A bug in ESP-IDF swaps the two, so this is set to left.
        // See https://github.com/espressif/esp-idf/issues/6625

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0,
    };

    i2s_pin_config_t micPins = {
        .bck_io_num = BCLK_PIN,
        .ws_io_num = WS_PIN,
        .data_out_num = -1,
        .data_in_num = DATA_PIN,
    };

    esp_err_t err;
    err = i2s_driver_install(I2S_PORT, &micCfg, 0, NULL);
    if (err != ESP_OK) {
        Serial.println("I2S driver initialization failed");
        while (true);
    }
    //REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(0));
    REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(1));
    REG_SET_BIT(I2S_RX_CONF1_REG(I2S_PORT), I2S_RX_MSB_SHIFT);
    err = i2s_set_pin(I2S_PORT, &micPins);
    if (err != ESP_OK) {
        Serial.println("Setting pins failed");
        while (true);
    }

    WiFi.begin(SSID, PASSWORD);
    while (!WiFi.isConnected()) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to WiFi!");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.macAddress());

    while (!client.connected()) {
        delay(500);
        Serial.print(".");
        client.connect(PC_IP, LISTEN_PORT);
    }
    client.setNoDelay(true);
    Serial.println("Connected to PC");
}

void loop() {
    static int32_t buf32[BUF_LEN];
    static int16_t buf16[BUF_LEN];
    size_t bytesRead;

    esp_err_t err = i2s_read(I2S_PORT, (uint8_t*) buf32, BUF_LEN*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("Error reading from microphone");
        Serial.println(err);
        return;
    }

    size_t samplesRead = bytesRead / sizeof(int32_t);
    for (int i=0; i<samplesRead; i++) {
        // Discard unused lower 12 bits. Sign bit is already where it needs to be.
        buf32[i] >>= 12;
        buf16[i] = (int16_t) (buf32[i] & 0xFFFF);
    }

    client.write((uint8_t*) buf16, samplesRead * sizeof(int16_t));
    client.flush();
}
