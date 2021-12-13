#ifndef SPY_CAMERA
#define SPY_CAMERA

#include "esp_camera.h"
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Preferences / module config
#define TARGET_DEFINITION   FRAMESIZE_XGA
#define TARGET_JPEG_QUALITY (8)
#define TARGET_FRAME_COUNT  (2)

/*
 * I/O Pin definitions
 */
#define CAM_FLASH_PIN     (4)

//CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

bool spy_camera_init(void);
camera_fb_t * spy_camera_take_picture(bool withFlash = true);
void spy_camera_close_buffer(camera_fb_t * fb);
void spy_camera_turn_flash_on(void);
void spy_camera_turn_flash_off(void);

#endif