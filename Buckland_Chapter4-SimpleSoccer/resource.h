//==============================================================================================
//【文件说明】resource.h —— 菜单/图标的"资源编号表"(注意:不是游戏 AI 源码)
//
//【这个文件是干什么的?】
//  Windows 程序的菜单、图标等"资源"都用一个整数编号来指代。本文件由 Visual Studio
//  资源编辑器自动生成,把这些编号起成见名知意的宏名(如 IDM_SHOW_STATES=40002)。
//  Script1.rc(资源脚本)和 main.cpp(响应菜单命令)都靠这些宏名引用同一个编号。
//
//【谁在使用这个文件?】
//  Script1.rc —— 资源脚本包含它,把菜单/图标按编号登记进去;
//  main.cpp  —— 收到菜单点击消息(WM_COMMAND)时,用这些宏名判断用户点了哪一项。
//
//【C++ 小课堂:#define 宏(纯文字替换)】
//  #define IDR_MENU1 101 意思是:编译前,凡是代码里写 IDR_MENU1 的地方,
//  一律先替换成 101 这个数字。它不占内存、不是变量,只是"起别名"。
//  命名约定:IDR_=菜单/图标等资源,IDI_=图标,IDM_/ID_=菜单项命令编号。
//==============================================================================================
//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by Script1.rc
//(原文注释:Microsoft Developer Studio 自动生成的包含文件,供 Script1.rc 使用)
//
// 下面一组宏:给菜单、图标和各个菜单项分配固定编号。
//  IDR_MENU1=主菜单;IDI_ICON1=程序图标;IDM_*=各菜单项(显示ID/状态/区域/辅助信息等开关)。
#define IDR_MENU1                       101
#define IDI_ICON1                       102
#define IDM_SHOW_IDS                    40001
#define IDM_SHOW_STATES                 40002
#define IDM_SHOW_REGIONS                40003
#define IDM_AIDS_SUPPORTSPOTS           40005
#define ID_AIDS_SHOWTARGETS             40006
#define ID_AIDS_NOAIDS                  40007
#define IDM_AIDS_HIGHLITE               40008

// Next default values for new objects
// 
// 下面这段是资源编辑器(APStudio)专用的"下次自动编号"提示,游戏运行时用不到;
// 只有在 Visual Studio 里打开资源编辑器时(APSTUDIO_INVOKED 被定义)才生效。
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        104
#define _APS_NEXT_COMMAND_VALUE         40009
#define _APS_NEXT_CONTROL_VALUE         1000
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
