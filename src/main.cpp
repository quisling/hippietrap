#include <FastLED.h>
//#include <DFRobot_BMI160.h> // BMI160 library
#include <Wire.h>      // I2C library

// Microphone
#include "driver/i2s.h"
#include "dsps_fft2r.h"
#include "dsps_wind.h"

volatile bool buttonPressed = false;

#include "effects.h"
#include "globals.h"

#define SERIAL_ENABLED true

#define BRIGHTNESS  150
#define FRAMES_PER_SECOND 1000
#define I2C_SDA 4
#define I2C_SCL 5
#define I2C_ADDR 0x68

uint8_t brightness = 10;
uint16_t frame = 0;

CRGB leds[NUM_LEDS];

int32_t raw_samples[512];
float fft_input[FFT_SIZE];    // Real signal input
float complex_buffer[FFT_SIZE * 2]; // Interleaved real/imaginary data

//DFRobot_BMI160 bmi160;

int chaseLed=0;
bool blinker = true;

//DFRobot_BMI160 bmi160;
const int8_t i2c_addr = 0x69;

//void IRAM_ATTR handleButton() {
//  buttonPressed = true;         // Set flag when button is pressed
//}

void BMI_setup(){
  Serial.begin(115200);
  delay(100);
  Serial.println("Hello!");
  
  /*/init the hardware bmin160  
  if (bmi160.softReset() != BMI160_OK){
    
    while(1) Serial.println("reset false");
  }
  
  //set and init the bmi160 i2c address
  if (bmi160.I2cInit(i2c_addr) != BMI160_OK){
    while(1) Serial.println("init false");
    //
  }*/
}

void i2c_mic_setup(){
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 8000,  // 8KHz default
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 4,
    .dma_buf_len = 1024
  };
  
  // and don't mess around with this
  i2s_pin_config_t i2s_mic_pins = {
    .bck_io_num = MIC_I2C_SCK,
    .ws_io_num = MIC_I2C_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = MIC_I2C_SD
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);

      // Initialize FFT
  esp_err_t ret = dsps_fft2r_init_fc32(NULL, FFT_SIZE);
  if (ret != ESP_OK) Serial.println("FFT init failed");

}

void print_mic_data(){
  // read from the I2S device
  size_t bytes_read = 0;
  i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * 512, &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);
  // dump the samples out to the serial channel.
  for (int i = 0; i < samples_read; i++)
  {
    Serial.printf("%ld\n", raw_samples[i]);
  }
}

void fastFourierTransformAudio(){
  size_t bytes_read;
  
  // 3. Read audio samples from INMP441
  i2s_read(I2S_NUM_0, fft_input, FFT_SIZE * sizeof(float), &bytes_read, portMAX_DELAY);
  
  // 4. Convert 24-bit audio to 32-bit float (INMP441 data is MSB-aligned)
  for (int i = 0; i < FFT_SIZE; i++) {
    int32_t raw_sample = ((int32_t)fft_input[i]) >> 8; // Discard lower 8 bits
    complex_buffer[i * 2] = (float)raw_sample / 8388608.0; // Normalize to [-1, 1]
    complex_buffer[i * 2 + 1] = 0;
  }

  // 5. Apply Hann window
  dsps_wind_hann_f32(complex_buffer, FFT_SIZE);

  // 6. Compute FFT
  dsps_fft2r_fc32(complex_buffer, FFT_SIZE);
  dsps_bit_rev_fc32(complex_buffer, FFT_SIZE); // Reorder output

  // 7. Calculate magnitude spectrum
  float magnitude[FFT_SIZE / 2];
  for (int i = 0; i < FFT_SIZE / 2; i++) {
    float real = complex_buffer[i * 2];
    float imag = complex_buffer[i * 2 + 1];
    magnitude[i] = sqrtf(real * real + imag * imag);
  }

  // 8. Find peak frequency
  int peak_bin = 0;
  float peak_val = 0;
  for (int i = 1; i < FFT_SIZE / 2; i++) { // Skip DC
    if (magnitude[i] > peak_val) {
      peak_val = magnitude[i];
      peak_bin = i;
    }
  }
  float peak_freq = (peak_bin * SAMPLE_RATE) / (float)FFT_SIZE;
  
  Serial.printf("Dominant frequency: %.1f Hz\n", peak_freq);
  delay(100);
}

void fastLedSetup(){
      // tell FastLED there's 60 NEOPIXEL leds on pin 10, starting at index 0 in the led array
      FastLED.addLeds<NEOPIXEL, DATA1>(leds, 0, NUM_LEDS_PER_STRIP);
      // tell FastLED there's 60 NEOPIXEL leds on pin 11, starting at index 60 in the led array
      FastLED.addLeds<NEOPIXEL, DATA2>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA3>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA4>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA5>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA6>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA7>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<NEOPIXEL, DATA8>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      if (LARGE_PARASOL){
        FastLED.addLeds<NEOPIXEL, DATA9>(leds, 8 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA10>(leds, 9 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA11>(leds, 10 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA12>(leds, 11 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA13>(leds, 12 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA14>(leds, 13 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA15>(leds, 14 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<NEOPIXEL, DATA16>(leds, 15 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      }
}

void setup() {
  delay(3000); // sanity delay
  if (SERIAL_ENABLED){
    Serial.begin(115200); // Start serial at 115200 baud
    Serial.println("Hello from ESP32-S3!");
  }
  fastLedSetup();
  FastLED.setBrightness( BRIGHTNESS );

  i2c_mic_setup();

  //pinMode(BUTTON_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  //attachInterrupt(BUTTON_PIN, handleButton, FALLING);

    //Wire.begin(I2C_SDA, I2C_SCL); // SDA = GPIO47, SCL = GPIO21
    //if (bmi160.I2cInit(I2C_ADDR) != BMI160_OK) {
    //  Serial.println("BMI160 init failed!");
    //  while (1);
    //}
}

void loop()
{
  // Add entropy to random number generator
  //random16_add_entropy( random());

  //chaserLed(leds, chaseLed, blinker);
  //powerTest(leds, blinker);
  rainbowFlow(leds,chaseLed);
  //wobbleRing(leds, chaseLed);
  //clockLed(leds);
  //print_mic_data();
  fastFourierTransformAudio();
  /*if (buttonPressed) {
    Serial.println("Button was pressed!");
    buttonPressed = false;      // Reset flag
    // Add your logic here, e.g., increment a counter or toggle a value
  }
    */
  FastLED.show(); // display this frame
  //FastLED.delay(1000 / FRAMES_PER_SECOND);
  //Serial.println("Hello hello");
  /*frame++;
  brightness = frame%100;
  if (brightness>=250){
    frame = 0;
  }
  FastLED.setBrightness( brightness );
  */
}

/*void BMI_loop(){  
  int i = 0;
  int rslt;
  int16_t accelGyro[6]={0}; 
  
  //get both accel and gyro data from bmi160
  //parameter accelGyro is the pointer to store the data
  rslt = bmi160.getAccelGyroData(accelGyro);
  if(rslt == 0){
    for(i=0;i<6;i++){
      if (i<3){
        //the first three are gyro data
        Serial.print(accelGyro[i]*3.14/180.0);Serial.print("\t");
      }else{
        //the following three data are accel data
        Serial.print(accelGyro[i]/16384.0);Serial.print("\t");
      }
    }
    Serial.println();
  }else{
    Serial.println("err");
  }
  delay(100);*/
  /*
   * //only read accel data from bmi160
   * int16_t onlyAccel[3]={0};
   * bmi160.getAccelData(onlyAccel);
   */

  /*
   * ////only read gyro data from bmi160
   * int16_t onlyGyro[3]={0};
   * bmi160.getGyroData(onlyGyro);
   */
//}
  
  
