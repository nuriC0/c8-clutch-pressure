#include <Arduino.h>
#include <SPI.h>
#include "MedianFilterLib2.h"

#define ODD       0
#define EVEN      1

#define BASE_CS   34
#define CLUTCH_CS 26

#define BASE_IN   1
#define ODD_IN    2
#define EVEN_IN   3

#define BASE_OUT  6
#define ODD_OUT   7
#define EVEN_OUT  8

void setChipValue(int cs, uint16_t value, int chan);

unsigned long sleepCountDown = 0;

uint16_t BASE_NUM = 1020;

MedianFilter2<uint16_t> medianBaseIn(5);
MedianFilter2<uint16_t> medianOddIn(5);
MedianFilter2<uint16_t> medianEvenIn(5);

MedianFilter2<uint16_t> medianBaseOut(5);
MedianFilter2<uint16_t> medianOddOut(5);
MedianFilter2<uint16_t> medianEvenOut(5);

MedianFilter2<uint16_t> _medianBaseIn(10);
MedianFilter2<uint16_t> _medianOddIn(10);
MedianFilter2<uint16_t> _medianEvenIn(10);

MedianFilter2<uint16_t> _medianBaseOut(10);
MedianFilter2<uint16_t> _medianOddOut(10);
MedianFilter2<uint16_t> _medianEvenOut(10);

bool enableLog = true;
unsigned long lastLog = 0;

void setup()
{

  //setup SPI
  pinMode(BASE_CS, OUTPUT);
  pinMode(CLUTCH_CS, OUTPUT);

  digitalWrite(BASE_CS, HIGH);
  digitalWrite(CLUTCH_CS, HIGH);

  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
  SPI.setClockDivider(SPI_CLOCK_DIV8);

  setChipValue(BASE_CS, BASE_NUM, 0);
  setChipValue(CLUTCH_CS, BASE_NUM, ODD);
  setChipValue(CLUTCH_CS, BASE_NUM, EVEN);

  Serial.begin(115200); // start it up

  // read value pins
  pinMode(BASE_IN, INPUT_PULLDOWN);
  pinMode(ODD_IN, INPUT_PULLDOWN);
  pinMode(EVEN_IN, INPUT_PULLDOWN);

  pinMode(BASE_OUT, INPUT_PULLDOWN);
  pinMode(ODD_OUT, INPUT_PULLDOWN);
  pinMode(EVEN_OUT, INPUT_PULLDOWN);

  Serial.println("Clutch Control active");

  if (enableLog == true){
    Serial.println("BASE_IN,BASE_OUT,ODD_IN,ODD_OUT,EVEN_IN,EVEN_OUT,");
  }
  
}

void loop()
{

  for (int i = 1; i <= 10; ++i) {
    //remember this is a 240Mhz processor, we need some delay
    delayMicroseconds(100); //in other words, give us an average over last 1Ms
    _medianBaseIn.AddValue(analogRead(BASE_IN));
    _medianOddIn.AddValue(analogRead(ODD_IN));
    _medianEvenIn.AddValue(analogRead(EVEN_IN));

    _medianBaseOut.AddValue(analogRead(BASE_OUT));
    _medianOddOut.AddValue(analogRead(ODD_OUT));
    _medianEvenOut.AddValue(analogRead(EVEN_OUT));

  }

  uint16_t _base = medianBaseIn.AddValue(_medianBaseIn.GetFiltered());
  uint16_t _odd = medianOddIn.AddValue(_medianOddIn.GetFiltered());
  uint16_t _even = medianEvenIn.AddValue(_medianEvenIn.GetFiltered());

  uint16_t _baseOut = medianBaseOut.AddValue(_medianBaseOut.GetFiltered());
  uint16_t _oddOut = medianOddOut.AddValue(_medianOddOut.GetFiltered());
  uint16_t _evenOut = medianEvenOut.AddValue(_medianEvenOut.GetFiltered());


  if (enableLog == true){

    if (micros() > lastLog + 100000){//log ever 100ms
      String report = "";

      //1. scale is -11bar to 80bar, should be calibrated based on ZERO
      //2. median on ESP32s2 is 1.1 V, but we are here for changes not to precise readings
      //3. adding extra ZERO so we don't have to deal with float/decimals
      //BASE
      report += map(_base, 0, 8191, -110, 800);
      report += ",";

      report += map(_baseOut, 0, 8191, -110, 800);
      report += ",";

      //ODD
      report += map(_odd, 0, 8191, -110, 800);
      report += ",";

      report += map(_oddOut, 0, 8191, -110, 800);
      report += ",";

      //EVEN
      report += map(_even, 0, 8191, -110, 800);
      report += ",";

      report += map(_evenOut, 0, 8191, -110, 800);
      report += ",";
      
      Serial.println(report);

      lastLog = micros();
    }

  }

  delay(1);//for shits and giggles, give it a breather.. 1Ms
}

void setChipValue(int cs, uint16_t value, int chan)
{

  digitalWrite(cs, LOW);

  uint16_t data = 0x1000;
  if (chan == 1) { data |= 0x8000; }
  // if (_buffered)    data |= 0x4000;
  // if (_gain == 1)   data |= 0x2000;
  data |= 0x2000;
  data |= (value << 2);

  SPI.transfer((uint8_t)(data >> 8));
  SPI.transfer((uint8_t)(data & 0xFF));
  // delayMicroseconds(15);

  digitalWrite(cs, HIGH);
}
