#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "spy_camera.h"
#include "ota.h"
#include "utils.h"
#include "secrets.h"
#include "door_sensor.h"
#include "telegram_helper.h"

/*
 * Local general definitions 
 */
#define TELEGRAM_CHECK_DATA_MS    (2000)

/*
 * I/O Pin definitions
 */
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
void IRAM_ATTR ISR_DoorSensor_Opened(void * context);

void setup() {

  Serial.begin(115200);
  Serial.println();

  pinMode(DBG_RED_LED_PIN, OUTPUT);
  spy_camera_init();
  door_sensor_init();
  utils_write_hostname(hostname);

  WiFi.setHostname(hostname);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);

  while (WiFi.status() != WL_CONNECTED);

  time_t now;
  utils_init_time(&now);
  ota_init_enable();
  bot.waitForResponse = 500;
  sprintf(tmp, "✅ ESP32 boot OK at `%s`\nRevision `%s`", ctime(&now), GIT_TAG);
  bot.sendMessage(CHAT_ID, tmp, "Markdown");
}

void loop() {

  // !!! CRITICAL LOOP, AVOID EDITING IT !!!
  while (true) {

    // Handle OTA updates here
    ota_task_handle();

    // !!! CRTITICAL DO NOT TOUCH OR YOU MIGHT BLOCK OTA UPDATES !!!
    if (!ota_is_updating()) {

      // ISR handle events
      door_sensor_task_handle();
      dated_event_t isr_event = door_sensor_get_event();

      switch (isr_event.type) {

        case EVENT_DOOR_OPENED:
          sprintf(tmp, "⚠️🚪 Door opened!\nTime: `%s`Id = `%d`", ctime(&isr_event.time), isr_event.id);
          bot.sendMessage(CHAT_ID, tmp, "Markdown");
          sendPhotoTelegram(true);
          break;

        case EVENT_DOOR_CLOSED:
          sprintf(tmp, "✅🚪 Door closed.\nTime: `%s`Id = `%d`", ctime(&isr_event.time), isr_event.id);
          bot.sendMessage(CHAT_ID, tmp, "Markdown");
          break;
      }

      // Telegram update messages task
      // !!! 1600 ms ~ !!!
      if (millis() > last_check_telegram_data + TELEGRAM_CHECK_DATA_MS) {
        int pending_telegram_messages = bot.getUpdates(bot.last_message_received + 1);
        while (handleTelegramMessages(pending_telegram_messages));
        last_check_telegram_data = millis();
      }
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
      sendPhotoTelegram(true);
    }
    else if (text == "/status") {
      time_t now = utils_get_time();
      sprintf(tmp,
        "Uptime: `%llu seconds`\nIP: `%s`\nRSSI: `%d dBm`\nTime: `%s`",
        millis()/1000, WiFi.localIP().toString().c_str(), WiFi.RSSI(), ctime(&now)
      );
      bot.sendMessage(CHAT_ID, tmp, "Markdown");
    }
  }

  return pending;
}

void sendPhotoTelegram(bool withFlash) {
  bot.sendChatAction(CHAT_ID, "upload_photo");
  camera_fb_t * fb = spy_camera_take_picture(withFlash);
  telegram_helper_send_photo_buffer(&secured_client, fb->buf, fb->len);
  spy_camera_close_buffer(fb);
}
