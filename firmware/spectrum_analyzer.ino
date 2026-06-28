/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

  Most Arduinos have an on-board LED you can control. On the UNO, MEGA and ZERO
  it is attached to digital pin 13, on MKR1000 on pin 6. LED_BUILTIN is set to
  the correct LED pin independent of which board is used.
  If you want to know what pin the on-board LED is connected to on your Arduino
  model, check the Technical Specs of your board at:
  https://docs.arduino.cc/hardware/

  modified 8 May 2014
  by Scott Fitzgerald
  modified 2 Sep 2016
  by Arturo Guadalupi
  modified 8 Sep 2016
  by Colby Newman

  This example code is in the public domain.

  https://docs.arduino.cc/built-in-examples/basics/Blink/
*/

#include <driver/i2s.h>
#include <arduinoFFT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I2S pins
#define I2S_WS  25
#define I2S_SCK 26
#define I2S_SD  33

// FFT
#define SAMPLE_RATE     44100
#define FFT_SAMPLES     512
#define NUM_BARS        16
#define DISPLAY_HEIGHT  54  // leave room for labels

double vReal[FFT_SAMPLES];
double vImag[FFT_SAMPLES];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, FFT_SAMPLES, SAMPLE_RATE);

// Peak hold
int peaks[NUM_BARS] = {0};
int peakHoldTimer[NUM_BARS] = {0};
#define PEAK_HOLD_FRAMES 20
#define PEAK_DECAY 1

void setup() {
  Serial.begin(115200);

  // Init display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 not found");
    while (true);
  }
  display.clearDisplay();
  display.display();

  // Init I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = FFT_SAMPLES,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

void loop() {
  // Read samples from I2S
  int32_t rawSamples[FFT_SAMPLES];
  size_t bytes_read;
  i2s_read(I2S_NUM_0, rawSamples, sizeof(rawSamples), &bytes_read, portMAX_DELAY);

  // Convert to doubles and apply Hann window
  for (int i = 0; i < FFT_SAMPLES; i++) {
    vReal[i] = (double)(rawSamples[i] >> 8);
    vImag[i] = 0.0;
  }

  // Run FFT
  FFT.windowing(FFTWindow::Hann, FFTDirection::Forward);
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude();

  // Map FFT bins to display bars
  int barWidth = SCREEN_WIDTH / NUM_BARS;
  int binSize = (FFT_SAMPLES / 2) / NUM_BARS;

  display.clearDisplay();

  for (int bar = 0; bar < NUM_BARS; bar++) {
    // Average magnitude across bins for this bar
    double sum = 0;
    int binOffset = 4;  
    for (int bin = 0; bin < binSize; bin++) {
      sum += vReal[bar * binSize + bin + binOffset];
    }
    double avg = sum / binSize;

    // Scale to display height
    int barHeight = (int)(avg / 300000.0 * DISPLAY_HEIGHT);
    barHeight = constrain(barHeight, 0, DISPLAY_HEIGHT);

    int x = bar * barWidth;
    int y = DISPLAY_HEIGHT - barHeight;

    // Draw bar
    display.fillRect(x + 1, y, barWidth - 2, barHeight, SSD1306_WHITE);

    // Peak hold
    if (barHeight >= peaks[bar]) {
      peaks[bar] = barHeight;
      peakHoldTimer[bar] = PEAK_HOLD_FRAMES;
    } else {
      if (peakHoldTimer[bar] > 0) {
        peakHoldTimer[bar]--;
      } else {
        peaks[bar] = max(0, peaks[bar] - PEAK_DECAY);
      }
    }

    // Draw peak dot
    int peakY = DISPLAY_HEIGHT - peaks[bar];
    if (peaks[bar] > 0) {
      display.drawFastHLine(x + 1, peakY, barWidth - 2, SSD1306_WHITE);
    }
  }

  display.display();
}