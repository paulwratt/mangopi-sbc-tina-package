/**********************
 *      includes
 **********************/
#include "va_font.h"

typedef struct
{
    FT_Face face;
    uint8_t *dst_buffer;
    unsigned int font_size;
    unsigned int bpp;
}font_para;
struct FT_LibraryRec_ *m_ft = NULL;

static const uint8_t* copy_bitmap(FT_Bitmap *bitmap, uint8_t*dst_buffer, uint32_t size,
	uint8_t real_bpp, uint8_t dst_bpp)
{
    int mask = 0;
	uint8_t *src = NULL;
    uint32_t src_size = 0;
    uint8_t *dst = 0;
	int cnt = 0, cnt1 = 0, cnt2 = 0;

    switch(dst_bpp)
    {
        case 4:
            mask = 0xf0;
            break;
        case 2:

            mask = 0xc0;
            break;
        case 1:

            mask = 0x80;
             break;
        default:
			 com_warn("\n");
            break;
    }

    if(real_bpp < dst_bpp) {
        com_warn("\n");
    }

    src = bitmap->buffer;
    src_size = bitmap->width*bitmap->rows;
    dst = dst_buffer;
    memset(dst, 0, size*size);

    for (uint j = 0; j < src_size*dst_bpp/8; ++j)
    {
        for (int i = 0; i < 8; i += dst_bpp)
        {
            const bool ok = ((uint32_t)(src - bitmap->buffer) < src_size);
            const uint8_t val = (ok?*src:0);
            *dst |= ((val & mask) >> i);
            if (ok)
            {
                ++src;
                cnt1++;
            }
            else{
                cnt2++;
            }
        }
        ++dst;
        cnt++;
    }
    return ( const uint8_t*)dst_buffer;
}

static const uint8_t *__user_font_get_bitmap(const lv_font_t * font, uint32_t unicode_letter)
{
    font_para *para = (font_para *)font->dsc;
    uint8_t *dst_buffer = para->dst_buffer;
    uint8_t real_bpp = para->face->glyph->bitmap.pixel_mode;

    switch(real_bpp)
    {
        case 1:
            real_bpp = 1;
            break;
        case 2:
            real_bpp = 8;
            break;
        default:
            break;
    }

    if(para->bpp == 8)
    {
        return (const uint8_t *)(para->face->glyph->bitmap.buffer);
    }
    else
    {
        return copy_bitmap(&para->face->glyph->bitmap,dst_buffer,para->font_size,real_bpp,para->bpp);
    }
}

static bool __user_font_get_glyph_dsc(const lv_font_t * font, lv_font_glyph_dsc_t * dsc_out,
	uint32_t unicode_letter, uint32_t unicode_letter_next)
{
    FT_Error error;
    FT_Glyph glyph;
    FT_UInt glyph_index;
    font_para *para = (font_para *)font->dsc;

    glyph_index = FT_Get_Char_Index(para->face, unicode_letter);
    if(glyph_index == 0) {
         return false;
    }

    error = FT_Load_Glyph(para->face, glyph_index, FT_LOAD_DEFAULT);
    error |= FT_Render_Glyph(para->face->glyph, FT_RENDER_MODE_NORMAL);
    // FT_Get_Glyph(para->face->glyph, &glyph);

    if(error) {
		com_warn("\n");
	return false;
    }

    dsc_out->adv_w = para->face->glyph->metrics.horiAdvance >> 6;
    dsc_out->box_h = para->face->glyph->bitmap.rows;
    dsc_out->box_w = para->face->glyph->bitmap.width;
    dsc_out->ofs_x = para->face->glyph->bitmap_left;
    dsc_out->ofs_y = para->face->glyph->bitmap_top - para->face->glyph->bitmap.rows;
    dsc_out->bpp   = para->bpp;
	// FT_Done_Glyph(glyph);
    return true;
}

lv_font_t *ttf_font_create(const char* fileName, int size, int bpp)
{
    FT_Error error;
    lv_font_t *font = NULL;
    font_para *para = NULL;

	font = (lv_font_t *)malloc(sizeof(lv_font_t));
	if(font == NULL) {
		com_err("\n");
		goto END;
	}
	memset(font, 0, sizeof(lv_font_t));

	para = (font_para *)malloc(sizeof(font_para));
	if(para == NULL) {
		com_err("\n");
		goto END;
	}
    memset(para, 0, sizeof(font_para));
	para->dst_buffer = (uint8_t *)malloc(size*size);
	if(para->dst_buffer == NULL) {
		com_err("\n");
		goto END;
	}
    para->font_size = size;
    para->bpp = bpp;

    error = FT_New_Face(m_ft, fileName, 0, &para->face);
    if(error) {
        com_err("\n");
		goto END;
    }

    error = FT_Set_Pixel_Sizes(para->face, 0, size);
    if(error) {
        com_err("\n");
		goto END;
    }

    font->get_glyph_bitmap = __user_font_get_bitmap;
    font->get_glyph_dsc = __user_font_get_glyph_dsc;
    font->line_height = (para->face->size->metrics.height >> 6);
    font->base_line = -(para->face->size->metrics.descender >> 6);
    font->subpx = LV_FONT_SUBPX_NONE;
    font->dsc = para;
	return font;

	END:
		if(font) {
			if(para) {
				if(para->dst_buffer) {
					free(para->dst_buffer);
					para->dst_buffer = NULL;
				}
				free(para);
				para = NULL;
			}
			free(font);
			font = NULL;
		}

    return NULL;
}

void ttf_font_destory(lv_font_t *font)
{
	font_para *para = NULL;
	if(font) {
		para = (font_para *)font->dsc;
		if(para) {
			if(para->dst_buffer) {
				free(para->dst_buffer);
				para->dst_buffer = NULL;
			}
			free(para);
			para = NULL;
		}
		free(font);
	}
}

void ttf_font_init(void)
{
	FT_Init_FreeType(&m_ft);
}

void ttf_font_uninit(void)
{
	FT_Done_FreeType(m_ft);
}
