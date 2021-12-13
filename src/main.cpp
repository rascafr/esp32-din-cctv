#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "spy_camera.h"
#include "ota.h"
#include "utils.h"
#include "secrets.h"

/*
 * Local general definitions 
 */
#define DOOR_SENSOR_DEBOUNCE_MS   (500)
#define TELEGRAM_CHECK_DATA_MS    (1000)

/*
 * I/O Pin definitions
 */
#define DOOR_SENSOR_PIN   (18)
#define DBG_RED_LED_PIN   (33)

/*
 * Private local variables 
 */
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
char hostname[23]; // ESP32-CAMERA-xxxxxxxxx
char tmp[100];
uint32_t last_check_telegram_data;

/*
 * Private local functions prototypes
 */
void sendPhotoTelegram(bool withFlash);
int handleTelegramMessages(int pending);

void setup() {

  Serial.begin(115200);
  Serial.println();

  pinMode(DBG_RED_LED_PIN, OUTPUT);

  spy_camera_init();

  utils_write_hostname(hostname);

  WiFi.setHostname(hostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED);

  time_t now;
  utils_init_time(&now);

  ota_init_enable();

  sprintf(tmp, "✅ ESP32 boot OK at `%s`\nRevision `%s`", ctime(&now), GIT_TAG);
  bot.sendMessage(CHAT_ID, tmp, "Markdown");
}

void loop() {

  // !!! CRITICAL LOOP, AVOID EDITING IT !!!
  while (true) {

    // Handle OTA updates here
    ota_task_handle();

    // !!! CRTITICAL DO NOT TOUCH OR YOU MIGHT BLOCK OTA UPDATES !!!
    if (!ota_is_updating() && millis() > last_check_telegram_data + TELEGRAM_CHECK_DATA_MS) {
      int pending_telegram_messages = bot.getUpdates(bot.last_message_received + 1);
      while (handleTelegramMessages(pending_telegram_messages));
      last_check_telegram_data = millis();
    }
  }
}

int handleTelegramMessages(int pending) {
  // handle here
  for (int i=0;i<pending;i++) {
    String text = bot.messages[i].text;
    pending = bot.getUpdates(bot.last_message_received + 1);

    if (text == "/reboot") {
      bot.sendMessage(CHAT_ID, "ESP32 rebooting now...", "Markdown");
      ESP.restart();
    }
    else if (text == "/capture") {
      // https://core.telegram.org/bots/api#sendchataction
      bot.sendChatAction(CHAT_ID, "upload_photo");
      sendPhotoTelegram(true);
    }
    else if (text == "/status") {
      time_t now = utils_get_time();
      sprintf(tmp, 
        "Uptime: `%ld seconds`\nIP: `%s`\nRSSI: `%d dBm`\nTime: `%s`",
        millis()/1000, WiFi.localIP().toString().c_str(), WiFi.RSSI(), ctime(&now)
      );
      bot.sendMessage(CHAT_ID, tmp, "Markdown");
    }
  }

  return pending;
}

void sendPhotoTelegram(bool withFlash) {
  // should use: bot.sendPhotoByBinary
  // refer to: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP32/SendPhoto/PhotoFromSerial/PhotoFromSerial.ino#L93
  const char* myDomain = "api.telegram.org";
  String getAll = "";
  String getBody = "";

  camera_fb_t * fb = spy_camera_take_picture(withFlash);

  if (secured_client.connect(myDomain, 443)) {
    
    String head = "--ESP32HomeSurveillance\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + String(CHAT_ID) + "\r\n--ESP32HomeSurveillance\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--ESP32HomeSurveillance--\r\n";

    uint16_t imageLen = fb->len;
    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = imageLen + extraLen;
  
    secured_client.println("POST /bot"+String(BOT_TOKEN)+"/sendPhoto HTTP/1.1");
    secured_client.println("Host: " + String(myDomain));
    secured_client.println("Content-Length: " + String(totalLen));
    secured_client.println("Content-Type: multipart/form-data; boundary=ESP32HomeSurveillance");
    secured_client.println();
    secured_client.print(head);
    uint8_t *fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        secured_client.write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        secured_client.write(fbBuf, remainder);
      }
    }  
    
    secured_client.print(tail);
    
    spy_camera_close_buffer(fb);
    
    int waitTime = 10000;   // timeout 10 seconds
    long startTimer = millis();
    bool state = false;
    
    while ((startTimer + waitTime) > millis()){
      delay(100);      
      while (secured_client.available()) {
        char c = secured_client.read();
        if (state == true) getBody += String(c);        
        if (c == '\n') {
          if (getAll.length() == 0) state=true; 
          getAll = "";
        } 
        else if (c != '\r')
          getAll += String(c);
        startTimer = millis();
      }
      if (getBody.length() > 0) break;
    }
    secured_client.stop();
  }
}
