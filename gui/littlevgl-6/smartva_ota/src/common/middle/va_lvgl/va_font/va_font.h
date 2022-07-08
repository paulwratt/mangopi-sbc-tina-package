#ifndef __VA_FONT_H__
#define __VA_FONT_H__
#if CONFIG_FONT_ENABLE
#include "smt_config.h"
#include "lvgl.h"
#include <ft2build.h>
#include FT_GLYPH_H
#include FT_FREETYPE_H

void ttf_font_init(void);
void ttf_font_uninit(void);
lv_font_t *ttf_font_create(const char* fileName, int size, int bpp);
void ttf_font_destory(lv_font_t *font);
#endif
#endif /*__VA_IMAGE_H__*/
