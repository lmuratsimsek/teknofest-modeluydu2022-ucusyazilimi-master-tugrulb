#include <WiFi.h>              // Built-in
#include <ESPmDNS.h>           // Built-in
#include <AsyncTCP.h>          // https://github.com/me-no-dev/AsyncTCP
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include "esp_system.h"        // Built-in
#include "esp_spi_flash.h"     // Built-in 
#include "esp_wifi_types.h"    // Built-in
#include "esp_bt.h"            // Built-in
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define  FS SD
AsyncWebServer server(80);
// ESPNOW
#include <esp_wifi.h>
#include <esp_now.h>
// ESP-TEENSY
#include <SoftwareSerial.h>
SoftwareSerial espSendSerial(16, 17); // RX - TX
// ESPNOW DATA
typedef struct struct_message {
  double a; //GPS2LAT
  double b; //GPS2LONG
  float c; //GPS2ALT
  float d; //YUKSEKLİK
  float e; // BASİNC
} struct_message;
struct_message myData;
int size_struct = sizeof(struct struct_message); //Struct Boyutu
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) { //Data alma
  memcpy(&myData, incomingData, sizeof(myData));
  delay(100);
}
// ESP-TEENSY SEND
void sendData (const struct_message* table)
{
  
    espSendSerial.write((char*)table, size_struct);
    
  

}
typedef struct
{
  String filename;
  String ftype;
  String fsize;
} fileinfo;
bool videoAktarimBilgisi = false;
String   webpage, MessageLine;
fileinfo Filenames[200]; // Enough for most purposes!
bool     StartupErrors = false;
int      start, downloadtime = 1, uploadtime = 1, downloadsize, uploadsize, downloadrate, uploadrate, numfiles;
const char* ssid          = "TUGRULB";
const char* password      = "123456789";
const char* ServerName    = "fileserver";
void setup() {
  Serial.begin(115200);
  espSendSerial.begin(9600); //Teensy-esp haberleşme
  while (!Serial);
  pinMode(21,OUTPUT);
  digitalWrite(21,HIGH);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("TUGRULB", "123456789");
  IPAddress myIP = WiFi.softAPIP();
  Serial.println("IP Address: " + WiFi.softAPIP().toString());
           /* Task handle. */
  if (!StartMDNSservice(ServerName)) {
    Serial.println("Error starting mDNS Service...");;
    StartupErrors = true;
  }
  if (!SD.begin(5)) {
    Serial.println("Error preparing Filing System...");
    StartupErrors = true;
  }
  // ##################### HOMEPAGE HANDLER ###########################
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Home Page...");
    UploadFileSelect();
    
    //Home(); // Build webpage ready for display
    request->send(200, "text/html", webpage);
  });
  // ##################### DOWNLOAD HANDLER ##########################
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Downloading file...");

    Select_File_For_Function("[DOWNLOAD]", "downloadhandler"); // Build webpage ready for display
    request->send(200, "text/html", webpage);
  });
  // ##################### UPLOAD HANDLERS ###########################
  server.on("/upload", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Uploading file...");

    UploadFileSelect(); // Build webpage ready for display
    request->send(200, "text/html", webpage);
  });
  // Set handler for '/handleupload'
  server.on("/handleupload", HTTP_POST, [](AsyncWebServerRequest * request) {},
  [](AsyncWebServerRequest * request, const String & filename, size_t index, uint8_t *data,
     size_t len, bool final) {
      videoAktarimBilgisi = true;
    handleFileUpload(request, filename, index, data, len, final);
  });
  // ##################### DIR HANDLER ###############################
  server.on("/dir", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("File Directory...");

    Dir(request); // Build webpage ready for display
    request->send(200, "text/html", webpage);
  });
  // ##################### DELETE HANDLER ############################
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("Deleting file...");

    Select_File_For_Function("[DELETE]", "deletehandler"); // Build webpage ready for display
    request->send(200, "text/html", webpage);
  });
  // ##################### NOT FOUND HANDLER #########################
  server.onNotFound(notFound);
  server.begin();  // Start the server
  if (!StartupErrors)
    Serial.println("System started successfully...");
  else
    Serial.println("There were problems starting all services...");
  Directory();     // Update the file list

  if (esp_now_init() != ESP_OK) { //esp-now kontrolü
    Serial.println("ESP-NOW Hatası.");
    return;
  }



}
unsigned long prevMillis = 0;
unsigned long currentMillis = 0;
const long interval = 800;


int period = 1000;
unsigned long time_now = 0;
void loop() {
  if((unsigned long)(millis() - time_now) > period){
        time_now = millis();
        if(!videoAktarimBilgisi){
          sendData(&myData);
        }
    }
 

  esp_now_register_recv_cb(OnDataRecv);
  delay(100);
 




}

//#############################################################################################
void Dir(AsyncWebServerRequest * request) {
  String Fname1, Fname2;
  int index = 0;
  Directory(); // Get a list of the current files on the FS
  webpage  = HTML_Header();
  if (numfiles > 0) {
    webpage += "<table class='center'>";
    webpage += "<tr><th>Type</th><th>File Name</th><th>File Size</th><th class='sp'></th><th>Type</th><th>File Name</th><th>File Size</th></tr>";
    while (index < numfiles) {
      Fname1 = Filenames[index].filename;
      Fname2 = Filenames[index + 1].filename;
      webpage += "<tr>";
      webpage += "<td style = 'width:5%'>" + Filenames[index].ftype + "</td><td style = 'width:25%'>" + Fname1 + "</td><td style = 'width:10%'>" + Filenames[index].fsize + "</td>";
      webpage += "<td class='sp'></td>";
      if (index < numfiles - 1) {
        webpage += "<td style = 'width:5%'>" + Filenames[index + 1].ftype + "</td><td style = 'width:25%'>" + Fname2 + "</td><td style = 'width:10%'>" + Filenames[index + 1].fsize + "</td>";
      }
      webpage += "</tr>";
      index = index + 2;
    }
    webpage += "</table>";
    webpage += "<p style='background-color:yellow;'><b>" + MessageLine + "</b></p>";
    MessageLine = "";
  }
  else
  {
    webpage += "<h2> </h2>";
  }
  request->send(200, "text/html", webpage);
}
//#############################################################################################
void Directory() {
  numfiles  = 0; // Reset number of FS files counter
  File root = SD.open("/");
  if (root) {
    root.rewindDirectory();
    File file = root.openNextFile();
    while (file) { // Now get all the filenames, file types and sizes
      Filenames[numfiles].filename = (String(file.name()).startsWith("/") ? String(file.name()).substring(1) : file.name());
      Filenames[numfiles].ftype    = (file.isDirectory() ? "Dir" : "File");
      Filenames[numfiles].fsize    = ConvBinUnits(file.size(), 1);
      file = root.openNextFile();
      numfiles++;
    }
    root.close();
  }
}

//#############################################################################################
void handleFileUpload(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    String file = filename;
    if (!filename.startsWith("/")) file = "/" + filename;
    request->_tempFile = SD.open(file, FILE_WRITE );
    if (!request->_tempFile) Serial.println("Error creating file for upload...");
    start = millis();
  }

  if (request->_tempFile) {
    if (len) {
      request->_tempFile.write(data, len); // Chunked data
      Serial.println("Transferred : " + String(len) + " Bytes");
      
    }
    if (final) {
      uploadsize = request->_tempFile.size();
      request->_tempFile.close();
      uploadtime = millis() - start;
      //request->redirect("/home");
      //videoAktarimBilgisi = false;
      //digitalWrite(21,LOW);
      videoAktarimBilgisi = false;
      
    }
  
    
  }
  
  

}
//#############################################################################################
void File_Delete() {
  SelectInput("Select a File to Delete", "handledelete", "filename");
}
//#############################################################################################
void Handle_File_Delete(String filename) { // Delete the file
  webpage = HTML_Header();
  if (!filename.startsWith("/")) filename = "/" + filename;
  File dataFile = SD.open(filename, FILE_READ); // Now read FS to see if file exists
  if (dataFile) {  // It does so delete it
    SD.remove(filename);
    webpage += "<h3>File '" + filename.substring(1) + "' has been deleted</h3>";
    webpage += "<a href='/dir'>[Enter]</a><br><br>";
  }
  else
  {
    webpage += "<h3>File [ " + filename + " ] does not exist</h3>";
    webpage += "<a href='/dir'>[Enter]</a><br><br>";
  }

}
//#############################################################################################
void UploadFileSelect() {
  webpage = HTML_Header();
  webpage += "<form method = 'POST' action = '/handleupload' enctype='multipart/form-data'>";
  webpage += "<input type='file' name='filename'><br><br>";
  webpage += "<input type='submit' value='Dosya Aktarımını Başlat'>";
  webpage += "</form>";
}
String processor(const String& var) {
  if (var == "HELLO_FROM_TEMPLATE") return F("Hello world!");
  return String();
}
void notFound(AsyncWebServerRequest *request) { // Process selected file types
  String filename;
  if (request->url().startsWith("/downloadhandler") || request->url().startsWith("/deletehandler"))
  {
    if (!request->url().startsWith("/renamehandler")) filename = request->url().substring(request->url().indexOf("~/") + 1);
    start = millis();
    if (request->url().startsWith("/downloadhandler"))
    {
      Serial.println("Download handler started...");
      MessageLine = "";
      File file = SD.open(filename, FILE_READ);
      String contentType = getContentType("download");
      AsyncWebServerResponse *response = request->beginResponse(contentType, file.size(), [file](uint8_t *buffer, size_t maxLen, size_t total) mutable -> size_t {
        File filecopy = file;
        int bytes = filecopy.read(buffer, maxLen);
        if (bytes + total == filecopy.size()) filecopy.close();
        return max(0, bytes);
      });
      response->addHeader("Server", "ESP Async Web Server");

      request->send(response);
      downloadtime = millis() - start;
      downloadsize = GetFileSize(filename);
      request->redirect("/dir");
    }
    if (request->url().startsWith("/deletehandler"))
    {
      Serial.println("Delete handler started...");
      Handle_File_Delete(filename); // Build webpage ready for display
      request->send(200, "text/html", webpage);
    }
  }
  else
  {
    Page_Not_Found();
    request->send(200, "text/html", webpage);
  }
}

//#############################################################################################
void Handle_File_Download() {
  String filename = "";
  int index = 0;
  Directory(); // Get a list of files on the FS
  webpage = HTML_Header();
  webpage += "<h3>Download</h3>";
  webpage += "<table>";
  webpage += "<tr><th>Dosya Adı</th><th>Dosya Boyutu</th></tr>";
  while (index < numfiles) {
    webpage += "<tr><td><a href='" + Filenames[index].filename + "'></a><td>" + Filenames[index].fsize + "</td></tr>";
    index++;
  }
  webpage += "</table>";
  webpage += "<p>" + MessageLine + "</p>";
}
//#############################################################################################
String getContentType(String filenametype) { // Tell the browser what file type is being sent
  if (filenametype == "download") {
    return "application/octet-stream";
  } else if (filenametype.endsWith(".txt"))  {
    return "text/plainn";
  } else if (filenametype.endsWith(".htm"))  {
    return "text/html";
  } else if (filenametype.endsWith(".html")) {
    return "text/html";
  } else if (filenametype.endsWith(".css"))  {
    return "text/css";
  } else if (filenametype.endsWith(".js"))   {
    return "application/javascript";
  } else if (filenametype.endsWith(".png"))  {
    return "image/png";
  } else if (filenametype.endsWith(".gif"))  {
    return "image/gif";
  } else if (filenametype.endsWith(".jpg"))  {
    return "image/jpeg";
  } else if (filenametype.endsWith(".ico"))  {
    return "image/x-icon";
  } else if (filenametype.endsWith(".xml"))  {
    return "text/xml";
  } else if (filenametype.endsWith(".pdf"))  {
    return "application/x-pdf";
  } else if (filenametype.endsWith(".zip"))  {
    return "application/x-zip";
  } else if (filenametype.endsWith(".gz"))   {
    return "application/x-gzip";
  }
  return "text/plain";
}
//#############################################################################################
void Select_File_For_Function(String title, String function) {
  String Fname1, Fname2;
  int index = 0;
  Directory(); // Get a list of files on the FS
  webpage = HTML_Header();
  webpage += "<table class='center'>";
  webpage += "<tr><th>File Name</th><th>File Size</th><th class='sp'></th><th>File Name</th><th>File Size</th></tr>";
  while (index < numfiles) {
    Fname1 = Filenames[index].filename;
    Fname2 = Filenames[index + 1].filename;
    if (Fname1.startsWith("/")) Fname1 = Fname1.substring(1);
    if (Fname2.startsWith("/")) Fname1 = Fname2.substring(1);
    webpage += "<tr>";
    webpage += "<td style='width:25%'><button><a href='" + function + "~/" + Fname1 + "'>" + Fname1 + "</a></button></td><td style = 'width:10%'>" + Filenames[index].fsize + "</td>";
    webpage += "<td class='sp'></td>";
    if (index < numfiles - 1) {
      webpage += "<td style='width:25%'><button><a href='" + function + "~/" + Fname2 + "'>" + Fname2 + "</a></button></td><td style = 'width:10%'>" + Filenames[index + 1].fsize + "</td>";
    }
    webpage += "</tr>";
    index = index + 2;
  }
  webpage += "</table>";

}
//#############################################################################################
void SelectInput(String Heading, String Command, String Arg_name) {
  webpage = HTML_Header();
  webpage += "<h3>" + Heading + "</h3>";
  webpage += "<form  action='/" + Command + "'>";
  webpage += "Filename: <input type='text' name='" + Arg_name + "'><br><br>";
  webpage += "<input type='submit' value='Enter'>";
  webpage += "</form>";

}
//#############################################################################################
int GetFileSize(String filename) {
  int filesize;
  File CheckFile = SD.open(filename, FILE_READ);
  filesize = CheckFile.size();
  CheckFile.close();
  return filesize;
}
//#############################################################################################
void Home() {
  webpage = HTML_Header();
  webpage += "<h1>Home Page</h1>";
  webpage += "<h2>ESP Asychronous WebServer Example</h2>";
  webpage += "<img src = 'icon' alt='icon'>";
  webpage += "<h3>File Management - Directory, Upload, Download, Stream and Delete File Examples</h3>";
}

//#############################################################################################
void Page_Not_Found() {
  webpage = HTML_Header();
  webpage += "<div class='notfound'>";
  webpage += "<h1>Sorry</h1>";
  webpage += "<p>Error 404 - Page Not Found</p>";
  webpage += "</div><div class='left'>";
  webpage += "<p>The page you were looking for was not found, it may have been moved or is currently unavailable.</p>";
  webpage += "<p>Please check the address is spelt correctly and try again.</p>";
  webpage += "<p>Or click <b><a href='/'>[Here]</a></b> for the home page.</p></div>";
}
//#############################################################################################
String ConvBinUnits(size_t bytes, byte resolution) {
  if      (bytes < 1024)                 {
    return String(bytes) + " B";
  }
  else if (bytes < 1024 * 1024)          {
    return String(bytes / 1024.0, resolution) + " KB";
  }
  else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0, resolution) + " MB";
  }
  else return "";
}
//#############################################################################################
String EncryptionType(wifi_auth_mode_t encryptionType) {
  switch (encryptionType) {
    case (WIFI_AUTH_OPEN):
      return "OPEN";
    case (WIFI_AUTH_WEP):
      return "WEP";
    case (WIFI_AUTH_WPA_PSK):
      return "WPA PSK";
    case (WIFI_AUTH_WPA2_PSK):
      return "WPA2 PSK";
    case (WIFI_AUTH_WPA_WPA2_PSK):
      return "WPA WPA2 PSK";
    case (WIFI_AUTH_WPA2_ENTERPRISE):
      return "WPA2 ENTERPRISE";
    case (WIFI_AUTH_MAX):
      return "WPA2 MAX";
    default:
      return "";
  }
}
//#############################################################################################
bool StartMDNSservice(const char* Name) {
  esp_err_t err = mdns_init();             // Initialise mDNS service
  if (err) {
    printf("MDNS Init failed: %d\n", err); // Return if error detected
    return false;
  }
  mdns_hostname_set(Name);                 // Set hostname
  return true;
}
//#############################################################################################
String HTML_Header() {
  String page;
  page  = "<!DOCTYPE html>";
  page += "<html lang = 'en'>";
  page += "<head>";
  page += "<title>TUGRULB WEB SERVER</title>";
  page += "<meta charset='UTF-8'>"; // Needed if you want to display special characters like the ° symbol
  page += "<style>";
  page += "body {width:19em;margin-left:auto;margin-right:auto;font-family:Arial,Helvetica,sans-serif;font-size:12px;color:black;background-color:#ffff;text-align:center;}";
  page += "footer {padding:0.08em;background-color:cyan;font-size:1.1em;}";
  page += "table {font-family:arial,sans-serif;border-collapse:collapse;width:70%;}"; // 70% of 75em!
  page += "table.center {margin-left:auto;margin-right:auto;}";
  page += "td, th {border:1px solid #dddddd;text-align:left;padding:8px;}";
  page += "tr:nth-child(even) {background-color:#dddddd;}";
  page += "h4 {color:slateblue;font:0.8em;text-align:left;font-style:oblique;text-align:center;}";
  page += ".center {margin-left:auto;margin-right:auto;}";
  page += ".topnav {overflow: hidden;background-color:white;}";
  page += ".topnav a {float:left;color:blue;text-align:center;padding:0.6em 0.6em;text-decoration:none;font-size:1.3em;}";
  page += ".topnav a:hover {background-color:#ccc6c6;color:black;}";
  page += ".topnav a.active {background-color:white;color:blue;}";
  page += ".notfound {padding:0.8em;text-align:left;font-size:1.5em;}";
  page += ".left {text-align:center;}";
  page += ".medium {font-size:1.4em;padding:0;margin:0}";
  page += ".ps {font-size:0.7em;padding:0;margin:0}";
  page += ".sp {background-color:white;white-space:nowrap;width:2%;}";
  page += "</style>";
  page += "</head>";
  page += "<body>";
  page += "<div class = 'topnav'>";
  page += "<a href='/upload'>Upload</a> ";
  page += "<a href='/download'>Download</a>";
  page += "<a href='/delete'>Delete</a>";
  page += "</div>";
  return page;
}
