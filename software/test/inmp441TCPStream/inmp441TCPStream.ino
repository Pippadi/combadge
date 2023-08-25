#include <driver/i2s.h>
#include <WiFi.h>

#define BUF_LEN 2048
#define SAMPLE_RATE 44100
#define I2S_PORT I2S_NUM_0

// Change me
#define DATA_PIN 33
#define BCLK_PIN 32
#define WS_PIN 25

// Change me
#define PC_IP IPAddress(192, 168, 0, 50)
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
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,
        // L/R should be grounded for I2S_CHANNEL_FMT_ONLY_LEFT.
        // If only zeroes are being received, set this to
        // I2S_CHANNEL_FMT_ONLY_RIGHT. A bug in ESP-IDF swaps the two.
        // See https://github.com/espressif/esp-idf/issues/6625

        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S),
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

    client.connect(PC_IP, LISTEN_PORT);
    while (!client.connected()) { delay(10); }
}

void loop() {
    static int32_t buf32[BUF_LEN];
    static int16_t buf[BUF_LEN];
    size_t bytesRead;

    delay(int(1000.0 * float(BUF_LEN) / float(SAMPLE_RATE)) / 2);

    esp_err_t err = i2s_read(I2S_PORT, (uint8_t*) buf32, BUF_LEN*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("Error reading from microphone");
        return;
    }

    size_t sampleCnt = bytesRead / sizeof(int32_t);
    for (int i=0; i<sampleCnt; i++) {
        // Discard unused lower 8 bits, and get rid of 3 bits of noise.
        // The number 11 was empirically determined to provide the best signal.
        buf32[i] >>= 11;
        buf[i] = (int16_t) buf32[i];
    }

    client.write((uint8_t*) buf, sampleCnt * sizeof(int16_t));
}
