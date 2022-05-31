#include <WiFiManager.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <HTTPUpdate.h>
#define LED 2
WiFiClient CLIENT;
HTTPClient HTTP;
String API_URL = "http://103.174.114.226/api";
String THIS_MAC_ADDRESS = WiFi.macAddress();
String THIS_FIRMWARE_VERSION = "1.0";
JSONVar SERVER_DEVICE;
JSONVar SERVER_FIRMWARE;
void setup() {
  Serial.begin(115200);
  Serial.println(THIS_MAC_ADDRESS);
  WiFiManager MANAGER;
  bool SUCCESS = MANAGER.autoConnect("OTA Update", "12345678");
  if(!SUCCESS) {
    Serial.println("Gagal terhubung");
  }  else {
    Serial.println("Berhasil terhubung");
    Serial.println("Versi firmware: " + THIS_FIRMWARE_VERSION);
  }
}
void loop() {
  Serial.println("Mulai");
  // MENGECEK PERANGKAT
  Serial.println("Mengecek perangkat");
  HTTP.begin(CLIENT, API_URL + "/device/get?mac_address=" + THIS_MAC_ADDRESS);
  int RESPONSE_CODE = HTTP.GET();
  if(RESPONSE_CODE == 200) {
    Serial.println("Perangkat ditemukan, mengambil data perangkat");
    // MENGAMBIL DATA PERANGKAT
    SERVER_DEVICE = JSON.parse(HTTP.getString());
    String SERVER_DEVICE_STATUS = (const char*)SERVER_DEVICE["status"];
    // MENGECEK STATUS PERANGKAT SETELAH MEMPERBARUI
    Serial.println("Mengecek status perangkat");
    if(SERVER_DEVICE_STATUS == "2") {
      POST("/device/set", "status=0");
      POST("/log/store", "log=Perangkat selesai diperbarui&time=1");
      Serial.println("Perangkat selesai diperbarui");
    } else if(SERVER_DEVICE_STATUS == "1") {
      POST("/device/set", "status=2");
      Serial.println("Pembaruan firmware tersedia, perangkat akan melakukan pembaruan");
      HTTP.begin(CLIENT, API_URL + "/firmware/get-latest?mac_address=" + THIS_MAC_ADDRESS);
      int RESPONSE_CODE = HTTP.GET();
      if(RESPONSE_CODE == 200) {
        SERVER_FIRMWARE = JSON.parse(HTTP.getString());
        int SERVER_FIRMWARE_ID = SERVER_FIRMWARE["id"];
        POST("/device/set", "firmware_id=" + SERVER_FIRMWARE_ID);
        POST("/log/store", "log=Perangkat akan melakukan pembaruan");
        t_httpUpdate_return RETURN = httpUpdate.update(CLIENT, API_URL + "/firmware/" + SERVER_FIRMWARE_ID + "/download");
        Serial.println(RETURN);
      } else {
        Serial.println("Pembaruan gagal, firmware tidak ditemukan");
      }
    } else {
      Serial.println("Perangkat aktif");
      // GENERATE SUHU DAN KELEMBABAN
      float TEMPERATURE = random(0, 50);
      float HUMIDITY = random(0, 80);
      // MENGIRIM DATA
      POST("/data/store", "data={\"temperature\":" + String(TEMPERATURE) + ",\"humidity\":" + String(HUMIDITY) + "}");
      Serial.println("Berhasil mengirim data");
    }
  } else if(RESPONSE_CODE == 422) {
    Serial.println("Perangkat tidak ditemukan");
  } else {
    Serial.println("Terjadi kesalahan");
  }
  HTTP.end();
  Serial.println("Selesai");
  delay(5000);
}
void POST(String URL, String DATA)
{
  HTTP.begin(CLIENT, API_URL + URL);
  HTTP.addHeader("Content-Type", "application/x-www-form-urlencoded");
  HTTP.POST("mac_address=" + THIS_MAC_ADDRESS + "&" + DATA);
  HTTP.end();
}
