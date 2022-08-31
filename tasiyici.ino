//bmp 180
#include <Wire.h>
#include<stdlib.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;
//espnow
#include <esp_now.h> // https://github.com/espressif/esp-idf
#include <WiFi.h>
//gps
#include <TinyGPS++.h> // https://github.com/mikalhart/TinyGPSPlus
TinyGPSPlus gps;
HardwareSerial GPS(2);

//ham veriler
double gps2lat = 0.000;
double gps2long = 0.000;
float gps2alt = 0.000;
float yukseklik = 0;
float basinc;
// alıcı MAC Address
uint8_t broadcastAddress[] = {0x30, 0xC6, 0xF7, 0x2F, 0x9A, 0x34};

// Data structure
typedef struct struct_message {
  double a;
  double b;
  float c;
  float d;
  float e;
} struct_message;
// data object
struct_message myData;
// Peer info
esp_now_peer_info_t peerInfo;
// Callback function called when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Wire.begin(21, 22);


  GPS.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX

  //115200 seri haberleşme
  Serial.begin(115200);
  // gps serial
  //Serial1.begin(9600, SERIAL_8N1, 34, 12);   //17-TX 18-RX
  // wifi modu
  WiFi.mode(WIFI_STA);
  if (!bmp.begin()) {
    Serial.println("BMP Hatası.");
  }
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW Hatası.");
    return;
  }
  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("MAC Bağlantı Hatası");
    return;
  }
}

void loop() {
 
  // put your main code here, to run repeatedly:
  gps2lat = gps.location.lat();
  myData.a = gps2lat;
  gps2long = gps.location.lng();
  myData.b = gps2long;
  gps2alt = gps.altitude.feet() / 3.2808;
  myData.c = gps2alt;
  yukseklik = bmp.readAltitude(); //readSealevelPressure();
  // yukseklik=random(24,32);
  myData.d = yukseklik;
  basinc = bmp.readPressure();
  //basinc=random(24,32);
  myData.e = basinc;
  Serial.println(basinc);
  Serial.println(yukseklik);
  Serial.println(gps2lat);
  Serial.println(gps2long);
  Serial.println(gps2alt);


  //veri gönder
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  // veri gönderim konrol
  if (result == ESP_OK) {
    Serial.println("Veri İletildi");
  }
  else {
    esp_now_add_peer(&peerInfo);
  }
  smartDelay(250);
  if (millis() > 5000 && gps.charsProcessed() < 10)
    Serial.println(F("No GPS data received: check wiring"));
}


static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPS.available())
      gps.encode(GPS.read());
  } while (millis() - start < ms);
}
