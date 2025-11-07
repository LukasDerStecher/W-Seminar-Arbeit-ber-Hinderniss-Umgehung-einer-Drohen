#include "driver/gpio.h"

#define PPM_FRAME_LENGTH    22500   // Gesamtlänge einer PPM-Frame in µs
#define PPM_PULSE_LENGTH      300   // Pulsdauer in µs
#define PPM_CHANNELS            6
#define DEFAULT_CHANNEL_VALUE 1500  // Neutralwert für Kanäle in µs
#define OUTPUT_PIN              27  // besser GPIO25/26/27 statt 13

volatile uint16_t channelValue[PPM_CHANNELS] = {
  DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE,
  DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE, DEFAULT_CHANNEL_VALUE
};

hw_timer_t *timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

enum ppmState_e { PPM_STATE_PULSE, PPM_STATE_SPACE };

volatile bool ppmBusy = false;

void IRAM_ATTR onPpmTimer() {
  ppmBusy = true;

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

   ppmBusy = false;
}

volatile unsigned long int a,b,c;
volatile int x[15],ch1[15],ch[7],i;

volatile unsigned long int a1,b1,c1;
volatile int x1[15],ch3[15],ch2[7],i1;

hw_timer_t * timerRX = NULL;  // Hardware-Timer Handle

void IRAM_ATTR read_me()  {
 //this code reads value from RC reciever from ch pin (Pin 2 or 3)
 //this code gives channel values from 0-1000 values 
 //    -: ABHILASH :-    //
  a = timerRead(timerRX);   // statt micros()
  c=a-b;      //calculating time inbetween two peaks
  b=a;        // 
  x[i]=c;     //storing 15 value in array
  i=i+1;       if(i==15){for(int j=0;j<15;j++) {ch1[j]=x[j];}
  i=0;}}//copy store all values from temporary array another array after 15 reading

void IRAM_ATTR read_me1()  {
 //this code reads value from RC reciever from ch pin (Pin 2 or 3)
 //this code gives channel values from 0-1000 values 
 //    -: ABHILASH :-    //
  a1 = timerRead(timerRX);   // statt micros()
  c1=a1-b1;      //calculating time inbetween two peaks
  b1=a1;        // 
  x1[i1]=c1;     //storing 15 value in array
  i1=i1+1;       if(i1==15){for(int j1=0;j1<15;j1++) {ch3[j1]=x1[j1];}
  i1=0;}}//copy store all values from temporary array another array after 15 reading

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

  pinMode(13, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(13), read_me, FALLING);
  // enabling interrupt at pin 2 
  
  pinMode(15, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(15), read_me1, FALLING);
  // enabling interrupt at pin 2 

  timerRX = timerBegin(1000000);  // 1 MHz Timer
}

void loop() {
  read_rc();
  read_rc1();

  for (int i=1; i<=6; i++) {
    channelValue[i-1] = constrain(ch[i], 1000, 2000);
  }
  
  if(channelValue[5] < 1300){
    if(ch2[1] < 40 && ch2[1] > 5){
      channelValue[2-1] = constrain(channelValue[1-1] - (1000*ch2[1]),1300,2000);
    }
  }
    
    for (int i=0; i<6; i++) {
    Serial.print(channelValue[i]);Serial.print("\t");
    }Serial.print("\n");
}

void read_rc(){
  int i,j,k=0;
    for(k=14;k>-1;k--){if(ch1[k]>5000){j=k;}}  //detecting separation space 10000us in that another array                     
    for(i=1;i<=6;i++){ch[i]=(ch1[i+j]);}} //assign 6 channel values after separation space

void read_rc1(){
  int i1,j1,k1=0;
    for(k1=14;k1>-1;k1--){if(ch3[k1]>10000){j1=k1;}}  //detecting separation space 10000us in that another array                     
    for(i1=1;i1<=6;i1++){ch2[i1]=(ch3[i1+j1]-1510);}} //assign 6 channel values after separation space
