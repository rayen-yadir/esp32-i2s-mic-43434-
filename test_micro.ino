#include <driver/i2s_std.h>

#define I2S_SD   4
#define I2S_SCK  5
#define I2S_WS   9

#define SAMPLE_RATE      16000              // taux demande au peripherique I2S
#define WAV_SAMPLE_RATE  (SAMPLE_RATE * 2)  // taux REEL du flux (bug connu I2S mono ESP32 = souvent 2x)
#define RECORD_SECONDS   5
#define TOTAL_SAMPLES    (WAV_SAMPLE_RATE * RECORD_SECONDS)

i2s_chan_handle_t rx_handle;

void setup() {
  Serial.begin(921600); // Vitesse max pour envoyer les donnees vite
  delay(1000);

  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  i2s_new_channel(&chan_cfg, NULL, &rx_handle);

  i2s_std_config_t std_cfg = {
    .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
    .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
                  I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
    .gpio_cfg = {
      .mclk = I2S_GPIO_UNUSED,
      .bclk = (gpio_num_t)I2S_SCK,
      .ws   = (gpio_num_t)I2S_WS,
      .dout = I2S_GPIO_UNUSED,
      .din  = (gpio_num_t)I2S_SD,
      .invert_flags = { false, false, false },
    },
  };

  i2s_channel_init_std_mode(rx_handle, &std_cfg);
  i2s_channel_enable(rx_handle);

  Serial.println("READY");
  delay(500);
  Serial.println("Enregistrement dans 3...");
  delay(1000);
  Serial.println("2...");
  delay(1000);
  Serial.println("1...");
  delay(1000);
  Serial.println("GO ! Parle maintenant !");

  // Buffer
  const int CHUNK = 256;
  int32_t buf32[CHUNK];
  int16_t buf16[CHUNK];

  int totalSamplesRead = 0;

  // Entete WAV
  int dataSize = TOTAL_SAMPLES * 2; // 16 bits = 2 bytes par sample
  int fileSize = dataSize + 44 - 8;

  Serial.write("RIFF", 4);
  Serial.write((uint8_t*)&fileSize, 4);
  Serial.write("WAVE", 4);
  Serial.write("fmt ", 4);
  int fmtSize = 16;
  Serial.write((uint8_t*)&fmtSize, 4);
  int16_t audioFormat = 1; // PCM
  Serial.write((uint8_t*)&audioFormat, 2);
  int16_t numChannels = 1; // Mono
  Serial.write((uint8_t*)&numChannels, 2);
  int sampleRate = WAV_SAMPLE_RATE;
  Serial.write((uint8_t*)&sampleRate, 4);
  int byteRate = WAV_SAMPLE_RATE * 2;
  Serial.write((uint8_t*)&byteRate, 4);
  int16_t blockAlign = 2;
  Serial.write((uint8_t*)&blockAlign, 2);
  int16_t bitsPerSample = 16;
  Serial.write((uint8_t*)&bitsPerSample, 2);
  Serial.write("data", 4);
  Serial.write((uint8_t*)&dataSize, 4);

  // Envoie les samples
  while (totalSamplesRead < TOTAL_SAMPLES) {
    size_t bytesRead = 0;
    i2s_channel_read(rx_handle, buf32, sizeof(buf32), &bytesRead, 1000);
    int samplesRead = bytesRead / 4;

    for (int i = 0; i < samplesRead; i++) {
      buf16[i] = (int16_t)(buf32[i] >> 16);
    }

    Serial.write((uint8_t*)buf16, samplesRead * 2);
    totalSamplesRead += samplesRead;
  }

  Serial.println("\nENDOFWAV");
  Serial.println("Enregistrement termine !");
}

void loop() {}
