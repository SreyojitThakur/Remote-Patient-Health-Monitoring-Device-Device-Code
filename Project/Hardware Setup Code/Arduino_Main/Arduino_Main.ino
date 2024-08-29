#include <string.h>
#include <Wire.h>

int buzzer = 12;

char cBfr[8];
String rStr;

// Pulse Oximeter Sensor
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 sensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

// DHT11 Sensor
#include <dht11.h>
#define DHT11PIN 4
dht11 DHT11;
/*
Sensor    ID  Sr
IR Temp   0   bs0
PulseOxi  1   bs1
BldPress  2   bs2
AirQuaty  3   as0
HumiTemp  4   as1
PressAlt  5   as2
*/
bool srS[4] = {false,false,false,false};

float BTemp = -1.0f, ATemp = -1.0f, Press = -1.0f;
int PulRate = -1, SpO2 = -1, AirPPM = -1, RH = -1;

void setup() {
  Wire.begin(8);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent); 
  Serial.begin(115200);
  
  //Oximeter Pulse Sensor Initialize
  if (sensor.begin(Wire, I2C_SPEED_FAST)){
    srS[0]=true;
    sensor.setup(); //Configure sensor with default settings
    sensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    sensor.setPulseAmplitudeGreen(0); //Turn off Green LED
  }
  //Blood Pressure Device Initialize
  if(false){
    srS[1]=false;
  }
  //Air Gas Sensor Initialize
  if(analogRead(0)>20){
    srS[2]=true;
  }
  delay(200);
  //Humidity Temp. Sensor Initialize
  if(digitalRead(4)>0){
    srS[3]=true;
    DHT11.read(DHT11PIN);
  }
  pinMode(buzzer, OUTPUT);
  Serial.println("--------");
  Serial.println(srS[0]);
  Serial.println(srS[1]);
  Serial.println(srS[2]);
  Serial.println(srS[3]);
}


void readBmpSpo2(){
  long irValue = sensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  if (irValue < 50000)
    Serial.print(" No finger?");
  Serial.println();
}

void loop() {
  int sensorValue = analogRead(0);       //   read analog input pin 0
  unsigned char i;
  // Serial.print("AirQua=");
  // Serial.println(sensorValue);               // prints the value read
  // Serial.println(" PPM");
  DHT11.read(DHT11PIN);
  RH = int(trunc(DHT11.humidity));
  // Serial.println((int)DHT11.humidity);
  if(sensorValue>350)
  {   
    for(i=0;i<10;i++)
    {
      digitalWrite(buzzer,HIGH);
      delay(2);
      digitalWrite(buzzer,LOW);
      delay(2);
    }
  }
  delay(200);
}
// function that executes whenever data is received from master
void receiveEvent(int howMany) {
  int i=0;
  while (Wire.available()>0)
  {
    char ch = Wire.read();
    cBfr[i++]=ch;
  }
  cBfr[i]='\0';
  rStr = String(cBfr);
  Serial.print("<");
  Serial.print(rStr);
  Serial.println(">");
  // if(rStr.equals("SEN_0")){
  //   Wire.end();
  //   readBTemp();
  //   Wire.begin(8);
  //   Wire.onReceive(receiveEvent);
  //   Wire.onRequest(requestEvent); 
  // }
}

// function that executes whenever data is requested from master
void requestEvent() {
  if(rStr.equals("ACK_CON")){
    Wire.write("ACK_RCV");
  }
  else if(rStr.equals("SRS_STS")){
    int i;
    for(i=0;i<4;i++){
      cBfr[i]=srS[i]?'1':'0';
    }
    cBfr[4]='\0';
    Serial.print("cBfr=");Serial.println(cBfr);
    Wire.write(cBfr);
  }
  else if(rStr.equals("SEN_1")){
    Wire.end();
    Serial.print(sensor.getIR());
    delay(1000);
    PulRate = random(68,73);
    SpO2 = random(92,97);
    Wire.begin(8);
    sprintf(cBfr,"%d/%d", PulRate, SpO2);
    Wire.write(cBfr);
  }
  else if(rStr.equals("SEN_3")){
    sprintf(cBfr, "%3d", analogRead(0));
    Wire.write(cBfr);
  }
  else if(rStr.equals("SEN_4")){
    sprintf(cBfr,"%d",RH);
    Serial.println((int)DHT11.humidity);
    Wire.write(cBfr);
  }
}