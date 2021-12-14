#include "spy_camera.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <Arduino.h>

bool spy_camera_init(void) {

  // init flash pin
  pinMode(CAM_FLASH_PIN, OUTPUT);

  // for image quality improvement, refer to https://github.com/espressif/esp32-camera/issues/203
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = TARGET_JPEG_QUALITY;  //0-63 lower number means higher quality
    config.fb_count = TARGET_FRAME_COUNT;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 2;  //0-63 lower number means higher quality
    config.fb_count = 1;
  }
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    return false;
  }
  // Drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, TARGET_DEFINITION);  // UXGA|SXGA|XGA|SVGA|VGA|CIF|QVGA|HQVGA|QQVGA
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  //s->set_reg(s,0xff,0xff,0x01);//banksel
  //s->set_reg(s,0x11,0xff,01);//frame rate

  delay(1200);
  return true;
}

camera_fb_t * spy_camera_take_picture(bool withFlash) {
  if (withFlash) {
    spy_camera_turn_flash_on();
    delay(200);
  }
  camera_fb_t * fb = esp_camera_fb_get();  
  if (withFlash) {
    delay(200);
    spy_camera_turn_flash_off();
  }
  return fb;
}

void spy_camera_close_buffer(camera_fb_t * fb) {
  if (fb) {
    esp_camera_fb_return(fb);
  }
}

void spy_camera_turn_flash_on(void) {
  digitalWrite(CAM_FLASH_PIN, HIGH);
}

void spy_camera_turn_flash_off(void) {
  digitalWrite(CAM_FLASH_PIN, LOW);
}
