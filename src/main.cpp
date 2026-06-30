#include <FastLED.h>
#undef LITTLE_ENDIAN          // DFRobot_BMI160.h unconditionally redefines this; suppress redefinition warning
#include <DFRobot_BMI160.h> // BMI160 library
#include <Wire.h>      // I2C library

// Microphone
#include "driver/i2s.h"
#include "dsps_fft2r.h"
#include "dsps_wind.h"

// Button state
volatile uint32_t pressStartMs    = 0;   // millis() when button went down
volatile bool     releaseDetected = false; // set by ISR on button release
volatile uint32_t pressDurationMs = 0;   // duration of last press in ms

// Menu / effect state
uint8_t  activeEffect     = 1;     // 1 = rainbowFlow, 2 = audioSpectrum
bool     explosionPending = false; // long press queued explosion
uint8_t  menuChoice       = 1;     // kept for HARDCODED_MENU compatibility


#include "effects.h"
#include "globals.h"

#define BRIGHTNESS  30
#define FRAMES_PER_SECOND 1000
#define I2C_SDA 47
#define I2C_SCL 21
#define I2C_ADDR 0x69

uint8_t brightness = 10;
uint16_t frame = 0;

CRGB leds[NUM_LEDS];

int32_t fft_raw[FFT_SIZE];        // Raw int32_t PCM samples from I2S
float fft_input[FFT_SIZE];        // Converted float samples
float complex_buffer[FFT_SIZE * 2]; // Interleaved real/imaginary data
// 17 boundaries = 16 bands, log-spaced 60 Hz–8 kHz (Nyquist limit at 16 kHz sample rate).
// One band per arm on LARGE_PARASOL; adjacent pairs averaged on SMALL_PARASOL (8 arms).
extern const int bands[] = {60, 80, 110, 150, 200, 270, 370, 500,
                             680, 920, 1250, 1700, 2300, 3150, 4300, 6000, 8000};
extern const int numBands = sizeof(bands)/sizeof(bands[0]) - 1; // = 16
float bandMagnitudes[16] = {0};

DFRobot_BMI160 bmi160;

int iterating_variable=0;
bool blinker = true;

const int8_t i2c_addr = 0x69;

void IRAM_ATTR handleButton() {
  if (digitalRead(BUTTON_PIN_1)) {          // RISING — button pressed down
    pressStartMs = millis();
  } else {                                  // FALLING — button released
    pressDurationMs = millis() - pressStartMs;
    releaseDetected = true;
  }
}

void BMI_setup(){
  Wire.end();
  Wire.begin(I2C_SDA, I2C_SCL); // SDA = GPIO47, SCL = GPIO21
  //Wire.setPins( I2C_SDA,  I2C_SCL);
  int rslt = bmi160.I2cInit(I2C_ADDR);
  if (rslt != BMI160_OK && DEBUG_MODE) {
    Serial.print("BMI160 init failed with: ");
    Serial.println(rslt);
    while (1) {
    }
  }

  //init the hardware bmin160  
  if (bmi160.softReset() != BMI160_OK){
    while(DEBUG_MODE)
    {
      Serial.println("reset false");
    }
  }
  
  //set and init the bmi160 i2c address
  if (bmi160.I2cInit(I2C_ADDR) != BMI160_OK){
    while(DEBUG_MODE) 
    {
      Serial.println("init false");
    }
  }
}

void i2c_mic_setup(){
  i2s_config_t i2s_config = {};
  i2s_config.mode              = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX);
  i2s_config.sample_rate       = SAMPLE_RATE;
  i2s_config.bits_per_sample   = I2S_BITS_PER_SAMPLE_32BIT;
  i2s_config.channel_format    = I2S_CHANNEL_FMT_ONLY_LEFT;
  i2s_config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  i2s_config.intr_alloc_flags  = 0;
  i2s_config.dma_buf_count     = 4;
  i2s_config.dma_buf_len       = 512;
  i2s_config.use_apll          = false;
  i2s_config.tx_desc_auto_clear = false;
  i2s_config.fixed_mclk        = 0;

  i2s_pin_config_t i2s_mic_pins = {};
  i2s_mic_pins.bck_io_num   = MIC_I2C_SCK;
  i2s_mic_pins.ws_io_num    = MIC_I2C_WS;
  i2s_mic_pins.data_out_num = I2S_PIN_NO_CHANGE;
  i2s_mic_pins.data_in_num  = MIC_I2C_SD;

  ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
  ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &i2s_mic_pins));

      // Initialize FFT
  esp_err_t ret = dsps_fft2r_init_fc32(NULL, FFT_SIZE);
  #ifdef DEBUG_MODE
    if (ret != ESP_OK) Serial.println("FFT init failed");
  #endif
}

void print_mic_data(){
  // read from the I2S device
  size_t bytes_read = 0;
  i2s_read(I2S_NUM_0, fft_raw, FFT_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);
  int samples_read = bytes_read / sizeof(int32_t);
  // dump the samples out to the serial channel.
  for (int i = 0; i < samples_read; i++)
  {
    #ifdef DEBUG_MODE 
      Serial.printf("%ld\n", fft_raw[i]);
    #endif
  }
}

void fastFourierTransformAudio(){
  size_t bytes_read;
  
  // 3. Read audio samples into a correctly-typed int32_t buffer
  i2s_read(I2S_NUM_0, fft_raw, FFT_SIZE * sizeof(int32_t), &bytes_read, portMAX_DELAY);

  // 4. Generate Hann window, convert samples to float, apply window, build complex buffer
  float hann_window[FFT_SIZE];
  dsps_wind_hann_f32(hann_window, FFT_SIZE);
  for (int i = 0; i < FFT_SIZE; i++) {
    // I2S 32-bit frame: data is in the upper 24 bits, drop the lowest 8
    int32_t raw_sample = fft_raw[i] >> 8;
    float normalized = (float)raw_sample / 8388608.0f; // Normalize to [-1, 1]
    complex_buffer[i * 2]     = normalized * hann_window[i]; // Real, windowed
    complex_buffer[i * 2 + 1] = 0.0f;                        // Imaginary
  }

  // 5. Compute FFT
  dsps_fft2r_fc32(complex_buffer, FFT_SIZE);
  dsps_bit_rev_fc32(complex_buffer, FFT_SIZE);

  // 6. Calculate magnitudes and group into bands
  float binWidth = (float)SAMPLE_RATE / FFT_SIZE;
  for (int band = 0; band < numBands; band++) {
    int startBin = (int)(bands[band] / binWidth);
    int endBin = (int)(bands[band+1] / binWidth);
    bandMagnitudes[band] = 0;
    
    for (int i = startBin; i < endBin; i++) {
      float real = complex_buffer[i * 2];
      float imag = complex_buffer[i * 2 + 1];
      bandMagnitudes[band] += sqrtf(real*real + imag*imag);
    }
    if (endBin > startBin)
      bandMagnitudes[band] /= (endBin - startBin); // Average
  }

  // 7. Map magnitudes to LEDs
  for (int i = 0; i < numBands; i++) {
    float magnitude = bandMagnitudes[i];
    magnitude = log10(magnitude + 1) * 20; // Convert to dB-like scale
    int ledLevel = map(constrain(magnitude, 0, 40), 0, 40, 0, NUM_LEDS_PER_STRIP);
    
    // Light up LEDs (example: 4 bands → 4 LED segments)
    int ledsPerBand = NUM_LEDS / numBands;
    for (int j = 0; j < ledsPerBand; j++) {
      int pos = i * ledsPerBand + j;
      leds[pos] = (j < ledLevel) ? CRGB(CHSV(i * 40, 255, 255)) : CRGB::Black;

    }
    #ifdef DEBUG_MODE
      Serial.print(ledLevel);
      Serial.print(" ");
    #endif
  }
  #ifdef DEBUG_MODE
    for (int i = 0; i < numBands; i++) {
      Serial.print(bandMagnitudes[i]);
      Serial.print(" "); // Add a space between numbers
    }
    Serial.println(); // Newline after printing
  #endif

  // 8. Add decay effect for smooth transitions
  fadeToBlackBy(leds, NUM_LEDS, 50);
}

void fastLedSetup(){
      // tell FastLED there's 60 CHIPSET leds on pin 10, starting at index 0 in the led array
      FastLED.addLeds<CHIPSET, DATA1>(leds, 0, NUM_LEDS_PER_STRIP);
      // tell FastLED there's 60 CHIPSET leds on pin 11, starting at index 60 in the led array
      FastLED.addLeds<CHIPSET, DATA2>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA3>(leds, 2 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA4>(leds, 3 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA5>(leds, 4 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA6>(leds, 5 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA7>(leds, 6 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      FastLED.addLeds<CHIPSET, DATA8>(leds, 7 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      #ifdef LARGE_PARASOL //IFDEF to allow SMALL_PARASOL to use these pins for additional buttons etc. 
        FastLED.addLeds<CHIPSET, DATA9>(leds, 8 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA10>(leds, 9 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA11>(leds, 10 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA12>(leds, 11 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA13>(leds, 12 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA14>(leds, 13 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA15>(leds, 14 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
        FastLED.addLeds<CHIPSET, DATA16>(leds, 15 * NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
      #endif
}

void setup() {
  delay(3000); // sanity delay
  Serial.begin(115200);
  fastLedSetup();
  FastLED.setBrightness( BRIGHTNESS );

  i2c_mic_setup();

  pinMode(BUTTON_PIN_1, INPUT_PULLDOWN);
  attachInterrupt(BUTTON_PIN_1, handleButton, CHANGE);

  //BMI_setup();
}
void buttonHandler() {
  if (!releaseDetected) return;
  releaseDetected = false;        // consume the event
  iterating_variable = 0;         // reset animation state for whichever effect runs next

  if (pressDurationMs >= 2000) {
    // Long press: trigger explosion once, then return to current effect
    explosionPending = true;
  } else {
    // Short press: cycle forward through effects 1 … MENU_MAX-1
    activeEffect = (activeEffect % (MENU_MAX - 1)) + 1;
    menuChoice = activeEffect;
  }
}

void bmi_loop(){  
  while(true){
    int i = 0;
    int rslt;
    int16_t accelGyro[6]={0}; 
    
    //get both accel and gyro data from bmi160
    //parameter accelGyro is the pointer to store the data
    rslt = bmi160.getAccelGyroData(accelGyro);
    #ifdef DEBUG_MODE
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
      delay(100);
    #endif
    
    
    //only read accel data from bmi160
    int16_t onlyAccel[3]={0};
    bmi160.getAccelData(onlyAccel);
    

    //only read gyro data from bmi160
    int16_t onlyGyro[3]={0};
    bmi160.getGyroData(onlyGyro);
  }
}

void blinkLed(int blinks)
{
  for ( int i = 0; i < blinks; i++)
  {
    leds[60] = CRGB::HotPink;
    FastLED.show();
    delay(250);
    leds[60] = CRGB::Black;
    FastLED.show();
  }

  if(menuChoice == 0)
  { blinkLed(10); }
}

void loop()
{
  if (HARDCODED_MENU > 0) menuChoice = HARDCODED_MENU;

  if (explosionPending) {
    // Run explosion phases; when all 3 phases complete (iterating_variable > 2),
    // clear the flag and return to the previously active effect.
    explosionEffect(leds, iterating_variable);
    if (iterating_variable > 2) {
      explosionPending = false;
      iterating_variable = 0;
      menuChoice = activeEffect;
    }
  } else {
    switch (menuChoice) {
      case 1:
        rainbowFlow(leds, iterating_variable);
        break;
      case 2:
        audioSpectrum(leds);
        break;
      case 3:
        sparklingRain(leds);
        break;
      default:
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        break;
    }
  }

  buttonHandler();
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
        #ifdef DEBUG_MODE)  Serial.print(accelGyro[i]*3.14/180.0);Serial.print("\t");
      }else{
        //the following three data are accel data
        #ifdef DEBUG_MODE)  Serial.print(accelGyro[i]/16384.0);Serial.print("\t");
      }
    }
    #ifdef DEBUG_MODE)  Serial.println();
  }else{
    #ifdef DEBUG_MODE)  Serial.println("err");
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
  
  
