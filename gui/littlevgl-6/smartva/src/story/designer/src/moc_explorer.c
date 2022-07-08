/**********************
 *      includes
 **********************/
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include "moc_explorer.h"
#include "ui_explorer.h"
#include "lvgl.h"
#include "page.h"
#include "ui_resource.h"
#include "rat_common.h"
#include "DiskManager.h"
#include "media_mixture.h"
#include "lang.h"
#include "bs_widget.h"

/**********************
 *       variables
 **********************/
typedef struct
{
	rat_ctrl_t rat_ctrl;
	explorer_list_t *explorer_list;
	lv_obj_t *backImageBtn;
	explorer_ui_t ui;
} explorer_para_t;
static explorer_para_t Explorer_Para;
static lv_obj_t *label_dir_base = NULL;
static lv_task_t *auto_load_task = NULL;
//static lv_task_t *HotPlug_Process_task = NULL;
static hotplug_message_focus_win_t *RegisterInfo = NULL;
static pthread_mutex_t Explorer_HotPlug_Mutex;

static ui_image_t explorer_image_list[EXPLORER_IMAGE_NUM_MAX] = {
	{NULL, EXPLORER_IMAGE_AUDIO_PATH},
	{NULL, EXPLORER_IMAGE_VIDEO_PATH},
	{NULL, EXPLORER_IMAGE_PICTURE_PATH},
	{NULL, EXPLORER_IMAGE_FIRMWARE_PATH},
	{NULL, EXPLORER_IMAGE_FOLDER_PATH},
	{NULL, EXPLORER_IMAGE_TEXT_PATH},
	{NULL, EXPLORER_IMAGE_USB_PATH},
	{NULL, EXPLORER_IMAGE_SD_PATH},
	{NULL, EXPLORER_IMAGE_MORE_PATH},
	{NULL, EXPLORER_IMAGE_UNKOWN_PATH},
	{NULL, EXPLORER_RETURN_PATH}
};

static void Explorer_FileList_Update(void);
static void Explorer_FileList_event(lv_obj_t * btn, lv_event_t event);
static void Explorer_AutoLoad_Task(lv_task_cb_t *task);
static int Explorer_TypeAll_Update_Filelist(file_list_t* cur_list, int start, int end);

/**********************
 *  functions
 **********************/
static bool __is_in_obj(lv_obj_t * obj, __u32 x, __u32 y)
{
	lv_area_t  obj_area;
	lv_obj_get_coords(obj, &obj_area);

	if (x > obj_area.x2 || x < obj_area.x1 || y > obj_area.y2 || y < obj_area.y1)
	{
		return false;
	}
	else
	{
		return true;
	}
}
static void back_btn_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED) {
		//destory_page(PAGE_EXPLORER);
		//create_page(PAGE_HOME);
		switch_page(PAGE_EXPLORER, PAGE_HOME);
	}
}
static void Filesize2str(__u32 size, char *str) {
	// С��1 k
	if (size < 1024) {
		sprintf(str, "%d B",size);
	}
	// С�� 1 M
	else if ( size < (1024*1024)) {
		sprintf(str, "%d K",size/1024);
	}
	// С�� 1 G
	else if (size < (1024*1024*1024)) {
		sprintf(str, "%d.%d%d M",size/(1024*1024), ((size%(1024*1024))/1024)*1000/1024/100,((size%(1024*1024))/1024)*1000/1024%100/10);	//������λС��
	}
	else {
		sprintf(str, "%d.%d%d G",size/(1024*1024*1024),(size%(1024*1024*1024))/(1024*1024)*1000/1024/100,(size%(1024*1024*1024))/(1024*1024)*1000/1024%100/10);
	}
}
static __s32 mtime_to_time_string(time_t *mtime, char *string) {
	struct tm *p = NULL;
	__s32 Index = 0;

	p = gmtime(mtime);

	string[Index++] = (1900 + p->tm_year)/1000 + '0';
	string[Index++] = (1900 + p->tm_year)/100%10 + '0';
	string[Index++] = (1900 + p->tm_year)/10%10 + '0';
	string[Index++] = (1900 + p->tm_year)%10 + '0';

	string[Index++] = '-';
	string[Index++] = p->tm_mon/10 + '0';
	string[Index++] = p->tm_mon%10 + '0';

	string[Index++] = '-';
	string[Index++] = p->tm_mday/10 + '0';
	string[Index++] = p->tm_mday%10 + '0';

	string[Index++] = ' ';
	string[Index++] = p->tm_hour / 10 + '0';
	string[Index++] = p->tm_hour % 10 + '0';
	string[Index++] = ':';
	string[Index++] = p->tm_min / 10 + '0';
	string[Index++] = p->tm_min % 10 + '0';
	string[Index++] = ':';
	string[Index++] = p->tm_sec / 10 + '0';
	string[Index++] = p->tm_sec % 10 + '0';
	string[Index++] = '\0';
	return 0;
}
static void Explorer_Return_Home_Event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_CLICKED)
	{
		//create_page(PAGE_HOME);
		//destory_page(PAGE_EXPLORER);
		switch_page(PAGE_EXPLORER, PAGE_HOME);
	}
}

static void Explorer_Update_Directory(const char *FilePath)
{
	lv_label_set_text(label_dir_base, FilePath);
}

static void Explorer_Release_CurFileList(void)
{
	file_list_t *Temp = NULL;
	file_list_t *Child = Explorer_Para.explorer_list->top_file_list->child;
	while (Child != NULL) {
		Temp = Child;
		Child = Child->child;
		delete_file_list_nod(Temp);
	}
	Explorer_Para.explorer_list->top_file_list->child = NULL;
}

static void Explorer_Update_FileInfo(const char *FilePath)
{
	char str[50] = {0};
	struct stat file_stat;

	memset(&file_stat, 0x00, sizeof(struct stat));
	stat(FilePath, &file_stat);
	Filesize2str(file_stat.st_size, str);
	lv_label_set_text(Explorer_Para.ui.label_6, str);

	memset(str, 0x00, sizeof(str));
	mtime_to_time_string(&file_stat.st_mtime, str);
	lv_label_set_text(Explorer_Para.ui.label_8, str);
	return ;
}
static void Explorer_Back_Btn_Event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_RELEASED) {
		unsigned int start = 0, end = 0;

		if (Explorer_Para.explorer_list->cur_file_list->parent != Explorer_Para.explorer_list->top_file_list &&
			Explorer_Para.explorer_list->cur_file_list != Explorer_Para.explorer_list->top_file_list) {
			struct file_list_s  *parent = Explorer_Para.explorer_list->cur_file_list->parent;
//			delete_file_list_nod(Explorer_Para.explorer_list->cur_file_list);
			Explorer_Para.explorer_list->cur_file_list = parent;
			Explorer_Para.explorer_list->CurIsEmpty = 0;
		} else if (Explorer_Para.explorer_list->CurIsEmpty == 1){
			printf("do nothing\n");
			return ;
		} else {
			printf("this top dir\n");
			return ;
		}

		if (auto_load_task != NULL) {
			lv_task_del(auto_load_task);
			auto_load_task = NULL;
		}
		lv_list_clean(Explorer_Para.ui.list_2);
		start = lv_list_get_size(Explorer_Para.ui.list_2);
		end = ((start + EXPLORER_LIST_TASKLOAD_ITEM_NUM) > Explorer_Para.explorer_list->cur_file_list->total)?
				Explorer_Para.explorer_list->cur_file_list->total:(start + EXPLORER_LIST_TASKLOAD_ITEM_NUM);
		Explorer_TypeAll_Update_Filelist(Explorer_Para.explorer_list->cur_file_list, start, end);
		Explorer_Update_Directory(Explorer_Para.explorer_list->cur_file_list->file_path);
		if (end < Explorer_Para.explorer_list->cur_file_list->total) {
			auto_load_task = lv_task_create((void *)Explorer_AutoLoad_Task,100,LV_TASK_PRIO_MID,NULL);
		}
	}
}
static __s32 Explorer_Add_FileListItem(rat_media_type_t type, const char *FileName){
	lv_obj_t *img_type = NULL;
	lv_obj_t *btn_temp = NULL;
	lv_obj_t *label_name = NULL;
	lv_obj_t *img_more = NULL;
	const void * img_src = NULL;

//	printf("%s %d %s type:%d\n", __FILE__, __LINE__, __func__, type);
	switch(type) {
		case RAT_MEDIA_TYPE_PIC:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_PICTURE);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_AUDIO:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_AUDIO);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_VIDEO:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_VIDEO);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_FIRMWARE:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_FIRMWARE);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_EBOOK:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_TEXT);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_UNKNOWN:
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_UNKOWN);
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2, img_src, FileName);
			break;
		case RAT_MEDIA_TYPE_FOLDER:
			btn_temp = lv_list_add_btn(Explorer_Para.ui.list_2,NULL,NULL);
			/*�ر��Զ����֣���Ϊ�ֶ�����*/
			lv_btn_set_layout(btn_temp, LV_LAYOUT_OFF);
			img_type = lv_img_create(btn_temp,NULL);

			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_FOLDER);
			lv_img_set_src(img_type, img_src);
			lv_obj_align(img_type,NULL,LV_ALIGN_IN_LEFT_MID,24,0);

			img_more = lv_img_create(btn_temp,NULL);
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_MORE);
			lv_img_set_src(img_more, img_src);
			lv_obj_align(img_more,img_type,LV_ALIGN_OUT_RIGHT_MID,510,0);//������ͼ����Ϊ����Ŀ��

			label_name = lv_label_create(btn_temp,NULL);
			lv_label_set_text(label_name, FileName);
			lv_obj_align(label_name,img_type,LV_ALIGN_OUT_RIGHT_MID,12,0);
			break;
		default:
		break;
	}
	if (btn_temp) {
		lv_obj_set_event_cb(btn_temp,Explorer_FileList_event);
	} else {
		printf("%s %d %s btn_temp null\n", __FILE__, __LINE__, __func__);
	}
	return 0;
}
static int Explorer_TypeAll_Update_Filelist(file_list_t* cur_list, int start, int end)
{
	unsigned int i = 0;
	file_item_t *file_item = NULL;

	end = (end > cur_list->total)?cur_list->total:end;
	/*�˴�ֻ����2��ҳ���Ԫ�أ��������̼߳���ʣ��Ԫ�ػ��⻬����̬����*/
	for (i = start; i < end; i++) {
		file_item = get_file_list_item(cur_list, i);
		if (file_item != NULL) {
			rat_media_type_t type;

//			printf("%s %d %s file_item->name:%s\n", __FILE__, __LINE__, __func__, file_item->name);
			type = get_file_list_item_file_type(file_item);
			Explorer_Add_FileListItem(type, file_item->name);
		} else {
			printf("%s %d file_item == NULL\n", __FILE__, __LINE__);
		}

	}
	lv_label_set_text(Explorer_Para.ui.label_6,"");
	lv_label_set_text(Explorer_Para.ui.label_8,"");
	return 0;
}
static void Explorer_FileList_SwitchToApp(int Index) {
	int ret = EPDK_OK;
	__u32 i = 0, count = 0;
	rat_media_type_t media_type;
	char FileName[RAT_MAX_FULL_PATH_LEN] = {0};
	char FilePath[RAT_MAX_FULL_PATH_LEN + 16] = {0};
	DiskInfo_t DiskInfo, *disk_tmp = NULL;

	count = DiskManager_GetDiskNum();
	if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
		file_item_t* file_item = NULL;

		file_item = get_file_list_item(Explorer_Para.explorer_list->cur_file_list, Index);
		media_type = rat_get_file_type(file_item->name);
		strcpy(FileName, file_item->name);
		sprintf(FilePath, "%s/%s", Explorer_Para.explorer_list->cur_file_list->file_path, file_item->name);
	} else {
		rat_entry_t ItemInfo;
		extern char * SLIB_strchrlast(char * pstr, char srch_char);

		memset(&ItemInfo,0,sizeof(rat_entry_t));
		ret = rat_get_item_info_by_index(Explorer_Para.rat_ctrl.handle, Index, &ItemInfo);
		if (ret == EPDK_FAIL) {
			printf("get file information form rat fail!!");
			return ;
		}
		media_type = Explorer_Para.rat_ctrl.media_type;
		strcpy(FileName, SLIB_strchrlast(ItemInfo.Path, '/') + 1);
		strcpy(FilePath, ItemInfo.Path);
	}
	memset(&DiskInfo, 0x00, sizeof(DiskInfo_t));
	count = DiskManager_GetDiskNum();
	for (i = 0; i < count; i++) {
		disk_tmp = DiskManager_GetDiskInfoByIndex(i);
		if (disk_tmp == NULL) {
			printf("%s %d DiskManager_GetDiskInfoByIndex fail\n", __FILE__, __LINE__);
			continue;
		}
		if (strncmp(disk_tmp->MountPoint, FilePath, strlen(disk_tmp->MountPoint)) == 0) {
			memcpy(&DiskInfo, disk_tmp, sizeof(DiskInfo));
			break;
		}
	}
	if (i == count) {
		printf("DiskManager_GetDiskInfoByIndex fail!!");
		return ;
	}
	if((media_type == RAT_MEDIA_TYPE_AUDIO)
			|| (media_type == RAT_MEDIA_TYPE_VIDEO)
			|| (media_type == RAT_MEDIA_TYPE_PIC)
//			|| (media_type == RAT_MEDIA_TYPE_EBOOK)
			) {
		media_external_click(media_type, FilePath, &DiskInfo);
	} else if (media_type == RAT_MEDIA_TYPE_FIRMWARE) {
		extern int ota_check_file(char *file_path);
		if (ota_check_file(FilePath) == 0) {
			//destory_page(current_page());
			//create_page(PAGE_OTA);
			switch_page(current_page(), PAGE_OTA);
		}
	}
}
static void Explorer_FileList_event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_SHORT_CLICKED) {
		lv_indev_t * indev = lv_indev_get_act();
		/*����list�ڶ��˲��ܴ��������¼�������������жϵ��λ���Ƿ���Ԫ����*/
		if (__is_in_obj(btn,indev->proc.types.pointer.act_point.x,indev->proc.types.pointer.act_point.y)) {
			unsigned int Index = lv_list_get_btn_index(Explorer_Para.ui.list_2, btn);

			if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
				file_item_t *sel_item = get_file_list_item(Explorer_Para.explorer_list->cur_file_list, Index);
				if (sel_item->fatdirattr == FSYS_ATTR_DIRECTORY) {
					Explorer_Para.explorer_list->file_item = sel_item;
					Explorer_FileList_Update();
				}
				else//��������ļ����͸����ļ���Ϣ
				{
					char FilePath[RAT_MAX_FULL_PATH_LEN + 16] = {0};
					sprintf(FilePath, "%s/%s", Explorer_Para.explorer_list->cur_file_list->file_path, sel_item->name);
					Explorer_Update_FileInfo(FilePath);
					// do something,switch app
					Explorer_FileList_SwitchToApp(Index);
				}
			}else {
				int ret = 0;
				rat_entry_t ItemInfo;

				memset(&ItemInfo,0,sizeof(rat_entry_t));
				ret = rat_get_item_info_by_index(Explorer_Para.rat_ctrl.handle, Index, &ItemInfo);
				if (ret == EPDK_FAIL) {
					printf("get file information form rat fail!!");
					return ;
				}
				Explorer_Update_Directory(ItemInfo.Path);
				Explorer_Update_FileInfo(ItemInfo.Path);
				// do something,switch app
				Explorer_FileList_SwitchToApp(Index);
			}

		}
	}else if (LV_EVENT_LONG_PRESSED_REPEAT == event) {
		lv_indev_t * indev = lv_indev_get_act();
		if (__is_in_obj(btn,indev->proc.types.pointer.act_point.x,indev->proc.types.pointer.act_point.y)) {
			unsigned int Index = lv_list_get_btn_index(Explorer_Para.ui.list_2, btn);

			if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
				file_item_t *sel_item = get_file_list_item(Explorer_Para.explorer_list->cur_file_list, Index);
				if (sel_item->fatdirattr != FSYS_ATTR_DIRECTORY) {
					char FilePath[RAT_MAX_FULL_PATH_LEN + 16] = {0};
					sprintf(FilePath, "%s/%s", Explorer_Para.explorer_list->cur_file_list->file_path, sel_item->name);
					Explorer_Update_FileInfo(FilePath);
				}
			} else {
				int ret = 0;
				rat_entry_t ItemInfo;

				memset(&ItemInfo,0,sizeof(rat_entry_t));
				ret = rat_get_item_info_by_index(Explorer_Para.rat_ctrl.handle, Index, &ItemInfo);
				if (ret == EPDK_FAIL) {
					printf("get file information form rat fail!!");
					return ;
				}
				Explorer_Update_FileInfo(ItemInfo.Path);
			}

		}
	}
	return ;
}
static void Explorer_AutoLoad_Task(lv_task_cb_t *task)
{
	if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
		file_list_t* cur_list = Explorer_Para.explorer_list->cur_file_list;
		unsigned int list_btn_num = lv_list_get_size(Explorer_Para.ui.list_2), start = 0, end = 0;

		start = list_btn_num;
		end = ((list_btn_num + EXPLORER_LIST_TASKLOAD_ITEM_NUM) > cur_list->total)?cur_list->total:(list_btn_num + EXPLORER_LIST_TASKLOAD_ITEM_NUM);
		Explorer_TypeAll_Update_Filelist(cur_list, start, end);
		if (end == cur_list->total) {
			lv_task_del(auto_load_task);
			auto_load_task = NULL;
		}
	} else {
		if (!rat_is_cursor_end(Explorer_Para.rat_ctrl.handle)) {
			__u32 Index = lv_list_get_size(Explorer_Para.ui.list_2);
			rat_move_cursor_forward(Explorer_Para.rat_ctrl.handle, EXPLORER_LIST_TASKLOAD_ITEM_NUM);
			Explorer_Para.rat_ctrl.total = rat_get_cur_scan_cnt(Explorer_Para.rat_ctrl.handle);
//			printf("%s %d %s Explorer_Para.rat_ctrl.total:%d\n",__FILE__,__LINE__,__func__, Explorer_Para.rat_ctrl.total);
			for (; Index < Explorer_Para.rat_ctrl.total; Index++) {
				int ret = 0;
				rat_entry_t ItemInfo;
				extern char * SLIB_strchrlast(char * pstr, char srch_char);

				memset(&ItemInfo,0,sizeof(rat_entry_t));
				ret = rat_get_item_info_by_index(Explorer_Para.rat_ctrl.handle, Index, &ItemInfo);
				if (ret == EPDK_FAIL) {
					printf("get file information form rat fail!!");
					return ;
				}
				if (Index == 0) {
					Explorer_Update_Directory(ItemInfo.Path);
					Explorer_Update_FileInfo(ItemInfo.Path);
				}
//				printf("%s %d %s SLIB_strchrlast(ItemInfo.Path, '/') + 1:%s\n",__FILE__,__LINE__,__func__, SLIB_strchrlast(ItemInfo.Path, '/') + 1);
				Explorer_Add_FileListItem(Explorer_Para.rat_ctrl.media_type, SLIB_strchrlast(ItemInfo.Path, '/') + 1);
			}
		}else {
			lv_task_del(auto_load_task);
			auto_load_task = NULL;
		}
	}
}
static void Explorer_FileList_Update(void) {
	if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
		file_list_t *FileList = NULL;
		char  file_path[RAT_MAX_FULL_PATH_LEN+16] = {0};

		/*打开第一个设备，进入第一级目录*/
		if (Explorer_Para.explorer_list && Explorer_Para.explorer_list->cur_file_list &&
			Explorer_Para.explorer_list->cur_file_list->file_path &&
			strlen(Explorer_Para.explorer_list->cur_file_list->file_path) != 0) {
			sprintf(file_path, "%s/", Explorer_Para.explorer_list->cur_file_list->file_path);
		}
		if (Explorer_Para.explorer_list && Explorer_Para.explorer_list->file_item && Explorer_Para.explorer_list->file_item->name) {
//			printf("%s %d %s name:%s\n", __FILE__, __LINE__, __func__, Explorer_Para.explorer_list->file_item->name);
			strcat(file_path, Explorer_Para.explorer_list->file_item->name);
			FileList = new_file_list_nod(file_path, NULL);
			if (FileList != NULL) {
				unsigned int start = 0, end = 0;

				FileList->parent = Explorer_Para.explorer_list->cur_file_list;
				Explorer_Para.explorer_list->cur_file_list->child = FileList;
				Explorer_Para.explorer_list->cur_file_list = FileList;

				if (auto_load_task != NULL) {
					lv_task_del(auto_load_task);
					auto_load_task = NULL;
				}
				lv_list_clean(Explorer_Para.ui.list_2);
				start = lv_list_get_size(Explorer_Para.ui.list_2);
				end = ((start + EXPLORER_LIST_TASKLOAD_ITEM_NUM) > Explorer_Para.explorer_list->cur_file_list->total) ?
							Explorer_Para.explorer_list->cur_file_list->total:(start + EXPLORER_LIST_TASKLOAD_ITEM_NUM);
				Explorer_TypeAll_Update_Filelist(Explorer_Para.explorer_list->cur_file_list, start, end);
				//开始预加载任务
				if (end < Explorer_Para.explorer_list->cur_file_list->total) {
					auto_load_task = lv_task_create((void *)Explorer_AutoLoad_Task,100,LV_TASK_PRIO_MID,NULL);
				}
			} else {
				lv_list_clean(Explorer_Para.ui.list_2);
				Explorer_Para.explorer_list->CurIsEmpty = 1;
			}
			Explorer_Update_Directory(file_path);
		}else {
			lv_list_clean(Explorer_Para.ui.list_2);
			Explorer_Para.explorer_list->CurIsEmpty = 1;
		}
	} else {
		__u32 Index = 0;

		strcpy(Explorer_Para.rat_ctrl.SearchPath, Explorer_Para.explorer_list->file_item->name);
		Explorer_Para.rat_ctrl.handle = rat_open(Explorer_Para.rat_ctrl.SearchPath, Explorer_Para.rat_ctrl.media_type, EXPLORER_LIST_PERLOAD_ITEM_NUM); //全部搜索
		Explorer_Para.rat_ctrl.total = rat_get_cur_scan_cnt(Explorer_Para.rat_ctrl.handle);
		lv_list_clean(Explorer_Para.ui.list_2);
		for (Index = 0; Index < Explorer_Para.rat_ctrl.total; Index++) {
			int ret = 0;
			rat_entry_t ItemInfo;
			extern char * SLIB_strchrlast(char * pstr, char srch_char);

			memset(&ItemInfo,0,sizeof(rat_entry_t));
			ret = rat_get_item_info_by_index(Explorer_Para.rat_ctrl.handle, Index, &ItemInfo);
			if (ret == EPDK_FAIL) {
				printf("get file information form rat fail!!");
				return ;
			}
			if (Index == 0) {
				Explorer_Update_Directory(ItemInfo.Path);
				Explorer_Update_FileInfo(ItemInfo.Path);
			}
			Explorer_Add_FileListItem(Explorer_Para.rat_ctrl.media_type, SLIB_strchrlast(ItemInfo.Path, '/') + 1);
		}
		if (!rat_is_cursor_end(Explorer_Para.rat_ctrl.handle)) {
			auto_load_task = lv_task_create((void *)Explorer_AutoLoad_Task,100,LV_TASK_PRIO_MID,NULL);
		}
	}
}
static void Explorer_MediumList_Update(void) {
	if (Explorer_Para.rat_ctrl.media_type == RAT_MEDIA_TYPE_ALL) {
		Explorer_Release_CurFileList();
	}else {
		if (Explorer_Para.rat_ctrl.handle) {
			rat_close(Explorer_Para.rat_ctrl.handle);
			Explorer_Para.rat_ctrl.handle = 0;
		}
	}
	Explorer_Para.explorer_list->cur_file_list = Explorer_Para.explorer_list->top_file_list;
	Explorer_FileList_Update();
}

static void Explorer_MediumList_Event(lv_obj_t * btn, lv_event_t event)
{
	if (event == LV_EVENT_SHORT_CLICKED) {
		lv_indev_t * indev = lv_indev_get_act();
		/*����list�ڶ��˲��ܴ��������¼�������������жϵ��λ���Ƿ���Ԫ����*/
		if(__is_in_obj(btn, indev->proc.types.pointer.act_point.x, indev->proc.types.pointer.act_point.y))
		{
			unsigned int CurItem = 0, CurFocusItem = 0;
			lv_obj_t *CurFocus_btn_obj = NULL;

			if (auto_load_task) {
				lv_task_del(auto_load_task);
				auto_load_task = NULL;
			}
			CurFocus_btn_obj = lv_list_get_btn_selected(Explorer_Para.ui.medium_list_1);
			CurFocusItem = lv_list_get_btn_index(Explorer_Para.ui.medium_list_1, CurFocus_btn_obj);
			CurItem = lv_list_get_btn_index(Explorer_Para.ui.medium_list_1, btn);
			if (CurFocusItem == CurItem) {
				printf("not need to update\n");
				return ;
			}
			lv_list_set_btn_selected(Explorer_Para.ui.medium_list_1, btn);
			Explorer_Para.explorer_list->file_item = get_file_list_item(Explorer_Para.explorer_list->top_file_list, CurItem);
			Explorer_MediumList_Update();
		}
	}
}
static void Explorer_MediumList_AddItem(unsigned int FocusItem, unsigned int Index) {
	file_item_t *file_item = NULL;
	rat_media_type_t type;
	lv_obj_t *btn_tmp = NULL;
	const void * img_src = NULL;
	int string_id = 0;

	file_item = get_file_list_item(Explorer_Para.explorer_list->top_file_list, Index);//��node��������Ϣ
	type = get_file_list_item_file_type(file_item);
	switch(type) {
		case RAT_MEDIA_TYPE_SD_DEVICE:
			string_id = LANG_EXPLORER_SDCARD;
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_SD);
			break;
		case RAT_MEDIA_TYPE_USB_DEVICE:
			string_id = LANG_EXPLORER_U_DISK;
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_USB);
			break;
		case RAT_MEDIA_TYPE_LOCAL_DEVICE:
			string_id = LANG_EXPLORER_LOCALDISK;
			img_src = get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_IMAGE_SD);
			break;
		default:
		break;
	}
	btn_tmp = lv_list_add_btn(Explorer_Para.ui.medium_list_1, img_src, get_text_by_id(string_id));
#if CONFIG_FONT_ENABLE
	lv_obj_t *list_lable = NULL;
	static lv_style_t style0_label_1;

	list_lable = lv_list_get_btn_label(btn_tmp);
	lv_style_copy(&style0_label_1, &lv_style_transp);
	style0_label_1.text.color = lv_color_hex(0x000000);
	style0_label_1.text.line_space = 2;
	style0_label_1.text.font = get_font_lib()->msyh_16;
	lv_label_set_style(list_lable, LV_LABEL_STYLE_MAIN, &style0_label_1);
#endif
	if (FocusItem == Index) {
		lv_obj_t *btn_obj = NULL;

		btn_obj = lv_list_get_btn_selected(Explorer_Para.ui.medium_list_1);
		if (btn_obj) {
			lv_btn_set_state(btn_obj, LV_BTN_STATE_REL);
		}
		Explorer_Para.explorer_list->file_item = get_file_list_item(Explorer_Para.explorer_list->top_file_list, FocusItem);
		lv_list_set_btn_selected(Explorer_Para.ui.medium_list_1, btn_tmp);
	}
	lv_obj_set_event_cb(btn_tmp, Explorer_MediumList_Event);
	Explorer_Para.explorer_list->file_item = get_file_list_item(Explorer_Para.explorer_list->top_file_list, FocusItem);
}
static void Explorer_MediumList_DelItem(unsigned int Index) {
	int focus_index = 0;
	lv_obj_t *current_obj = NULL;
	lv_list_remove(Explorer_Para.ui.medium_list_1, Index);

	current_obj = lv_list_get_btn_selected(Explorer_Para.ui.medium_list_1);
	focus_index = lv_list_get_btn_index(Explorer_Para.ui.medium_list_1, current_obj);
	Explorer_Para.explorer_list->file_item = get_file_list_item(Explorer_Para.explorer_list->top_file_list, focus_index);
}

static void Explorer_MediumList_Init(char *MountPoint) {
	unsigned int Index = 0, FocusItem = -1;

	Explorer_Para.explorer_list = (explorer_list_t*)malloc(sizeof(explorer_list_t));
	memset(Explorer_Para.explorer_list,0,sizeof(explorer_list_t));
	/*Left list*/
	Explorer_Para.explorer_list->top_file_list = new_file_root_list_nod(NULL);
	if (MountPoint) {
		FocusItem = FileList_GetItemNum_ByMountPoint(Explorer_Para.explorer_list->top_file_list, MountPoint);
	} else {
		FocusItem = 0;
	}
	if (Explorer_Para.explorer_list->top_file_list) {
		for (Index = 0; Index < Explorer_Para.explorer_list->top_file_list->total; Index++) {
			Explorer_MediumList_AddItem(FocusItem, Index);
		}
		Explorer_Para.explorer_list->cur_file_list = Explorer_Para.explorer_list->top_file_list;
	}
	Explorer_FileList_Update();
	return ;
}
static void Explorer_DiskHotPlugCallBack(DiskInfo_t *DiskInfo) {
	pthread_mutex_lock(&Explorer_HotPlug_Mutex);
	if (DiskInfo->operate == MEDIUM_PLUGIN) {
		FileList_AddFileNod_To_RootList(Explorer_Para.explorer_list->top_file_list, DiskInfo);
		Explorer_MediumList_AddItem(Explorer_Para.explorer_list->top_file_list->total - 1, Explorer_Para.explorer_list->top_file_list->total - 1);
		Explorer_MediumList_Update();
	} else {
		lv_obj_t *CurFocus_btn_obj = NULL, *NewFocus_btn_obj = NULL;
		int ItemNum = 0, CurFocusItem = 0;

		ItemNum = FileList_GetItemNum_ByMountPoint(Explorer_Para.explorer_list->top_file_list, DiskInfo->MountPoint);
		CurFocus_btn_obj = lv_list_get_btn_selected(Explorer_Para.ui.medium_list_1);
		CurFocusItem = lv_list_get_btn_index(Explorer_Para.ui.medium_list_1, CurFocus_btn_obj);
		if (ItemNum == CurFocusItem) {
			if (CurFocusItem == 0) {
				NewFocus_btn_obj = lv_list_get_next_btn(Explorer_Para.ui.medium_list_1, CurFocus_btn_obj);
			} else {
				NewFocus_btn_obj = lv_list_get_prev_btn(Explorer_Para.ui.medium_list_1, CurFocus_btn_obj);
			}
			if (NewFocus_btn_obj) {
				lv_list_set_btn_selected(Explorer_Para.ui.medium_list_1, NewFocus_btn_obj);
			} else {
				printf("not disk to explorer will return to home");
				pthread_mutex_unlock(&Explorer_HotPlug_Mutex);
				switch_page(PAGE_HOME, PAGE_EXPLORER);
				return ;
			}
		}
		FileList_DelFileNod_To_RootList(Explorer_Para.explorer_list->top_file_list, DiskInfo);
		Explorer_MediumList_DelItem(ItemNum);
		Explorer_Para.explorer_list->cur_file_list = Explorer_Para.explorer_list->top_file_list;
		Explorer_MediumList_Update();
	}
	pthread_mutex_unlock(&Explorer_HotPlug_Mutex);
}
#if CONFIG_FONT_ENABLE
static void explorer_label_text_init(explorer_ui_t *ui)
{
	static lv_style_t style0_label_2;
	lv_style_copy(&style0_label_2, &lv_style_transp);
	style0_label_2.text.color = lv_color_hex(0x000000);
	style0_label_2.text.line_space = 2;
	style0_label_2.text.font = get_font_lib()->msyh_16;

	lv_label_set_text(ui->label_2, get_text_by_id(LANG_EXPLORER_MEDIUM));
	lv_label_set_text(ui->label_5, get_text_by_id(LANG_HOME_EXPLORER));
	lv_label_set_text(ui->label_1, get_text_by_id(LANG_EXPLORER_INFOMATION));
	lv_label_set_text(ui->label_4, get_text_by_id(LANG_EXPLORER_FILE_SIZE));
	lv_label_set_text(ui->label_7, get_text_by_id(LANG_EXPLORER_FILE_MODIFIED_TIME));

	lv_label_set_style(ui->label_2, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(ui->label_5, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(ui->label_1, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(ui->label_4, LV_LABEL_STYLE_MAIN, &style0_label_2);
	lv_label_set_style(ui->label_7, LV_LABEL_STYLE_MAIN, &style0_label_2);
}
#endif

static int create_explorer(void)
{
//	rat_media_type_t Type = RAT_MEDIA_TYPE_EBOOK;
	rat_media_type_t Type = RAT_MEDIA_TYPE_ALL;

	memset(&Explorer_Para, 0x00, sizeof(explorer_para_t));
	Explorer_Para.ui.cont = lv_cont_create(lv_scr_act(), NULL);
	lv_obj_set_size(Explorer_Para.ui.cont, LV_HOR_RES_MAX, LV_VER_RES_MAX);
	static lv_style_t cont_style;
	lv_style_copy(&cont_style, &lv_style_plain);
	cont_style.body.main_color = LV_COLOR_BLUE;
	cont_style.body.grad_color = LV_COLOR_BLUE;
	lv_cont_set_style(Explorer_Para.ui.cont, LV_CONT_STYLE_MAIN, &cont_style);
	lv_cont_set_layout(Explorer_Para.ui.cont, LV_LAYOUT_OFF);
	lv_cont_set_fit(Explorer_Para.ui.cont, LV_FIT_NONE);

	static lv_style_t back_btn_style;
	lv_style_copy(&back_btn_style, &lv_style_pretty);
	back_btn_style.text.font = &lv_font_roboto_28;
	lv_obj_t *back_btn = lv_btn_create(Explorer_Para.ui.cont, NULL);
	lv_obj_align(back_btn, Explorer_Para.ui.cont, LV_ALIGN_IN_TOP_LEFT, 0, 0);
	lv_obj_t *back_lable = lv_label_create(back_btn, NULL);
	lv_label_set_text(back_lable, LV_SYMBOL_LEFT);
	lv_obj_set_event_cb(back_btn, back_btn_event);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_REL, &back_btn_style);
	lv_btn_set_style(back_btn, LV_BTN_STYLE_PR, &back_btn_style);

	explorer_auto_ui_create(&Explorer_Para.ui);
#if CONFIG_FONT_ENABLE
	explorer_label_text_init(&Explorer_Para.ui);
#endif
	lv_list_set_sb_mode(Explorer_Para.ui.list_2, LV_SB_MODE_AUTO);

	//return imgbtn
	lv_obj_set_click(Explorer_Para.ui.image_1, true);
	lv_obj_set_ext_click_area(Explorer_Para.ui.image_1, 27,27,27,27);
	lv_obj_set_event_cb(Explorer_Para.ui.image_1, Explorer_Return_Home_Event);

	label_dir_base = lv_label_create(Explorer_Para.ui.direct_cont,NULL);
	lv_label_set_long_mode(label_dir_base, LV_LABEL_LONG_SROLL_CIRC);
	lv_obj_set_pos(label_dir_base, 42, 10);
	lv_obj_set_size(label_dir_base, 519, 22);
	//back imgbtn
	if (Type == RAT_MEDIA_TYPE_ALL) {
#ifdef LV_USE_IMG
		Explorer_Para.backImageBtn = lv_img_create(Explorer_Para.ui.cont, NULL);
		lv_obj_set_pos(Explorer_Para.backImageBtn, 763, 59);
		lv_obj_set_size(Explorer_Para.backImageBtn, 32, 32);
		lv_img_set_src(Explorer_Para.backImageBtn, get_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX, EXPLORER_BACK_IMAGE));
		lv_obj_set_click(Explorer_Para.backImageBtn, true);
		lv_obj_set_ext_click_area(Explorer_Para.backImageBtn, 27,27,27,27);
		lv_obj_set_event_cb(Explorer_Para.backImageBtn, Explorer_Back_Btn_Event);
#endif // LV_USE_IMG
	}

	Explorer_Para.rat_ctrl.media_type = Type;
	pthread_mutex_init(&Explorer_HotPlug_Mutex, NULL);
	RegisterInfo = malloc(sizeof(hotplug_message_focus_win_t));
	if (RegisterInfo != NULL) {
		memset(RegisterInfo, 0x00, sizeof(hotplug_message_focus_win_t));
		strcpy(RegisterInfo->Cur_Win, "explorer");
		RegisterInfo->CallBackFunction = Explorer_DiskHotPlugCallBack;
		DiskManager_Register(RegisterInfo);
	}
	if (DiskManager_GetDiskNum() == 0) {
		printf("not disk to explorer will return to home\n");
		//create_page(PAGE_HOME);
		//destory_page(PAGE_EXPLORER);
		switch_page(PAGE_HOME, PAGE_EXPLORER);
		return 0;
	}
	Explorer_MediumList_Init(NULL);
	return 0;
}

static int destory_explorer(void)
{
	if (RegisterInfo != NULL) {
		DiskManager_UnRegister(RegisterInfo);
	}
    pthread_mutex_destroy(&Explorer_HotPlug_Mutex);
	explorer_auto_ui_destory(&Explorer_Para.ui);
	lv_obj_del(Explorer_Para.ui.cont);
	if (Explorer_Para.backImageBtn) {
		lv_obj_del(Explorer_Para.backImageBtn);
	}
	if (label_dir_base) {
		lv_obj_del(label_dir_base);
	}
	if (auto_load_task) {
		lv_task_del(auto_load_task);
		auto_load_task = NULL;
	}

	if (Explorer_Para.explorer_list != NULL)
	{
		if (Explorer_Para.explorer_list->top_file_list != NULL) {
			delete_file_list_chain(Explorer_Para.explorer_list->top_file_list);
		}
		Explorer_Para.explorer_list->cur_file_list = NULL;
		Explorer_Para.explorer_list->top_file_list = NULL;
		free(Explorer_Para.explorer_list);
	}
	if (Explorer_Para.rat_ctrl.handle) {
		rat_close(Explorer_Para.rat_ctrl.handle);
		Explorer_Para.rat_ctrl.handle = 0;
	}

	free_image_buff_form_list(explorer_image_list, EXPLORER_IMAGE_NUM_MAX);
	return 0;
}

static int show_explorer(void)
{
	lv_obj_set_hidden(Explorer_Para.ui.cont, 0);

	return 0;
}

static int hide_explorer(void)
{
	lv_obj_set_hidden(Explorer_Para.ui.cont, 1);

	return 0;
}

static int msg_proc_explorer(MsgDataInfo *msg)
{
	return 0;
}

static page_interface_t page_explorer =
{
	.ops =
	{
		create_explorer,
		destory_explorer,
		show_explorer,
		hide_explorer,
		msg_proc_explorer,
	},
	.info =
	{
		.id         = PAGE_EXPLORER,
		.user_data  = NULL
	}
};

void REGISTER_PAGE_EXPLORER(void)
{
	reg_page(&page_explorer);
}
