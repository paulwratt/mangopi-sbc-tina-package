#include "rat_common.h"
#include "common.h"

void media_play_mod_icon_destory(void);
int media_update_playmode_btn(lv_obj_t*btn_obj, rat_play_mode_e play_mode);
lv_obj_t* media_mbox_create(const char* message, unsigned int time, lv_event_cb_t event_cb);
lv_obj_t *create_mbox_reminder_playmode_change(lv_obj_t*btn_obj);
