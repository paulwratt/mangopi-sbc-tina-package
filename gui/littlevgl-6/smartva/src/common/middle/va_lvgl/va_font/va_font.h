#ifndef __VA_FONT_H__
#define __VA_FONT_H__
#include "smt_config.h"
#include "lvgl.h"
#include "lang.h"
#include <ft2build.h>
#include FT_GLYPH_H
#include FT_FREETYPE_H

void ttf_font_init(void);
void ttf_font_uninit(void);
lv_font_t *ttf_font_create(const char* fileName, int size, int bpp);
void ttf_font_destory(lv_font_t *font);

#endif /*__VA_IMAGE_H__*/
