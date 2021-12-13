#ifndef OTA
#define OTA

void ota_init_enable(void);
bool ota_is_updating(void);
void ota_task_handle(void);

#endif