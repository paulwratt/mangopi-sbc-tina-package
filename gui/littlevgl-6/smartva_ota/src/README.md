/*
*********************************************************************************************************
*
*						            smartva project
*
*						        (c) Copyright 2020-2030 allwinnertech from China
*										All	Rights Reserved
*
* File    : smartva
* By      : huangyixiu
* Version : V0.1
*********************************************************************************************************
*/

// Ŀ¼˵��
smartva���ܹ�����������㣬common�������㣩��story��Ӧ�ò㣩��
common��һ����ȫ־����ʦά�������εײ�Ĳ��죬Ϊstory���ṩͳһ�ġ����õĽӿڡ�
story��smartva�µ�һ��Ӧ�ã�����Ӧ�ã����ͻ����Զ�������n�����Ƶ�Ӧ�á�
.
������ common
��	  ������ common.h			// ����ͷ�ļ���story��ʹ��common����ģ�飬������ͷ�ļ����ɡ�
��	  ������ com_porting.h		// ����ͷ�ļ����û��޸�common�㣬��Ҫ�޸ĸ��ļ����ɡ�
��	  ������ driver			// �����ԽӲ�
��     ��      ������ va_audio.c		// ��audio�����Խ�
��     ��      ������ va_audio.h
��     ��      ������ ...
��     ��      ������ va_display.c	// ����ʾ�����Խ�
��     ��      ������ va_display.h
��     ��      ������ ...
��     ��      ������ ...
��	  ������ middle			// �м���ԽӲ㣬����ȫ־����Ƶ����������ȶԽӡ�
��     ��      ������ cjson			// cjson��
��     ��      ������ dblist			// ˫������
��     ��      ������ diskmanager	// ���̹�����
��     ��      ������ va_media		// ����Ƶ�м���ԽӲ�
��     ��      ������ va_photo		// ͼƬ�м���ԽӲ�
��     ��      ������ ...
��     ��      ������ ...
������ story
��	  ������ business				// ҵ��㣬��GUI�޹ص�ҵ���߼�
��     ��      ������ main.c			// ���������
��     ��      ������ ...
��     ��      ������ ...
��	  ������ config				// ���ò�
��     ��      ������ lv_conf.h		// littlevgl���Ŀ����òü�
��     ��      ������ lv_drv_conf.h	// littlevgl�����Խӿ����òü�
��     ��      ������ ...
��     ��      ������ ...
��	  ������ designer				// guiͼ����Ʋ�
��     ��      ������ fonts			// ������Դ
��     ��      ������ images			// ͼƬ��Դ
��     ��      ������ lvgl			// .lvgl���ӻ��ļ�
��     ��      ������ src			// ����
��     ��      ��    ������ moc_home.c		// mocҵ���߼��㣬��GUI�йص�ҵ���߼�����������UI���룬
��	  ��	     ��    ������ moc_music.c		// mocҵ���߼��㣬���ߴ�����������ģ��һ�Σ�֮����Ҫ�ֶ��޸ġ�
��	  ��	     ��		  ������ ui_home.c			// ui���棬 ͼ�ν��湤�߲������������ֶ��޸ġ�
��	  ��	     ��    ������ ui_music.c
��	  ��	     ��    ������ ...