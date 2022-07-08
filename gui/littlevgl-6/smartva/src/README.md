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

// 目录说明
smartva：总共包括两个大层，common（公共层）和story（应用层）。
common：一般由全志工程师维护，屏蔽底层的差异，为story层提供统一的、易用的接口。
story：smartva下的一个应用（公版应用），客户可以定制添加n个类似的应用。
.
├── common
│	  ├── common.h			// 对外头文件，story层使用common及子模块，包含该头文件即可。
│	  ├── driver			// 驱动对接层
│     │      ├── va_audio.c		// 与audio驱动对接
│     │      ├── va_audio.h
│     │      ├── ...
│     │      ├── va_display.c	// 与显示驱动对接
│     │      ├── va_display.h
│     │      ├── ...
│     │      ├── ...
│	  ├── middle			// 中间件对接层，包括全志音视频、第三方库等对接。
│     │      ├── cjson			// cjson库
│     │      ├── dblist			// 双链表库
│     │      ├── diskmanager	// 磁盘管理库
│     │      ├── va_media		// 音视频中间件对接层
│     │      ├── va_photo		// 图片中间件对接层
│     │      ├── ...
│     │      ├── ...
├── story
│	  ├── business				// 业务层，与GUI无关的业务逻辑
│     │      ├── main.c			// 主函数入口
│     │      ├── ...
│     │      ├── ...
│	  ├── config				// 配置层
│     │      ├── lv_conf.h		// littlevgl核心库配置裁剪
│     │      ├── lv_drv_conf.h	// littlevgl驱动对接库配置裁剪
│     │      ├── smt_config.h	// smartva配置
│     │      ├── ...
│     │      ├── ...
│	  ├── designer				// gui图形设计层
│     │      ├── fonts			// 字体资源
│     │      ├── images			// 图片资源
│     │      ├── lvgl			// .lvgl可视化文件
│     │      ├── src			// 代码
│     │      │    ├── moc_home.c		// moc业务逻辑层，与GUI有关的业务逻辑，包含部分UI代码，
│	  │	     │    ├── moc_music.c		// moc业务逻辑层，工具创建产生代码模板一次，之后需要手动修改。
│	  │	     │	  ├── ui_home.c			// ui界面， 图形界面工具产生，不可以手动修改。
│	  │	     │    ├── ui_music.c
│	  │	     │    ├── ...
