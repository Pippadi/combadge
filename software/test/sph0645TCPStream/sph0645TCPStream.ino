#include <driver/i2s_std.h>
#include <soc/i2s_reg.h>
#include <WiFi.h>

#define BUF_LEN 256
#define SAMPLE_RATE 44100
#define I2S_PORT I2S_NUM_0

// Change me
#define DATA_PIN GPIO_NUM_7
#define BCLK_PIN GPIO_NUM_4
#define WS_PIN GPIO_NUM_17

// Change me
#define PC_IP IPAddress(192, 168, 1, 100)
#define SSID "YourSSID"
#define PASSWORD "YourPassword"

#define LISTEN_PORT 1592

WiFiClient client;

i2s_chan_handle_t rx_handle;
i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_PORT, I2S_ROLE_MASTER);
i2s_std_slot_config_t slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO);

i2s_std_config_t rx_conf = {
    .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
    .slot_cfg = slot_cfg,
    .gpio_cfg = {
        .mclk = I2S_GPIO_UNUSED,
        .bclk = BCLK_PIN,
        .ws = WS_PIN,
        .dout = I2S_GPIO_UNUSED,
        .din = DATA_PIN,
        .invert_flags = {
            .mclk_inv = false,
            .bclk_inv = false,
            .ws_inv = false,
        },
    },
};

void setup() {
    Serial.begin(115200);

    rx_conf.slot_cfg.slot_mask = I2S_STD_SLOT_LEFT;
    i2s_new_channel(&chan_cfg, NULL, &rx_handle);
    i2s_channel_init_std_mode(rx_handle, &rx_conf);
    i2s_channel_enable(rx_handle);

    //REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(0));
    REG_SET_BIT(I2S_RX_TIMING_REG(I2S_PORT), BIT(1));
    REG_SET_BIT(I2S_RX_CONF1_REG(I2S_PORT), I2S_RX_MSB_SHIFT);

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

    esp_err_t err = i2s_channel_read(rx_handle, (uint8_t*) buf32, BUF_LEN*sizeof(int32_t), &bytesRead, portMAX_DELAY);
    if (err != ESP_OK) {
        Serial.println("Error reading from microphone");
        Serial.println(err);
        return;
    }

    size_t samplesRead = bytesRead / sizeof(int32_t);
    for (int i=0; i<samplesRead; i++) {
        // Discard unused lower 11 bits. Sign bit is already where it needs to be.
        buf32[i] >>= 11;
        buf16[i] = (int16_t) (buf32[i] & 0xFFFF);
    }

    client.write((uint8_t*) buf16, samplesRead * sizeof(int16_t));
    client.flush();
}
