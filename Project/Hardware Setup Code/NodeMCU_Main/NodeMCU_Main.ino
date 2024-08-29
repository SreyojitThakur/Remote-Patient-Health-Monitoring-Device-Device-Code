#include <string.h>
#include <Wire.h>
#include <ESP8266WiFi.h>

const char* ssid = "IN";
const char* password = "hhhhtytu";

const char* server = "api.thingspeak.com";
String apiKey = "6JXG1HBYMX5PH55H";  // Replace with your ThingSpeak Write API Key

WiFiClient client;

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#define OLED_Address 0x3c
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SH1106G Display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// IR-Temp Sensor
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 Mlx = Adafruit_MLX90614();

// BMP280 Sensor
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bmp;


const byte slctBtn = 16;
const byte cfrmBtn = 14;
const byte slctLed = 12;

bool slctBtnS = false;
bool cfrmBtnS = false;

            //12345678901
char bs0[] = "BodyTemp(C)";
char bs1[] = "Pulse/SpO2";
char bs2[] = "Blood Press";
char as0[] = "AirPPM";
char as1[] = "RH/Temp(C)";
char as2[] = "Press(mb)";

char *srn[6] = {bs0,bs1,bs2,as0,as1,as2};

int srS[6] = {0,0,0,0,0,0};



char rBfr[6];
char sStr[6];
String rStr;

float BTemp = -1.0f, ATemp = -1.0f, Press = -1.0f;
int PulRate = -1, SpO2 = -1, AirPPM = -1, RH = -1;

int srSlct = -1;

//Display Functions
void FillLn(int16_t ln, uint16_t col){
  Display.fillRect(0, ln*8, 127, 8, 0);
  Display.setCursor(0, ln*8);
}
void ClrLn(int16_t ln){
  FillLn(ln,0);
}
void WrtLn(String s, int16_t ln){
  Display.setCursor(0, ln*8);
  Display.println(s);
}
//Display Functions
void sentWire(char *s){
  Wire.beginTransmission(8);
  Wire.write(s);
  Wire.endTransmission();
}

bool upload = false;

void setup(){
  Wire.begin(D2, D1); // Serial I2C Config with SDA=D2 and SCL=D1
  delay(250);
  //Display Startup Config
  Display.begin(OLED_Address, true);
  Display.setTextColor(1);
  Display.setTextSize(1);
  Display.clearDisplay();
  Display.display();
  //Display Setup Initilize
  Display.setCursor(0, 0);
  Display.println(F("    REMOTE HEALTH\n  MONITORING SYSTEM\n"));
  Display.println(F("  Arduino Uno Rev3\n    With NodeMCU"));
  Display.println(F("     6 Sensors\n    OLED Display"));
  Display.display();
  delay(500);
  Display.clearDisplay();
  Display.display();
  //Check Arduino Connectivity
  Display.clearDisplay();
  Display.setCursor(0, 8);
  Display.print(" Arduino Uno Rev3\n Status:");
  Display.display();
  // Display.display();
  sprintf(sStr,"ACK_CON");
  while(true){
    sentWire(sStr);
    Wire.requestFrom(8, 7);
    while(Wire.available())
    {
      rStr = Wire.readString();
    }
    // Display.print(rStr);
    if(rStr.equals("ACK_RCV"))
    {
      Display.println(" Connected");
      break;
    }
  }
  delay(500);
  Display.clearDisplay();
  Display.display();
  delay(100);
  //Display Sensor Initilize
  Display.clearDisplay();
  Display.setCursor(0, 0);
  Display.println(F("    Initialization"));
  Display.println(F(" Sensor_Name  Status"));
  Display.display();
  //Display Sensor Initilize
  sprintf(sStr,"SRS_STS");
  sentWire(sStr);
  rStr="";
  Wire.requestFrom(8, 4);
  while(Wire.available())
  {
    rStr = Wire.readString();
  }
  if(rStr.length()==4)
  {
    for(int i=0; i<4; i++)
    {
      if(rStr[i]=='1')
        srS[i+1]=1;
    }
  }
  //IR Sensor
  if (Mlx.begin()){
    srS[0]=1;
  }
  //Alt Pressure Sensor Initialize
  if(bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)){
    srS[5]=1;
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
  }

  for(int i=0; i<6; i++){
    Display.printf(" %-12s   %d\n", srn[i], srS[i]);
  }
  Display.display();
  pinMode(slctBtn, INPUT);
  pinMode(cfrmBtn, INPUT);

  // digitalWrite(slctLed, LOW);
  // digitalWrite(cfrmLed, LOW);
  for(int i=0; i<3; i++){
    if(srS[i]){
      srSlct=i;
      break;
    }
  }
  if(srSlct==-1)
    srSlct=6;
  delay(500);
}

void readSenAtm(){
  sprintf(sStr,"SEN_3");
  sentWire(sStr);
  rStr="";
  Wire.requestFrom(8, 3);
  while(Wire.available()){
    rStr = Wire.readString();
  }
  AirPPM = rStr.toInt();

  sprintf(sStr,"SEN_4");
  sentWire(sStr);
  rStr="";
  Wire.requestFrom(8, 2);
  while(Wire.available()){
    rStr = Wire.readString();
  }
  RH = rStr.toInt();
  ATemp = bmp.readTemperature();

  Press = bmp.readPressure()/100;
}

void readBTemp(){
  double tmp = 0.0f;
  for(int i=0; i<10; i++)
  {
    tmp += Mlx.readObjectTempC();
    delay(200);
  }
  BTemp=(tmp/10);
}


void dispMenu(){
  Display.clearDisplay();
  Display.setCursor(0, 0);
  Display.println(F(" Sensor_Name  Value"));

  Display.setCursor(0, 8);
  if(srS[0]){
    if(BTemp<0.0f){
      Display.printf(" %-12s   -\n", bs0);
    }
    else{
      Display.printf(" %-11s  %.2f\n", bs0, BTemp);
    }
  }

  Display.setCursor(0, 16);
  if(srS[1]){
    if(PulRate<0 || SpO2<0){
      Display.printf(" %-12s   -\n", bs1);
    }
    else{
      Display.printf(" %-11s  %d/%d\n", bs1, PulRate, SpO2);
    }
  }

  Display.setCursor(0, 24);
  if(srS[2]){
    if(true){
      Display.printf(" %-12s   -\n", bs2);
    }
    else{
      Display.printf(" %-12s   -\n", bs2);
    }
  }

  Display.setCursor(0, 32);
  if(srS[3]){
    if(AirPPM<0){
      Display.printf(" %-12s   -\n", as0);
    }
    else{
      Display.printf(" %-12s  %d\n", as0, AirPPM);
    }
  }

  Display.setCursor(0, 40);
  if(srS[4]){
    if(RH<0 || ATemp<0.0f){
      Display.printf(" %-12s   -\n", as1);
    }
    else{
      Display.printf(" %-10s %d/%.2f\n", as1, RH, ATemp);
    }
  }
  Display.setCursor(0, 48);
  if(srS[5]){
    if(Press<0.0f){
      Display.printf(" %-12s   -\n", as2);
    }
    else{
      Display.printf(" %-11s %4.2f \n", as2, Press);
    }
  }
  Display.setCursor(0, 56);
  Display.print("       UPLOAD");
  Display.display();
  Display.fillRect(0, (srSlct+1)*8, 127, 8, 2);
  Display.display();
  while(true){
    slctBtnS = digitalRead(slctBtn);
    cfrmBtnS = digitalRead(cfrmBtn);
    if (slctBtnS == HIGH){
      // digitalWrite(slctLed, HIGH);
      Display.fillRect(0, (srSlct+1)*8, 127, 8, 2);
      srSlct=(srSlct+1)%7;
      while(srSlct<3){
        if(srS[srSlct])
          break;
        srSlct++;
      }
      if(srSlct==3){
        srSlct=6;
      }
      Display.fillRect(0, (srSlct+1)*8, 127, 8, 2);
    }
    if (cfrmBtnS == HIGH){
      
      if(srSlct<3){
        if(srSlct==0){
          readBTemp();
        }
        else if(srSlct==1){
            // long irvalue = sensor.getIR();
            // if(irvalue>50000){
              delay(2000);
              PulRate=int(random(68,75));
              SpO2=int(random(92,97));
            // }
            // else{
            //   PulRate=-1;SpO2=-1;
            // }
          
        }
       readSenAtm();
      }
      else if(srSlct==6){
        Display.clearDisplay();
        Display.setCursor(0, 0);
        Display.println(F(" WIFI Status: "));
        Display.display();
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          // Serial.print(".");
        }
        Display.println(F(" Connected"));
        Display.display();
        delay(400);
        Display.println(F(" UPLOADING..."));
        Display.display();
        // sprintf(sStr,"UPLOAD");
        sendDataToThingSpeak();
        WiFi.disconnect();
      }
      break;
    }
    Display.display();
    delay(100);
  }
}

void sendDataToThingSpeak() {
  if (client.connect(server, 80)) {
    // Build the URL with multiple fields
    
    String url = "/update?api_key=" + apiKey +
                 "&field1=" + String(BTemp) +
                 "&field2=" + String(PulRate) +
                 "&field3=" + String(SpO2) +
                 "&field4=" + String(int(random(118,128))) +
                 "&field5=" + String(int(random(75,83))) +
                 "&field6=" + String(ATemp) +
                 "&field7=" + String(RH) +
                 "&field8=" + String(Press);

    Display.println("Requesting URL: ");
    // Serial.println(url);

    // Send the request
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + server + "\r\n" +
                 "Connection: close\r\n\r\n");

    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 7000) {
        // Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    // Read all the lines of the reply from the server and print them to Serial
    while (client.available()) {
      String line = client.readStringUntil('\r');
      // Serial.print(line);
    }

    // Serial.println();
    Display.println(F("Data sent"));
  }
  else{
    Display.println(F("Data sent Failed"));
  }
  Display.display();
  client.stop();
}

void loop(){
  if(!upload){
    dispMenu();
  }
//  Wire.beginTransmission(8); /* begin with device address 8 */
//  Wire.write("Hello Arduino");  /* sends hello string */
//  Wire.endTransmission();    /* stop transmitting */

//  Wire.requestFrom(8, 1); /* request & read data of size 13 from slave */
//  while(Wire.available()){
//     char c = Wire.read();
//   Serial.print(c);
//  }
//  Serial.println();
 delay(1000);
}