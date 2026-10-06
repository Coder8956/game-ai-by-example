//==============================================================================================
//【文件说明】resource.h —— 菜单命令的「编号表」(由 Visual Studio 资源编辑器自动生成)
//
//【这个文件是干什么的?】
//  Windows 程序的菜单项(如「加载地图」「暂停」「显示导航图」……)每个都有一个
//  数字编号。用户点菜单时,Windows 把这个编号发给程序,程序就知道该干啥。
//  本文件用 #define 把这些数字起成看得懂的名字(IDM_GAME_PAUSE 等),
//  这样代码里写 case IDM_GAME_PAUSE 而不是裸写 40017。
//
//【谁在使用这个文件?】
//  main.cpp 与 Raven_Game.cpp:窗口过程(WndProc)收到菜单命令编号时,
//  用这里定义的名字做 switch 判断(例如收到 40017 → 对应 IDM_GAME_PAUSE → 暂停)。
//  另外 Script1.rc(资源脚本)也引用这些编号来布置菜单。
//
//【本文件包含了谁?】
//  什么都没包含——纯 #define 宏定义表。
//
//【C++ 小课堂:#define 宏】
//   #define IDR_MENU1 101
//   预处理阶段:凡是代码里出现 IDR_MENU1,就被原样替换成 101。
//   它不是变量、不占内存,只是个「文本替换规则」。
// 注意:本文件由 VS 资源向导自动维护,一般不要手工改,我们只加注释。
//==============================================================================================
//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by Script1.rc
//(原文注释翻译:被 Script1.rc 资源脚本使用)
//
//--------------------------------------------------------------------------------
// 下面一大串 #define 都是菜单/资源命令编号。含义速记:
//   IDR_MENU1            = 菜单资源编号(101);
//   IDM_NAVIGATION_*     = 导航相关菜单:显示导航图/显示路径/路径平滑(快速/精确);
//   IDM_BOTS_SHOW_*      = 机器人显示开关:编号/血条/目标/视野锥/分数/目标队列/感知;
//   IDM_MAP_ADD/REMOVEBOT= 在地图上加/删一个机器人;
//   IDM_GAME_PAUSE       = 暂停游戏。
// 末尾 4 行 #ifdef APSTUDIO_INVOKED 是 VS 资源向导的「下次自动编号提示」,
// 只有用资源编辑器打开工程时才生效,程序运行时无实际作用。
//--------------------------------------------------------------------------------
#define IDR_MENU1                       101
#define ID_MENU_LOAD                    40001
#define IDM_MAP_LOAD                    40001
#define IDM_GAME_LOAD                   40001
#define IDM_NAVIGATION_SHOW_NAVGRAPH    40002
#define IDM_NAVIGATION_SHOW_PATH        40004
#define IDM_NAVIGATION_SMOOTH_PATHS_QUICK 40005
#define IDM_BOTS_SHOW_IDS               40006
#define IDM_BOTS_SHOW_HEALTH            40007
#define IDM_BOTS_SHOW_TARGET            40008
#define IDM_BOTS_SHOW_FOV               40009
#define IDM_BOTS_SHOW_SCORES            40010
#define IDM_BOTS_SHOW_GOAL_Q            40011
#define IDM_NAVIGATION_SHOW_INDICES     40012
#define IDM_MAP_ADDBOT                  40013
#define IDM_GAME_ADDBOT                 40013
#define IDM_MAP_REMOVEBOT               40014
#define IDM_GAME_REMOVEBOT              40014
#define IDM_NAVIGATION_SMOOTH_PATHS_PRECISE 40015
#define IDM_BOTS_SHOW_SENSED            40016
#define IDM_GAME_PAUSE                  40017

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        111
#define _APS_NEXT_COMMAND_VALUE         40018
#define _APS_NEXT_CONTROL_VALUE         1000
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
