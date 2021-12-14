#include "telegram_helper.h"
#include "secrets.h"

const char* myDomain = "api.telegram.org";
#define HTTPS_PORT  (443)

// should use: bot.sendPhotoByBinary
// refer to: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot/blob/master/examples/ESP32/SendPhoto/PhotoFromSerial/PhotoFromSerial.ino#L93
// ok, not working with > VGA pictures, keep the byte by byte write process
void telegram_helper_send_photo_buffer(WiFiClientSecure * secured_client, uint8_t * fbBuf, size_t fbLen) {

  if (secured_client->connect(myDomain, HTTPS_PORT)) {
    
    String head = "--ESP32HomeSurveillance\r\nContent-Disposition: form-data; name=\"chat_id\"; \r\n\r\n" + String(CHAT_ID) + "\r\n--ESP32HomeSurveillance\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"esp32-cam.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--ESP32HomeSurveillance--\r\n";

    uint16_t extraLen = head.length() + tail.length();
    uint16_t totalLen = fbLen + extraLen;
  
    secured_client->println("POST /bot" + String(BOT_TOKEN) + "/sendPhoto HTTP/1.1");
    secured_client->println("Host: " + String(myDomain));
    secured_client->println("Content-Length: " + String(totalLen));
    secured_client->println("Content-Type: multipart/form-data; boundary=ESP32HomeSurveillance");
    secured_client->println();
    secured_client->print(head);
    for (size_t n=0;n<fbLen;n=n+1024) {
      if (n+1024<fbLen) {
        secured_client->write(fbBuf, 1024);
        fbBuf += 1024;
      }
      else if (fbLen%1024>0) {
        size_t remainder = fbLen%1024;
        secured_client->write(fbBuf, remainder);
      }
    }
    secured_client->print(tail);
    // do not terminate with secured_client->stop();
  }
}
