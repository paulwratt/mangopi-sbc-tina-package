/**********************
 *      includes
 **********************/
#include "moc_slide_home.h"
#include "ui_slide_home.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "bs_widget.h"

// common
static lv_style_t style0_page_1;
static lv_style_t style1_page_1;
static lv_style_t style2_page_1;

// page0
static lv_style_t style0_button_1;
static lv_style_t style1_button_1;
static lv_style_t style0_label_1;
static lv_style_t style0_button_2;
static lv_style_t style1_button_2;
static lv_style_t style0_label_2;
static lv_style_t style0_button_3;
static lv_style_t style1_button_3;
static lv_style_t style0_label_3;
static lv_style_t style0_button_4;
static lv_style_t style1_button_4;
static lv_style_t style0_label_4;
static lv_style_t style0_button_5;
static lv_style_t style1_button_5;
static lv_style_t style0_label_5;
static lv_style_t style0_button_6;
static lv_style_t style1_button_6;
static lv_style_t style0_label_6;
static lv_style_t style0_button_7;
static lv_style_t style1_button_7;
static lv_style_t style0_label_7;

#define SHOME_X0	80
#define SHOME_XI	160
#define SHOME_X_SUM	5
#define SHOME_Y0	50
#define SHOME_YI	100
#define SHOME_Y_SUM	4
static lv_style_t style0_line_1;
static lv_obj_t *line_1 = NULL;
static lv_obj_t * page1_parent = NULL;
static lv_point_t line_1_points[5] = {
	{SHOME_X0 - (SHOME_XI/2 - SHOME_XI/4), SHOME_Y0 - (SHOME_YI/2 - SHOME_YI/4)},
	{SHOME_X0 + (SHOME_XI/2 - SHOME_XI/4), SHOME_Y0 - (SHOME_YI/2 - SHOME_YI/4)},
	{SHOME_X0 + (SHOME_XI/2 - SHOME_XI/4), SHOME_Y0 + (SHOME_YI/2 - SHOME_YI/4)},
	{SHOME_X0 - (SHOME_XI/2 - SHOME_XI/4), SHOME_Y0 + (SHOME_YI/2 - SHOME_YI/4)},
	{SHOME_X0 - (SHOME_XI/2 - SHOME_XI/4), SHOME_Y0 - (SHOME_YI/2 - SHOME_YI/4)},
};

// page3
static lv_obj_t * ta;
static lv_obj_t * kb;
static lv_style_t style_kb;
static lv_style_t style_kb_rel;
static lv_style_t style_kb_pr;


/**********************
 *       variables
 **********************/
typedef struct
{
	int id;
	int b_anim;
	lv_obj_t *img_bgd;
	lv_task_t *hbar_tid;
	lv_obj_t *button_1;
	lv_obj_t *label_1;
	lv_obj_t *button_2;
	lv_obj_t *label_2;
	lv_obj_t *button_3;
	lv_obj_t *label_3;
	lv_obj_t *button_4;
	lv_obj_t *label_4;
	lv_obj_t *button_5;
	lv_obj_t *label_5;
	lv_obj_t *button_6;
	lv_obj_t *label_6;
	lv_obj_t *button_7;
	lv_obj_t *label_7;
} slide_home_moc_t;

typedef struct
{
	slide_home_ui_t ui;
	slide_home_moc_t moc;
} slide_home_para_t;
static slide_home_para_t para;


/**********************
 *  functions
 **********************/
// common
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_SLIDE_HOME);
		//create_page(PAGE_HOME);
		//switch_page(PAGE_SLIDE_HOME, PAGE_HOME);
		anim_switch_page(PAGE_SLIDE_HOME, PAGE_HOME);
	}
}

static void button_event_cb(lv_obj_t * btn, lv_event_t event)
{
	int i, j;
	lv_coord_t x, y;
        lv_coord_t dst_x = 0;
	lv_coord_t dst_y = 0;
	static int cnt = 0;
	int index_x = 0;
	int index_y = 0;

	if (event == LV_EVENT_CLICKED) {
		;
	}
	else if (event == LV_EVENT_PRESSING) {
		x = lv_obj_get_x(btn) + lv_obj_get_width(btn)/2;
		for(i=1; i<SHOME_X_SUM; i++)
		{
			if(x <= (SHOME_X0 + SHOME_XI/2)) {
				dst_x = SHOME_X0;
				index_x = 0;
				break;
			}
			else if (x >= (SHOME_X0 + SHOME_XI*(SHOME_X_SUM-1) -  SHOME_XI/2)) {
				dst_x = SHOME_X0 + SHOME_XI*(SHOME_X_SUM-1);
				index_x = SHOME_X_SUM-1;
				break;
			}
			else if(x > (SHOME_X0 + SHOME_XI*i - SHOME_XI/2)
				&& x <= (SHOME_X0 + SHOME_XI*i + SHOME_XI/2))
			{
				dst_x = SHOME_X0 + SHOME_XI*i;
				index_x = i;
				break;
			}
		}

		y = lv_obj_get_y(btn) + lv_obj_get_height(btn)/2;
		for(j=1; j<SHOME_Y_SUM; j++)
		{
			if(y <= (SHOME_Y0 + SHOME_YI/2)) {
				dst_y = SHOME_Y0;
				index_y = 0;
				break;
			}
			else if (y >= (SHOME_Y0 + SHOME_YI*(SHOME_Y_SUM-1) -  SHOME_YI/2)) {
				dst_y = SHOME_Y0 + SHOME_YI*(SHOME_Y_SUM-1);
				index_y = SHOME_Y_SUM-1;
				break;
			}
			else if(y > (SHOME_Y0 + SHOME_YI*j - SHOME_YI/2)
				&& y <= (SHOME_Y0 + SHOME_YI*j + SHOME_YI/2))
			{
				dst_y = SHOME_Y0 + SHOME_YI*j;
				index_y = j;
				break;
			}
		}

		lv_obj_set_pos(line_1, SHOME_X0 + SHOME_XI*index_x - SHOME_XI/2,
			SHOME_Y0 + SHOME_YI*index_y - SHOME_YI/2);
		lv_obj_set_size(line_1, SHOME_XI, SHOME_YI);
		lv_obj_set_size(line_1, SHOME_XI, SHOME_YI);
	}
	else if (event == LV_EVENT_DRAG_BEGIN) {
		style0_line_1.line.opa = 64;
		lv_line_set_style(line_1, LV_LINE_STYLE_MAIN, &style0_line_1);
	}
	else if (event == LV_EVENT_DRAG_END)
	{
		cnt++;
		if(cnt == 2)
		{
			cnt = 0;
			x = lv_obj_get_x(btn) + lv_obj_get_width(btn)/2;
			//com_info("x =%d\n", x);
			for(i=1; i<SHOME_X_SUM; i++)
			{
				if(x <= (SHOME_X0 + SHOME_XI/2)) {
					dst_x = SHOME_X0;
					index_x = 0;
					break;
				}
				else if (x >= (SHOME_X0 + SHOME_XI*(SHOME_X_SUM-1) -  SHOME_XI/2)) {
					dst_x = SHOME_X0 + SHOME_XI*(SHOME_X_SUM-1);
					index_x = SHOME_X_SUM-1;
					break;
				}
				else if(x > (SHOME_X0 + SHOME_XI*i - SHOME_XI/2)
					 && x <= (SHOME_X0 + SHOME_XI*i + SHOME_XI/2))
				{
					dst_x = SHOME_X0 + SHOME_XI*i;
					index_x = i;
					break;
				}
			}
			dst_x = dst_x - lv_obj_get_width(btn)/2;
			lv_obj_set_x(btn, dst_x);
			//com_info("dst_x =%d\n", dst_x);

			y = lv_obj_get_y(btn) + lv_obj_get_height(btn)/2;
			//com_info("y =%d\n", y);
			for(j=1; j<SHOME_Y_SUM; j++)
			{
				if(y <= (SHOME_Y0 + SHOME_YI/2)) {
					dst_y = SHOME_Y0;
					index_y = 0;
					break;
				}
				else if (y >= (SHOME_Y0 + SHOME_YI*(SHOME_Y_SUM-1) -  SHOME_YI/2)) {
					dst_y = SHOME_Y0 + SHOME_YI*(SHOME_Y_SUM-1);
					index_y = SHOME_Y_SUM-1;
					break;
				}
				else if(y > (SHOME_Y0 + SHOME_YI*j - SHOME_YI/2)
					 && y <= (SHOME_Y0 + SHOME_YI*j + SHOME_YI/2))
				{
					dst_y = SHOME_Y0 + SHOME_YI*j;
					index_y = j;
					break;
				}
			}
			dst_y = dst_y - lv_obj_get_height(btn)/2;
			lv_obj_set_y(btn, dst_y);
			//com_info("dst_y =%d\n", dst_y);

			lv_obj_set_pos(line_1, SHOME_X0 + SHOME_XI*index_x - SHOME_XI/2,
				SHOME_Y0 + SHOME_YI*index_y - SHOME_YI/2);
			lv_obj_set_size(line_1, SHOME_XI, SHOME_YI);
			lv_line_set_points(line_1, line_1_points, 5);
			style0_line_1.line.opa = 0;
			lv_line_set_style(line_1, LV_LINE_STYLE_MAIN, &style0_line_1);
		}
	}
}

static void btn_hbar_return_event_cb(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//destory_page(PAGE_SLIDE_HOME);
		//create_page(PAGE_HOME);
		//switch_page(PAGE_SLIDE_HOME, PAGE_HOME);
		anim_switch_page(PAGE_SLIDE_HOME, PAGE_HOME);
	}
}

static void tabview_event_cb(lv_obj_t *obj, lv_event_t event)
{
	static uint8_t tab = 0;

	if(event == LV_EVENT_VALUE_CHANGED) {
		tab = lv_tabview_get_tab_act(obj);
		if(tab <= 1) {
			lv_tabview_set_tab_act(obj, 1, LV_ANIM_ON);
		}
		else if(tab >= 3) {
			lv_tabview_set_tab_act(obj, 3, LV_ANIM_ON);
		}
	}

}

static void clean_page(lv_obj_t * parent)	// ��page���ó�͸��
{
#ifdef LV_USE_PAGE
	lv_style_copy(&style0_page_1, &lv_style_pretty_color);
	style0_page_1.body.radius = 0;
	style0_page_1.body.opa = 0;
	style0_page_1.body.border.width = 0;
	style0_page_1.body.border.opa = 0;

	lv_style_copy(&style1_page_1, &lv_style_pretty);
	style1_page_1.body.radius = 0;
	style1_page_1.body.opa = 0;
	style1_page_1.body.border.width = 0;
	style1_page_1.body.border.opa = 0;

	lv_style_copy(&style2_page_1, &lv_style_pretty_color);
	style2_page_1.body.radius = 0;
	style2_page_1.body.opa = 0;
	style2_page_1.body.border.width = 0;
	style2_page_1.body.border.opa = 0;

	lv_page_set_style(parent, LV_PAGE_STYLE_BG, &style0_page_1);
	lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &style1_page_1);
	lv_page_set_style(parent, LV_PAGE_STYLE_SB, &style2_page_1);
#endif // LV_USE_PAGE
}

// page2
static void list_btn_event_handler(lv_obj_t * btn, lv_event_t event)
{

    if(event == LV_EVENT_SHORT_CLICKED) {
        lv_ta_add_char(ta, '\n');
        lv_ta_add_text(ta, lv_list_get_btn_text(btn));
    }
}

// page3
#if LV_USE_ANIMATION
static void kb_hide_anim_end(lv_anim_t * a)
{
    lv_obj_del(a->var);
    kb = NULL;
}
#endif

static void keyboard_event_cb(lv_obj_t * keyboard, lv_event_t event)
{
    (void) keyboard;    /*Unused*/

    lv_kb_def_event_cb(kb, event);

    if(event == LV_EVENT_APPLY || event == LV_EVENT_CANCEL) {
#if LV_USE_ANIMATION
        lv_anim_t a;
        a.var = kb;
        a.start = lv_obj_get_y(kb);
        a.end = LV_VER_RES;
        a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
        a.path_cb = lv_anim_path_linear;
        a.ready_cb = kb_hide_anim_end;
        a.act_time = 0;
        a.time = 300;
        a.playback = 0;
        a.playback_pause = 0;
        a.repeat = 0;
        a.repeat_pause = 0;
        lv_anim_create(&a);
#else
        lv_obj_del(kb);
        kb = NULL;
#endif
    }
}

static void text_area_event_handler(lv_obj_t * text_area, lv_event_t event)
{
    (void) text_area;    /*Unused*/

    /*Text area is on the scrollable part of the page but we need the page itself*/
    lv_obj_t * parent = lv_obj_get_parent(lv_obj_get_parent(ta));

    if(event == LV_EVENT_CLICKED) {
        if(kb == NULL) {
            kb = lv_kb_create(parent, NULL);
            lv_obj_set_size(kb, lv_obj_get_width_fit(parent), lv_obj_get_height_fit(parent) / 2);
            lv_obj_align(kb, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
            lv_kb_set_ta(kb, ta);
            lv_kb_set_style(kb, LV_KB_STYLE_BG, &style_kb);
            lv_kb_set_style(kb, LV_KB_STYLE_BTN_REL, &style_kb_rel);
            lv_kb_set_style(kb, LV_KB_STYLE_BTN_PR, &style_kb_pr);
            lv_obj_set_event_cb(kb, keyboard_event_cb);

#if LV_USE_ANIMATION
            lv_anim_t a;
            a.var = kb;
            a.start = LV_VER_RES;
            a.end = lv_obj_get_y(kb);
            a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_y;
            a.path_cb = lv_anim_path_linear;
            a.ready_cb = NULL;
            a.act_time = 0;
            a.time = 300;
            a.playback = 0;
            a.playback_pause = 0;
            a.repeat = 0;
            a.repeat_pause = 0;
            lv_anim_create(&a);
#endif
        }
    }

}

static void page0_create(lv_obj_t *parent)
{
	clean_page(parent);
}

static void page1_btn_create(lv_obj_t *parent)
{
	#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_1, &lv_style_btn_rel);
	style0_button_1.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_1.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_1.body.radius = 12;
	style0_button_1.body.border.width = 1;
	style0_button_1.body.shadow.width = 1;
	style0_button_1.body.padding.top = 0;
	style0_button_1.body.padding.bottom = 0;
	style0_button_1.body.padding.left = 0;

	lv_style_copy(&style1_button_1, &lv_style_btn_pr);
	style1_button_1.body.main_color = lv_color_hex(0x00aaff);
	style1_button_1.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_1.body.radius = 12;
	style1_button_1.body.border.width = 1;

	para.moc.button_1 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_1, 35, 15);
	lv_obj_set_size(para.moc.button_1, 90, 70);
	lv_btn_set_layout(para.moc.button_1, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_1, LV_BTN_STYLE_REL, &style0_button_1);
	lv_btn_set_style(para.moc.button_1, LV_BTN_STYLE_PR, &style1_button_1);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_1, &lv_style_transp);
	//style0_label_1.text.font = get_font_lib()->msyh_20;
	style0_label_1.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_1.text.line_space = 2;

	para.moc.label_1 = lv_label_create(para.moc.button_1, NULL);
	lv_label_set_text(para.moc.label_1, "控件1");
	lv_label_set_align(para.moc.label_1, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_1, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_1, 0, 21);
	lv_obj_set_size(para.moc.label_1, 90, 29);
	lv_label_set_style(para.moc.label_1, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_2, &lv_style_btn_rel);
	style0_button_2.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_2.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_2.body.radius = 12;
	style0_button_2.body.border.width = 1;
	style0_button_2.body.shadow.width = 1;
	style0_button_2.body.padding.top = 0;
	style0_button_2.body.padding.bottom = 0;
	style0_button_2.body.padding.left = 0;

	lv_style_copy(&style1_button_2, &lv_style_btn_pr);
	style1_button_2.body.main_color = lv_color_hex(0x00aaff);
	style1_button_2.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_2.body.radius = 12;
	style1_button_2.body.border.width = 1;

	para.moc.button_2 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_2, 35 + SHOME_XI*(2-1), 15);
	lv_obj_set_size(para.moc.button_2, 90, 70);
	lv_btn_set_layout(para.moc.button_2, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_2, LV_BTN_STYLE_REL, &style0_button_2);
	lv_btn_set_style(para.moc.button_2, LV_BTN_STYLE_PR, &style1_button_2);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_2, &lv_style_transp);
	//style0_label_2.text.font = get_font_lib()->msyh_20;
	style0_label_2.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_2.text.line_space = 2;

	para.moc.label_2 = lv_label_create(para.moc.button_2, NULL);
	lv_label_set_text(para.moc.label_2, "控件2");
	lv_label_set_align(para.moc.label_2, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_2, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_2, 0, 21);
	lv_obj_set_size(para.moc.label_2, 90, 29);
	lv_label_set_style(para.moc.label_2, LV_LABEL_STYLE_MAIN, &style0_label_2);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_3, &lv_style_btn_rel);
	style0_button_3.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_3.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_3.body.radius = 12;
	style0_button_3.body.border.width = 1;
	style0_button_3.body.shadow.width = 1;
	style0_button_3.body.padding.top = 0;
	style0_button_3.body.padding.bottom = 0;
	style0_button_3.body.padding.left = 0;

	lv_style_copy(&style1_button_3, &lv_style_btn_pr);
	style1_button_3.body.main_color = lv_color_hex(0x00aaff);
	style1_button_3.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_3.body.radius = 12;
	style1_button_3.body.border.width = 1;

	para.moc.button_3 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_3, 35 + SHOME_XI*(3-1), 15);
	lv_obj_set_size(para.moc.button_3, 90, 70);
	lv_btn_set_layout(para.moc.button_3, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_3, LV_BTN_STYLE_REL, &style0_button_3);
	lv_btn_set_style(para.moc.button_3, LV_BTN_STYLE_PR, &style1_button_3);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_3, &lv_style_transp);
	//style0_label_3.text.font = get_font_lib()->msyh_20;
	style0_label_3.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_3.text.line_space = 2;

	para.moc.label_3 = lv_label_create(para.moc.button_3, NULL);
	lv_label_set_text(para.moc.label_3, "控件3");
	lv_label_set_align(para.moc.label_3, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_3, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_3, 0, 21);
	lv_obj_set_size(para.moc.label_3, 90, 29);
	lv_label_set_style(para.moc.label_3, LV_LABEL_STYLE_MAIN, &style0_label_3);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_4, &lv_style_btn_rel);
	style0_button_4.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_4.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_4.body.radius = 12;
	style0_button_4.body.border.width = 1;
	style0_button_4.body.shadow.width = 1;
	style0_button_4.body.padding.top = 0;
	style0_button_4.body.padding.bottom = 0;
	style0_button_4.body.padding.left = 0;

	lv_style_copy(&style1_button_4, &lv_style_btn_pr);
	style1_button_4.body.main_color = lv_color_hex(0x00aaff);
	style1_button_4.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_4.body.radius = 12;
	style1_button_4.body.border.width = 1;

	para.moc.button_4 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_4, 35 + SHOME_XI*(4-1), 15);
	lv_obj_set_size(para.moc.button_4, 90, 70);
	lv_btn_set_layout(para.moc.button_4, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_4, LV_BTN_STYLE_REL, &style0_button_4);
	lv_btn_set_style(para.moc.button_4, LV_BTN_STYLE_PR, &style1_button_4);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_4, &lv_style_transp);
	//style0_label_4.text.font = get_font_lib()->msyh_20;
	style0_label_4.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_4.text.line_space = 2;

	para.moc.label_4 = lv_label_create(para.moc.button_4, NULL);
	lv_label_set_text(para.moc.label_4, "控件4");
	lv_label_set_align(para.moc.label_4, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_4, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_4, 0, 21);
	lv_obj_set_size(para.moc.label_4, 90, 29);
	lv_label_set_style(para.moc.label_4, LV_LABEL_STYLE_MAIN, &style0_label_4);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_5, &lv_style_btn_rel);
	style0_button_5.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_5.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_5.body.radius = 12;
	style0_button_5.body.border.width = 1;
	style0_button_5.body.shadow.width = 1;
	style0_button_5.body.padding.top = 0;
	style0_button_5.body.padding.bottom = 0;
	style0_button_5.body.padding.left = 0;

	lv_style_copy(&style1_button_5, &lv_style_btn_pr);
	style1_button_5.body.main_color = lv_color_hex(0x00aaff);
	style1_button_5.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_5.body.radius = 12;
	style1_button_5.body.border.width = 1;

	para.moc.button_5 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_5, 35 + SHOME_XI*(5-1), 15);
	lv_obj_set_size(para.moc.button_5, 90, 70);
	lv_btn_set_layout(para.moc.button_5, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_5, LV_BTN_STYLE_REL, &style0_button_5);
	lv_btn_set_style(para.moc.button_5, LV_BTN_STYLE_PR, &style1_button_5);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_5, &lv_style_transp);
	//style0_label_5.text.font = get_font_lib()->msyh_20;
	style0_label_5.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_5.text.line_space = 2;

	para.moc.label_5 = lv_label_create(para.moc.button_5, NULL);
	lv_label_set_text(para.moc.label_5, "控件5");
	lv_label_set_align(para.moc.label_5, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_5, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_5, 0, 21);
	lv_obj_set_size(para.moc.label_5, 90, 29);
	lv_label_set_style(para.moc.label_5, LV_LABEL_STYLE_MAIN, &style0_label_5);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_6, &lv_style_btn_rel);
	style0_button_6.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_6.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_6.body.radius = 12;
	style0_button_6.body.border.width = 1;
	style0_button_6.body.shadow.width = 1;
	style0_button_6.body.padding.top = 0;
	style0_button_6.body.padding.bottom = 0;
	style0_button_6.body.padding.left = 0;

	lv_style_copy(&style1_button_6, &lv_style_btn_pr);
	style1_button_6.body.main_color = lv_color_hex(0x00aaff);
	style1_button_6.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_6.body.radius = 12;
	style1_button_6.body.border.width = 1;

	para.moc.button_6 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_6, 35 + SHOME_XI*(1-1), 15 + SHOME_YI*(2-1));
	lv_obj_set_size(para.moc.button_6, 90, 70);
	lv_btn_set_layout(para.moc.button_6, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_6, LV_BTN_STYLE_REL, &style0_button_6);
	lv_btn_set_style(para.moc.button_6, LV_BTN_STYLE_PR, &style1_button_6);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_6, &lv_style_transp);
	//style0_label_6.text.font = get_font_lib()->msyh_20;
	style0_label_6.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_6.text.line_space = 2;

	para.moc.label_6 = lv_label_create(para.moc.button_6, NULL);
	lv_label_set_text(para.moc.label_6, "控件6");
	lv_label_set_align(para.moc.label_6, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_6, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_6, 0, 21);
	lv_obj_set_size(para.moc.label_6, 90, 29);
	lv_label_set_style(para.moc.label_6, LV_LABEL_STYLE_MAIN, &style0_label_6);
#endif // LV_USE_LABEL

#ifdef LV_USE_BTN
	lv_style_copy(&style0_button_7, &lv_style_btn_rel);
	style0_button_7.body.main_color = lv_color_hex(0xe5d6dc);
	style0_button_7.body.grad_color = lv_color_hex(0xe5d6dc);
	style0_button_7.body.radius = 12;
	style0_button_7.body.border.width = 1;
	style0_button_7.body.shadow.width = 1;
	style0_button_7.body.padding.top = 0;
	style0_button_7.body.padding.bottom = 0;
	style0_button_7.body.padding.left = 0;

	lv_style_copy(&style1_button_7, &lv_style_btn_pr);
	style1_button_7.body.main_color = lv_color_hex(0x00aaff);
	style1_button_7.body.grad_color = lv_color_hex(0x00aaff);
	style1_button_7.body.radius = 12;
	style1_button_7.body.border.width = 1;

	para.moc.button_7 = lv_btn_create(parent, NULL);
	lv_obj_set_pos(para.moc.button_7, 35 + SHOME_XI*(2-1), 15 + SHOME_YI*(2-1));
	lv_obj_set_size(para.moc.button_7, 90, 70);
	lv_btn_set_layout(para.moc.button_7, LV_LAYOUT_OFF);
	lv_btn_set_style(para.moc.button_7, LV_BTN_STYLE_REL, &style0_button_7);
	lv_btn_set_style(para.moc.button_7, LV_BTN_STYLE_PR, &style1_button_7);
#endif // LV_USE_BTN

#ifdef LV_USE_LABEL
	lv_style_copy(&style0_label_7, &lv_style_transp);
	//style0_label_7.text.font = get_font_lib()->msyh_20;
	style0_label_7.text.font = &microsoft_yahei_en_cn_24_4;
	style0_label_7.text.line_space = 2;

	para.moc.label_7 = lv_label_create(para.moc.button_7, NULL);
	lv_label_set_text(para.moc.label_7, "控件7");
	lv_label_set_align(para.moc.label_7, LV_LABEL_ALIGN_CENTER);
	lv_label_set_long_mode(para.moc.label_7, LV_LABEL_LONG_CROP);
	lv_obj_set_pos(para.moc.label_7, 0, 21);
	lv_obj_set_size(para.moc.label_7, 90, 29);
	lv_label_set_style(para.moc.label_7, LV_LABEL_STYLE_MAIN, &style0_label_7);
#endif // LV_USE_LABEL

}

static void page1_create(lv_obj_t * parent)
{
	clean_page(parent);
	lv_page_set_scrl_fit4(parent, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_NONE, LV_FIT_NONE);

	page1_btn_create(parent);

	page1_parent = parent;
	line_1 = lv_line_create(parent, NULL);
	lv_obj_set_pos(line_1, 0, 0);
	lv_obj_set_size(line_1, SHOME_XI, SHOME_YI);
	lv_line_set_points(line_1, line_1_points, 5);

	lv_style_copy(&style0_line_1, &lv_style_pretty);
	style0_line_1.line.opa = 0;
	lv_line_set_style(line_1, LV_LINE_STYLE_MAIN, &style0_line_1);

	lv_obj_set_drag(para.moc.button_1, 1);
	lv_obj_set_event_cb(para.moc.button_1, button_event_cb);
	lv_obj_set_drag(para.moc.button_2, 1);
	lv_obj_set_event_cb(para.moc.button_2, button_event_cb);
	lv_obj_set_drag(para.moc.button_3, 1);
	lv_obj_set_event_cb(para.moc.button_3, button_event_cb);
	lv_obj_set_drag(para.moc.button_4, 1);
	lv_obj_set_event_cb(para.moc.button_4, button_event_cb);
	lv_obj_set_drag(para.moc.button_5, 1);
	lv_obj_set_event_cb(para.moc.button_5, button_event_cb);
	lv_obj_set_drag(para.moc.button_6, 1);
	lv_obj_set_event_cb(para.moc.button_6, button_event_cb);
	lv_obj_set_drag(para.moc.button_7, 1);
	lv_obj_set_event_cb(para.moc.button_7, button_event_cb);
}

static void page2_create(lv_obj_t * parent)
{
    lv_coord_t hres = lv_disp_get_hor_res(NULL);
	clean_page(parent);
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    /*Create styles for the buttons*/
    static lv_style_t style_btn_rel;
    static lv_style_t style_btn_pr;
    lv_style_copy(&style_btn_rel, &lv_style_btn_rel);
    style_btn_rel.body.main_color = lv_color_hex3(0x333);
    style_btn_rel.body.grad_color = LV_COLOR_BLACK;
    style_btn_rel.body.border.color = LV_COLOR_SILVER;
    style_btn_rel.body.border.width = 1;
    style_btn_rel.body.border.opa = LV_OPA_50;
    style_btn_rel.body.radius = 0;

    lv_style_copy(&style_btn_pr, &style_btn_rel);
    style_btn_pr.body.main_color = lv_color_make(0x55, 0x96, 0xd8);
    style_btn_pr.body.grad_color = lv_color_make(0x37, 0x62, 0x90);
    style_btn_pr.text.color = lv_color_make(0xbb, 0xd5, 0xf1);

    lv_obj_t * list = lv_list_create(parent, NULL);
	lv_obj_set_width(list, lv_obj_get_width(parent)/2);
    lv_obj_set_height(list, lv_obj_get_height(parent)*2/3);
    lv_list_set_style(list, LV_LIST_STYLE_BG, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_SCRL, &lv_style_transp_tight);
    lv_list_set_style(list, LV_LIST_STYLE_BTN_REL, &style_btn_rel);
    lv_list_set_style(list, LV_LIST_STYLE_BTN_PR, &style_btn_pr);
    lv_obj_align(list, NULL, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 4);

    lv_obj_t * list_btn;
    list_btn = lv_list_add_btn(list, LV_SYMBOL_FILE, "New");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_DIRECTORY, "Open");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_TRASH, "Delete");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_EDIT, "Edit");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_SAVE, "Save");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_WIFI, "WiFi");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    list_btn = lv_list_add_btn(list, LV_SYMBOL_GPS, "GPS");
    lv_obj_set_event_cb(list_btn, list_btn_event_handler);

    lv_obj_t * mbox = lv_mbox_create(parent, NULL);
    lv_mbox_set_text(mbox, "Click a button to copy its text to the Text area ");
    lv_obj_set_width(mbox, hres - LV_DPI);
    static const char * mbox_btns[] = {"Got it", ""};
    lv_mbox_add_btns(mbox, mbox_btns);    /*The default action is close*/
    lv_obj_align(mbox, parent, LV_ALIGN_IN_TOP_MID, 0, LV_DPI / 2);
}

static void page3_create(lv_obj_t * parent)
{
	clean_page(parent);
    lv_page_set_scrl_fit(parent, LV_FIT_NONE);
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    static lv_style_t style_ta;
    lv_style_copy(&style_ta, &lv_style_pretty);
    style_ta.body.opa = LV_OPA_30;
    style_ta.body.radius = 0;
    style_ta.text.color = lv_color_hex3(0x222);

    ta = lv_ta_create(parent, NULL);
    lv_obj_set_size(ta, lv_page_get_scrl_width(parent), lv_obj_get_height(parent) / 2);
    lv_ta_set_style(ta, LV_TA_STYLE_BG, &style_ta);
    lv_ta_set_text(ta, "");
    lv_obj_set_event_cb(ta, text_area_event_handler);
    lv_style_copy(&style_kb, &lv_style_plain);
    lv_ta_set_text_sel(ta, true);

    style_kb.body.opa = LV_OPA_70;
    style_kb.body.main_color = lv_color_hex3(0x333);
    style_kb.body.grad_color = lv_color_hex3(0x333);
    style_kb.body.padding.left = 0;
    style_kb.body.padding.right = 0;
    style_kb.body.padding.top = 0;
    style_kb.body.padding.bottom = 0;
    style_kb.body.padding.inner = 0;

    lv_style_copy(&style_kb_rel, &lv_style_plain);
    style_kb_rel.body.opa = LV_OPA_TRANSP;
    style_kb_rel.body.radius = 0;
    style_kb_rel.body.border.width = 1;
    style_kb_rel.body.border.color = LV_COLOR_SILVER;
    style_kb_rel.body.border.opa = LV_OPA_50;
    style_kb_rel.body.main_color = lv_color_hex3(0x333);    /*Recommended if LV_VDB_SIZE == 0 and bpp > 1 fonts are used*/
    style_kb_rel.body.grad_color = lv_color_hex3(0x333);
    style_kb_rel.text.color = LV_COLOR_WHITE;

    lv_style_copy(&style_kb_pr, &lv_style_plain);
    style_kb_pr.body.radius = 0;
    style_kb_pr.body.opa = LV_OPA_50;
    style_kb_pr.body.main_color = LV_COLOR_WHITE;
    style_kb_pr.body.grad_color = LV_COLOR_WHITE;
    style_kb_pr.body.border.width = 1;
    style_kb_pr.body.border.color = LV_COLOR_SILVER;
}

static void page4_create(lv_obj_t * parent)
{
	clean_page(parent);
}

void slide_home_moc_create(slide_home_para_t *para)
{
	para->moc.b_anim = 0;
	
	background_photo_init();
	para->moc.img_bgd = lv_img_create(para->ui.cont, NULL);
	lv_obj_set_pos(para->moc.img_bgd, 0, 0);
	lv_obj_set_size(para->moc.img_bgd, 800, 480);
	lv_img_set_src(para->moc.img_bgd, get_background_photo());
	lv_obj_move_background(para->moc.img_bgd);

	// tabview
	lv_coord_t hres = lv_disp_get_hor_res(NULL);
    lv_coord_t vres = lv_disp_get_ver_res(NULL);
    lv_obj_t * tv = lv_tabview_create(para->ui.cont, NULL);
	lv_obj_set_pos(tv, 0, 50);
    lv_obj_set_size(tv, hres, vres-50);
	lv_obj_set_event_cb(tv, tabview_event_cb);
	lv_tabview_set_btns_hidden(tv, true);

    static lv_style_t style_tv_btn_bg;
    lv_style_copy(&style_tv_btn_bg, &lv_style_plain);
    style_tv_btn_bg.body.main_color = lv_color_hex(0x487fb7);
    style_tv_btn_bg.body.grad_color = lv_color_hex(0x487fb7);
    style_tv_btn_bg.body.padding.top = 0;
    style_tv_btn_bg.body.padding.bottom = 0;
	style_tv_btn_bg.body.opa = 0;

	static lv_style_t style_tv_indic;
    lv_style_copy(&style_tv_indic, &lv_style_plain);
	style_tv_btn_bg.body.opa = 0;

    static lv_style_t style_tv_btn_rel;
    lv_style_copy(&style_tv_btn_rel, &lv_style_btn_rel);
    style_tv_btn_rel.body.opa = 0;
    style_tv_btn_rel.body.border.width = 0;

    static lv_style_t style_tv_btn_pr;
    lv_style_copy(&style_tv_btn_pr, &lv_style_btn_pr);
    style_tv_btn_pr.body.radius = 0;
    style_tv_btn_pr.body.opa = 0;
    style_tv_btn_pr.body.main_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.grad_color = LV_COLOR_WHITE;
    style_tv_btn_pr.body.border.width = 0;
    style_tv_btn_pr.text.color = LV_COLOR_GRAY;
	lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_BG, &style_tv_btn_bg);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_INDIC, &style_tv_indic);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_PR, &style_tv_btn_pr);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_REL, &style_tv_btn_rel);
    lv_tabview_set_style(tv, LV_TABVIEW_STYLE_BTN_TGL_PR, &style_tv_btn_pr);

	// page
	lv_obj_t * tab0 = lv_tabview_add_tab(tv, "0");
    lv_obj_t * tab1 = lv_tabview_add_tab(tv, "1");
    lv_obj_t * tab2 = lv_tabview_add_tab(tv, "2");
	lv_obj_t * tab3 = lv_tabview_add_tab(tv, "3");
	lv_obj_t * tab4 = lv_tabview_add_tab(tv, "4");
	page0_create(tab0);
    page1_create(tab1);
	page2_create(tab2);
    page3_create(tab3);
	page4_create(tab4);
	lv_tabview_set_tab_act(tv, 1, LV_ANIM_OFF);
	lv_obj_move_foreground(tv);
	lv_obj_set_event_cb(para->ui.btn_hbar_return, btn_hbar_return_event_cb);
	lv_obj_set_event_cb(para->ui.btn_hbar_home, btn_hbar_return_event_cb);
}

void slide_home_moc_destory(slide_home_para_t *para)
{
	para->moc.b_anim = 0;
	background_photo_uninit();
}

static int create_slide_home(void)
{
	para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(para.ui.cont, LV_FIT_NONE);

	slide_home_auto_ui_create(&para.ui);
	slide_home_moc_create(&para);

	return 0;
}

static int destory_slide_home(void)
{
	slide_home_moc_destory(&para);
	slide_home_auto_ui_destory(&para.ui);
	lv_obj_del(para.ui.cont);

	return 0;
}

static int show_slide_home(void)
{
	lv_obj_set_hidden(para.ui.cont, 0);

	return 0;
}

static int hide_slide_home(void)
{
	lv_obj_set_hidden(para.ui.cont, 1);

	return 0;
}

static int msg_proc_slide_home(MsgDataInfo *msg)
{
	return 0;
}

static int b_anim_end = 0;

static void slide_home_anim_end(lv_anim_t * a)
{
	b_anim_end = 1;
}

static int anim_slide_home(page_id_t old_page, page_id_t new_page)
{
	lv_anim_t a;
	b_anim_end = 0;
	
	if (old_page == PAGE_HOME && new_page == PAGE_SLIDE_HOME)
	{
	    a.var = para.ui.cont;
	    a.start = LV_HOR_RES_MAX;
	    a.end = 0;
	    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
	    a.path_cb = lv_anim_path_linear;
	    a.ready_cb = slide_home_anim_end;
	    a.act_time = 0;
	    a.time = 1000;
	    a.playback = 0;
	    a.playback_pause = 0;
	    a.repeat = 0;
	    a.repeat_pause = 0;
	    lv_anim_create(&a);
	}
	else if (old_page == PAGE_SLIDE_HOME && new_page == PAGE_HOME)
	{
	    a.var = para.ui.cont;
	    a.start = 0;
	    a.end = LV_HOR_RES_MAX;
	    a.exec_cb = (lv_anim_exec_xcb_t)lv_obj_set_x;
	    a.path_cb = lv_anim_path_linear;
	    a.ready_cb = slide_home_anim_end;
	    a.act_time = 0;
	    a.time = 1000;
	    a.playback = 0;
	    a.playback_pause = 0;
	    a.repeat = 0;
	    a.repeat_pause = 0;
	    lv_anim_create(&a);
	}
	else
	{
		com_warn("\n");
	}
	
	return 0;
}

static int is_anim_end_slide_home(void)
{
	return b_anim_end;
}

static page_interface_t page_slide_home =
{
	.ops =
	{
		create_slide_home,
		destory_slide_home,
		show_slide_home,
		hide_slide_home,
		msg_proc_slide_home,
		anim_slide_home,
		is_anim_end_slide_home,
	},
	.info =
	{
		.id         = PAGE_SLIDE_HOME,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_SLIDE_HOME(void)
{
	reg_page(&page_slide_home);
}
