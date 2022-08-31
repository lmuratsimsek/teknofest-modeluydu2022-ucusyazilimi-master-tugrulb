//SD KART
#include <SD.h>
#include <SPI.h>
const int chipSelect = BUILTIN_SDCARD;
// Servo
#include<Servo.h>
Servo servo1;
Servo m1;

//***********************************************
//          Lora
//***********************************************
#include "Arduino.h"
#include "LoRa_E220.h"
#include <SoftwareSerial.h>
#define HWSERIAL Serial1
LoRa_E220 e220ttl(&HWSERIAL, 3, 5, 6);
#define DESTINATION_ADDL 0x93 // adress
#define DESTINATION_ADDH 0xEA
//***********************************************
//          gps
//***********************************************
#include <TinyGPSPlus.h>
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(15, 14);
//*****   PAKET NUMARASI   ************
#include <EEPROM.h>
//*****   SAAT  ************
#include "RTClib.h"
RTC_DS1307 rtc;
//*****   BASINÇ,SICAKLIK  ************
#include <Wire.h>
#include "DFRobot_BMP388.h"
#include "DFRobot_BMP388_I2C.h"
#include "Wire.h"
#include "SPI.h"
#include "math.h"
#include "bmp3_defs.h"
#include <DFRobot_BMX160.h>
#include <Adafruit_INA219.h>
DFRobot_BMX160 bmx160;
DFRobot_BMP388_I2C bmp388;
Adafruit_INA219 ina219;
#define  M_PI 3.14159265358979323846
//*****   ESP-TEENSY ONE WIRE ************
SoftwareSerial espReceiveSerial(7, 8);
// ESP pinleri 16 / 17 pinler.
// Teensy 7 - Rx == Esp 17 - Tx
// Teensy 8 - Tx == ESP 16 - Rx
// Teensy GND  == ESP GND
typedef struct struct_message {
  double a ; // GPS2LAT
  double b ; // GPS2LONG
  float c ; // GPS2ALT
  float d ; // YUKSELİK2
  float e; // BASINC2
} struct_message;
struct_message myData; // Object of struct
int size_struct = sizeof(struct struct_message);
bool receive(struct_message* table)
{ //if komutunu sonradan ekledim. kod yavaş
  if (espReceiveSerial.available() > 6) {
    return (espReceiveSerial.readBytes((char*)table, sizeof(struct_message)) == sizeof(struct_message));
  }

}
struct Commands {
  char servodurum[2];
  char tahrik[2];
  char videoaktarim[2];
  char buzzer[2];
} commands;
struct Telemetri  {
  char takimNo[15] = "318842";    //*
  byte paketNo[4];                //*
  char gondermeSaati[20];         //*
  byte basinc1 [10];              //*
  byte basinc2 [10];              //*
  byte yukseklik1[7];             //*
  byte yukseklik2[7];             //*
  byte fark[6];                   //*
  byte inisHizi[5];
  byte sicaklik[5];               //*
  byte gerilim[5];                //*
  char gps1lat[8];
  char gps1lng[8];
  char gps1alt[8];
  char gps2lat[8];                //*
  char gps2lng[8];                //*
  char gps2alt[9];                //*
  byte uydustatusu[4];
  byte pitch[8];                  //*
  byte roll[8];                   //*
  byte yaw [8];                   //*
  byte donusSayisi [8];           //*
  byte VideoAktarimBilgisi [4];

} telemetri;
//sd
File myFile;
// 1 HZ
unsigned long prevMillis = 0;
unsigned long currentMillis = 0;
const long interval = 1000;
void setup() {
  Serial.begin(9600);
  delay(500);
  e220ttl.begin();
  m1.attach(33);
  //arm(m1);
  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);
  configuration.SPED.airDataRate = AIR_DATA_RATE_111_625;
  configuration.CHAN = 23;
  ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);


  bmp388.begin();
  espReceiveSerial.begin(9600);
  bmp388.set_iic_addr(BMP3_I2C_ADDR_PRIM);

  if (bmx160.begin() != true) {
    Serial.println("Bmx160 Hatası");

  }
  if (bmp388.begin() != true) {
    Serial.println("BMP");
  }
  if (! rtc.begin()) {
    Serial.println("RTC Hatası");
    Serial.flush();
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC Reset!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  /*
  if (! ina219.begin()) {
    Serial.println("INA219 hatası");
    while (1) {
      delay(10);
    }
  }
  */
  delay(500);
  servo1.attach(23);
  servo1.write(commands.servodurum[0]);
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("SD Card initialized.");
  SD.remove("telemetry.txt");
  myFile = SD.open("telemetry.txt", FILE_WRITE);
  if (myFile) {
    Serial.println("File opened ok");
    //headings of data
    myFile.println("<TAKIM NO>,<PAKET NUMARASI>,<GÖNDERME SAATİ>,<BASINÇ1>,<BASINÇ2>,<YÜKSEKLİK1>,< YÜKSEKLİK2>,<IRTIFA FARKI>,<İNİŞ HIZI>, <SICAKLIK>,<PİL GERİLİMİ>,<GPS1 LATITUDE>,<GPS1 LONGITUDE>,<GPS1 ALTITUDE>,<GPS2 LATITUDE>,<GPS2 LONGITUDE>,<GPS2 ALTITUDE>,<UYDU STATÜSÜ>,<PITCH>,<ROLL>,<YAW>,<DÖNÜŞ SAYISI>,<VİDEO AKTARIM BİLGİSİ>");
  }
  myFile.close();
  pinMode(13, OUTPUT);
  pinMode(9, OUTPUT); //buzer
  //gps
  ss.begin(GPSBaud);
  



}
uint32_t timer = millis();
#define GPSECHO false
float deger = 0.00;
bool mesajgeldiMi = false;
int donus = 0;
//FIRCASIZ
unsigned long prevMillisB = 0;
unsigned long currentMillisB = 0;
const long intervalB = 1000;
String readstring;
void loop() {
  

  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
  float prevPos = getYaw();
  receive(&myData);
  while (e220ttl.available() > 1) {
    mesajgeldiMi = true;
    // Gelen mesaj okunuyor
    ResponseStructContainer rsc = e220ttl.receiveMessage(sizeof(Commands));
    struct Commands commands = *(Commands*) rsc.data;
    if (commands.servodurum[0] == '1') {
      m1.write(1200);
    }
    if (commands.servodurum[0] == '2') {
      //Servo Kapat
      Serial.println("Servo Kapatıldı");
      servo1.write(0);
    }
    if (commands.servodurum[0] == '3') {
      //Servo Aç
      Serial.println("Servo Açıldı");
      servo1.write(180);
    }
    if (commands.tahrik[0] == '4') {
      m1.write(1300);
    }
    if (commands.tahrik[0] == '5') {
      m1.write(1500);
    }
    if (commands.tahrik[0] == '6') {
      m1.write(1900);
    }
    if (commands.videoaktarim[0] == '7') {
      Serial.println("Video aktarımı başladı.");
      *(int*)(telemetri.VideoAktarimBilgisi) = 1;
    }
    if (commands.videoaktarim[0] == '0') {
      *(int*)(telemetri.VideoAktarimBilgisi) = 0;
    }
    if (commands.buzzer[0] == '0') {
      digitalWrite(9, LOW);
    }
    if (commands.buzzer[0] == '1') {
      digitalWrite(9, HIGH);
    }
    rsc.close();
  }


  currentMillis = millis();
  if (currentMillis - prevMillis > interval) {

   
 
    digitalWrite(13, HIGH);
    //Paket No
  

    *(int*)(telemetri.paketNo) = getPaketNo();
    e220ttl.sendFixedMessage(DESTINATION_ADDL, DESTINATION_ADDH, 23, &telemetri, sizeof(Telemetri));
    prevMillis = currentMillis;
    
    myFile = SD.open("telemetry.txt", FILE_WRITE);
    if (myFile) {
      myFile.print(telemetri.takimNo);
      myFile.print(",");
      myFile.print(*(int*)(telemetri.paketNo));
      myFile.print(",");
      myFile.print(telemetri.gondermeSaati);
      myFile.print(",");
      myFile.print(*(float*)(telemetri.basinc1));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.basinc2));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.yukseklik1));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.yukseklik2));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.fark));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.inisHizi));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.sicaklik));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.gerilim));
      myFile.print(",");
      myFile.print(telemetri.gps1lat);
      myFile.print(",");
      myFile.print(telemetri.gps1lng);
      myFile.print(",");
      myFile.print(telemetri.gps1alt);
      myFile.print(",");
      myFile.print(telemetri.gps2lat);
      myFile.print(",");
      myFile.print(telemetri.gps2lng);
      myFile.print(",");
      myFile.print(telemetri.gps2alt);
      myFile.print(",");
      myFile.print(*(int*)(telemetri.uydustatusu));
      myFile.print(",");
      myFile.print( *(float*)(telemetri.pitch));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.roll));
      myFile.print(",");
      myFile.print(*(float*)(telemetri.yaw));
      myFile.print(",");
      myFile.print(*(int*)(telemetri.donusSayisi));
      myFile.print(",");
      myFile.print(*(int*)(telemetri.VideoAktarimBilgisi));
      myFile.println("");
    }
    myFile.close();
    digitalWrite(13, LOW);


  }
  unsigned long start = millis();
  if (millis() - start < 1000)
  {
    while (ss.available())
      gps.encode(ss.read());
  }

  //Gonderme Saati
  DateTime now = rtc.now();
  String yil = String(now.year());
  String ay = String(now.month());
  String gun = String(now.day());
  String saat = String(now.hour());
  String dakika = String(now.minute());
  String saniye = String(now.second());
  String son = (yil + "" + ay + "" + gun + "" + saat + "" + dakika + "" + saniye);
  son.toCharArray(telemetri.gondermeSaati , 15);

  //Basınç1
  *(float*)(telemetri.basinc1) = bmp388.readPressure();
  //Basınç2
  *(float*)(telemetri.basinc2) = myData.e;
  //Yukseklik1
  float deger = 0.00;
  deger = bmp388.readAltitude();
  *(float*)(telemetri.yukseklik1) = deger;
  //Yukseklik2
  *(float*)(telemetri.yukseklik2) = myData.d;

  //Fark
  *(float*)(telemetri.fark) = deger - myData.d;
  // İnis Hizi
  *(float*)(telemetri.inisHizi) = gps.speed.kmph(); // 1 byte arttır
  
  //Sicaklik
  float deger2 = 0.00;
  deger2 = bmp388.readTemperature();
  *(float*)(telemetri.sicaklik) = deger2;
  //Pil Gerilimi
  //*(float*)(telemetri.gerilim) = ina219.getBusVoltage_V();
  *(float*)(telemetri.gerilim) = 13.0;
  //GPS1LAT
  //dtostrf( [doubleVar] , [sizeBeforePoint] , [sizeAfterPoint] , [WhereToStoreIt] )
  double dummy = 0.000;
  dtostrf(gps.location.lat(), 2, 3, telemetri.gps1lat);
  //GPS1LONG
  dtostrf(gps.location.lng(), 2, 3, telemetri.gps1lng);
  //GPS1ALT
  dtostrf(gps.altitude.meters(), 2, 3, telemetri.gps1alt);
  Serial.println(gps.altitude.meters());
 
  //GPS2LAT
  double gps2latitude = myData.a;
  dtostrf(gps2latitude, 2, 3, telemetri.gps2lat);
  //GPS2LONG
  double gps2longitude = myData.b;
  dtostrf(gps2longitude, 2, 3, telemetri.gps2lng);
  //GPS2ALT
  double gps2altitude = myData.c;
  dtostrf(gps2altitude, 2, 3, telemetri.gps2alt);
  //Uydu statüsü
  int uyduStatu = 3;
  *(int*)(telemetri.uydustatusu) = uyduStatu;
  //Pitch
  *(float*)(telemetri.pitch) = getPitch();
  //roll
  *(float*)(telemetri.roll) = getRoll();
  //yaw
  *(float*)(telemetri.yaw) = getYaw();
  //Donus sayisi
  float afterPos = *(float*)(telemetri.yaw);
  float resultPos  = prevPos - afterPos;
  if (resultPos > 100) {
    donus++;
  }
  if (resultPos < -100) {
    donus++;
  }
  *(int*)(telemetri.donusSayisi) = donus;
  // Video Aktarım Bilgisi
 

  


}

int paketno = 0;
int getPaketNo() {
  //İşlemcinin yeniden baslaması durumunda, paket numarası devamı.
  if (paketno == 0) {
    paketno = EEPROM.read(0);
    return ++paketno;
  } else {
    EEPROM.update(0, paketno);
    return ++paketno;
  }
}
float getPitch() {
  sBmx160SensorData_t Omagn, Ogyro, Oaccel;
  bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
  float pitch;
  pitch = 180 * atan (Oaccel.x / sqrt(Oaccel.y * Oaccel.y + Oaccel.z * Oaccel.z)) / M_PI;
  return pitch;
}
float getRoll() {
  sBmx160SensorData_t Omagn, Ogyro, Oaccel;
  bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
  float roll;
  roll = 180 * atan (Oaccel.y / sqrt(Oaccel.x * Oaccel.x + Oaccel.z * Oaccel.z)) / M_PI;
  return roll;
}
float getYaw() {
  float yaw;
  sBmx160SensorData_t Omagn, Ogyro, Oaccel;
  bmx160.getAllData(&Omagn, &Ogyro, &Oaccel);
  float heading = atan2(Omagn.x, Omagn.y) / 0.0174532925;
  if (heading < 0) {
    heading += 360;
    heading = 360 - heading;
  }
  yaw = heading;
  return yaw;
}

void arm(Servo m) {
  Serial.print("Arming.");
  m.write(0);
  delay(100);
  m.write(1224); //A value at which the motor starts turning
  delay(2000);
  m.write(1024); //A value at which the motor stands still
  delay(3000);
  Serial.println(" Armed!");
}
