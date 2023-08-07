#include <Arduino.h>
#include <SPI.h>
#include "MedianFilterLib2.h"

#define ODD       0
#define EVEN      1

#define BASE_CS   34
#define CLUTCH_CS 26                      //IF you use FLASH memory or PSRAM, you can not use this GPIO (thanks for pointing it out Eric)

#define BASE_IN   1
#define ODD_IN    2
#define EVEN_IN   3

#define BASE_OUT  6
#define ODD_OUT   7
#define EVEN_OUT  8

void setChipValue(int cs, uint16_t value, int chan);

uint16_t _baseT = 1020;                   //current state of DAC !!!
uint16_t _oddT  = 1020;
uint16_t _evenT = 1020;

uint16_t _baseTarget   = 960;             //target to reach on DAC, lower the value = lower the output based on INPUT
uint16_t _clutchTarget = 910;             //remember its not linear, so 900 will give you about 27% more clutch pressure, not 10%
uint16_t _gain         = 5;               //how fast we get to target

uint16_t _clutchGate = 2700;              //when we should start adding to pressure, it represents 3.333voltage -> 0 - 8191
uint16_t _baseGate   = 3700;

uint16_t BASE_NUM = 1020;                 //BASE of our DAC, always set to 1020 even tho 1024 is max

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

//LOG
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

  //LOG
  if (enableLog == true){
    Serial.println("BASE_T,BASE_IN,BASE_OUT,ODD_T,ODD_IN,ODD_OUT,EVEN_T,EVEN_IN,EVEN_OUT,");
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

  //MAGIC
  if (_odd > _clutchGate){
    if (_oddT > _clutchTarget){
      _oddT = _oddT - _gain;
    }
  } else {
    _oddT = BASE_NUM;
  }

  if (_even > _clutchGate){
    if (_evenT > _clutchTarget){
      _evenT = _evenT - _gain;
    }
  } else {
    _evenT = BASE_NUM;
  }

  //base should always follow clutches and not be seperate.. see Apendix 1.1
  if (_base > _baseGate || _oddT != BASE_NUM || _evenT != BASE_NUM) {
    if (_baseT > _baseTarget){
      _baseT = _baseT - _gain;
    }
  } else {
    _baseT = BASE_NUM;
  }

  //set chip values
  setChipValue(BASE_CS, _baseT, 0);
  setChipValue(CLUTCH_CS, _oddT, ODD);
  setChipValue(CLUTCH_CS, _evenT, EVEN);


  if (enableLog == true){

    if (micros() > lastLog + 50000){//log ever 50ms
      String report = "";

      //BASE
      report += _baseT;
      report += ",";

      report += map(_base, 0, 8191, -110, 800);   //scale is -11psi to 80psi... adding extra ZERO so we don't have to deal with float/decimals
      report += ",";

      report += map(_baseOut, 0, 8191, -110, 800);
      report += ",";

        //ODD
      report += _oddT;
      report += ",";

      report += map(_odd, 0, 8191, -110, 800);
      report += ",";

      report += map(_oddOut, 0, 8191, -110, 800);
      report += ",";

      //EVEN
      report += _evenT;
      report += ",";

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
