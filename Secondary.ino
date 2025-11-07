#include "driver/gpio.h"

#define PPM_FRAME_LENGTH    22500   // Gesamtlänge einer PPM-Frame in µs
#define PPM_PULSE_LENGTH      300   // Pulsdauer in µs
#define PPM_CHANNELS            6
#define DEFAULT_CHANNEL_VALUE 1500  // Neutralwert für Kanäle in µs
#define OUTPUT_PIN              4  // besser GPIO25/26/27 statt 13

volatile uint16_t channelValue[PPM_CHANNELS] = {
  DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE,
  DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE
};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

enum ppmState_e { PPM_STATE_PULSE, PPM_STATE_SPACE };

void IRAM_ATTR onPpmTimer() {
  static uint8_t  state       = PPM_STATE_PULSE;
  static uint8_t  curChannel  = 0;
  static uint32_t usedTime    = 0;

  portENTER_CRITICAL_ISR(&timerMux);

  if (state == PPM_STATE_PULSE) {
    gpio_set_level((gpio_num_t)OUTPUT_PIN, 0);  
    timerWrite(timer, 0);
    timerAlarm(timer, PPM_PULSE_LENGTH, false, 0);
    state = PPM_STATE_SPACE;
  }
  else {
    gpio_set_level((gpio_num_t)OUTPUT_PIN, 1);  

    if (curChannel >= PPM_CHANNELS) {
      uint32_t rest = PPM_FRAME_LENGTH - usedTime;
      timerWrite(timer, 0);
      timerAlarm(timer, rest, false, 0);
      curChannel = 0;
      usedTime = 0;
    } else {
      uint32_t spaceLen = channelValue[curChannel] - PPM_PULSE_LENGTH;
      usedTime += channelValue[curChannel];
      timerWrite(timer, 0);
      timerAlarm(timer, spaceLen, false, 0);
      curChannel++;
    }
    state = PPM_STATE_PULSE;
  }

  portEXIT_CRITICAL_ISR(&timerMux);
}

const int trigPin = 12;  
const int echoPin = 13;
float duration, distance;
const int trigPin1 = 9;  
const int echoPin1 = 10; 
float duration1, distance1;
const int trigPin2 = 14;  
const int echoPin2 = 15;  
float duration2, distance2;

void setup() {
  Serial.begin(115200);

  pinMode(OUTPUT_PIN, OUTPUT);
  gpio_set_level((gpio_num_t)OUTPUT_PIN, 1);

  // Neue API (Core 3.x): Frequenz in Hz
  timer = timerBegin(1000000); // 1 MHz
  timerAttachInterrupt(timer, &onPpmTimer);

  // Start mit Puls
  timerWrite(timer, 0);
  timerAlarm(timer, PPM_PULSE_LENGTH, false, 0);

  pinMode(trigPin, OUTPUT); 
	pinMode(echoPin, INPUT);
}

void loop() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration*.0343)/2;
  channelValue[0] = 1500 + distance;
  delay(100);
}