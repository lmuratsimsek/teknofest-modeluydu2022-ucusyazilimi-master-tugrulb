#include "Arduino.h"
#include "LoRa_E220.h"



#define DESTINATION_ADDL 0xA0
#define DESTINATION_ADDH 0x9B
#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 5); //D2-D1
//LoRa_E220 e220ttl(0, 2, 14, 13,12 );
LoRa_E220 e220ttl(&mySerial, 14, 13, 12 );
//C:\Users\lgg42\AppData\Local\Arduino15\packages\esp8266\hardware\esp8266\3.0.2\libraries\SoftwareSerial\src
// buff capacity değiştirildi.

struct Telemetri {
  char takimNo[15];
  byte paketNo[4];
  char gondermeSaati[20];
  byte basinc1 [10];
  byte basinc2 [10];
  byte yukseklik1[7];
  byte yukseklik2[7];
  byte fark[6];
  byte inisHizi[5];
  byte sicaklik[5];
  byte gerilim[5];
  char gps1lat[8];
  char gps1lng[8];
  char gps1alt[8];
  char gps2lat[8];
  char gps2lng[8];
  char gps2alt[9];
  byte uydustatusu[4];
  byte pitch[8];
  byte roll[8];
  byte yaw [8];
  byte donusSayisi [8];
  byte VideoAktarimBilgisi [4];

} telemetri;

struct Commands {
  char servodurum[2] = "1";
  char tahrik[2] = "1";
  char videoaktarim[2] = "0";
  char buzzer[2] = "0";
} commands;

void setup() {

  Serial.begin(9600);

  e220ttl.begin();
  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  configuration.CHAN = 23;
  configuration.SPED.airDataRate = AIR_DATA_RATE_111_625;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);
  delay(500);
}

void loop() {
  while (e220ttl.available()  > 1) {
    ResponseStructContainer rsc = e220ttl.receiveMessage(sizeof(Telemetri));
    struct Telemetri telemetri = *(Telemetri*) rsc.data;
    Serial.print(telemetri.takimNo);
    Serial.print(",");
    Serial.print(*(int*)(telemetri.paketNo));
    Serial.print(",");
    Serial.print(telemetri.gondermeSaati);
    Serial.print(",");
    Serial.print(*(float*)(telemetri.basinc1));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.basinc2));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.yukseklik1));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.yukseklik2));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.fark));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.inisHizi));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.sicaklik));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.gerilim));
    Serial.print(",");
    Serial.print(telemetri.gps1lat);
    Serial.print(",");
    Serial.print(telemetri.gps1lng);
    Serial.print(",");
    Serial.print(telemetri.gps1alt);
    Serial.print(",");
    Serial.print(telemetri.gps2lat);
    Serial.print(",");
    Serial.print(telemetri.gps2lng);
    Serial.print(",");
    Serial.print(telemetri.gps2alt);
    Serial.print(",");
    Serial.print(*(int*)(telemetri.uydustatusu));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.pitch));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.roll));
    Serial.print(",");
    Serial.print(*(float*)(telemetri.yaw));
    Serial.print(",");
    Serial.print(*(int*)(telemetri.donusSayisi));
    Serial.print(",");
    Serial.print(*(int*)(telemetri.VideoAktarimBilgisi));
    Serial.println("");
    rsc.close();
    char c = Serial.read();

    if (Serial.available() >= 1)
    {
      char c = Serial.read ();
      if (c == '1') { //Default Values
        commands.servodurum[0] = c;
        commands.tahrik[0] = c;
        commands.videoaktarim[0] = '0';
      }
      if (c == '2') {
        commands.servodurum[0] = c; //Servo Kapat
      }
      if (c == '3') {
        commands.servodurum[0] = c; //Servo Aç
      }
      if (c == '4') {
        commands.tahrik[0]  = c; //500 RPM
      }
      if (c == '5') {
        commands.tahrik[0]  = c; //1000RPM
      }
      if (c == '6') {
        commands.tahrik[0]  = c; //1500RPM
      }
      if (c == '7') {
        commands.videoaktarim[0] = c; //Video Aktarım bir
      }
      if (c == '8') {
        commands.videoaktarim[0] = '0';//Video Aktarım SIFIR
      }
      if (c == '9') {
        commands.buzzer[0] = '1';//BUZZER SIFIR
      }
      if (c == '0') {
        commands.buzzer[0] = '0';//BUZZER Bir
      }
      
    }
    e220ttl.sendFixedMessage(DESTINATION_ADDL, DESTINATION_ADDH, 23, &commands, sizeof(Commands));
    //commands.tahrik[0] = '1';

  }










}
