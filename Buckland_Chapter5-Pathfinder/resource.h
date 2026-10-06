//==============================================================================================
//【文件说明】resource.h —— Windows 资源"符号编号表"
//
//【这个文件是干什么的?】
//  Windows 程序里,工具栏、菜单、图标都叫"资源",每个资源都有一个数字编号。
//  本文件用 #define 给这些数字起名字(IDR_TOOLBAR1、ID_BUTTON_ASTAR……),
//  让代码里写"名字"而不是写"裸数字 40011",读起来一目了然。
//
//【谁在使用这个文件?】
//  main.cpp    —— #include 它,在响应菜单/工具栏消息(WM_COMMAND)时,
//                  用这些 ID 判断用户点了哪个按钮(ID_BUTTON_DFS 等);
//  toolbar.rc  —— 资源脚本文件也 #include 它,把按钮图片、菜单与编号对应起来。
//
//【C++ 小课堂:#define 宏】
//  #define ID_BUTTON_ASTAR 40011 —— 预处理阶段把代码里所有 ID_BUTTON_ASTAR
//  原样替换成 40011。它只是"文本替换",不占内存,也不做类型检查。
//==============================================================================================
//(下面三行英文注释是 Visual Studio 资源编辑器自动生成的头,原样保留)
//(原文注释:{{NO_DEPENDENCIES}} —— 表示依赖扫描工具可忽略本文件)
//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by toolbar.rc
//
// ---- 资源(工具栏/菜单/图标)编号,100~199 段 ----
#define IDR_TOOLBAR1                    101
#define IDR_MENU1                       103
#define IDI_ICON1                       104
// ---- 工具栏按钮命令编号,40000 段 ----
// 用户点按钮时,Windows 把这个编号放进 WM_COMMAND 消息的 wParam 里,
// main.cpp 的 WindowProc 用 switch 判断是哪一个,再触发对应动作。
#define ID_BUTTON_STOP                  40001
#define ID_BUTTON_START                 40002
#define ID_BUTTON_OBSTACLE              40003
#define ID_BUTTON_WATER                 40004
#define ID_BUTTON_MUD                   40005
#define ID_BUTTON_END                   40006
//(注:ID_BUTTON_END 在本工程里没有对应按钮,是预留编号)
#define ID_BUTTON_NORMAL                40007
#define ID_BUTTON_DFS                   40008
#define ID_BUTTON_BFS                   40009
#define ID_BUTTON_DIJKSTRA              40010
#define ID_BUTTON_ASTAR                 40011
// ---- 菜单命令编号 ----
// VIEW_GRAPH/VIEW_TILES:菜单里"是否显示网络图/是否显示格子"两个勾选项;
#define IDM_VIEW_GRAPH                  40013
#define IDM_VIEW_TILES                  40014
#define ID_MENU_SAVEAS                  40019
#define ID_MENU_LOAD                    40020
#define ID_MENU_NEW                     40021
// SAVEAS/LOAD/NEW:菜单里的"另存为/打开/新建"地图命令。

// Next default values for new objects
// 
// ---- 下面是资源编辑器自用的"下次编号建议",程序运行时用不到 ----
// APSTUDIO_INVOKED 等宏只有在 Visual Studio 资源编辑器打开本文件时才定义;
// 普通编译时这段被 #ifdef 跳过,不会真的编进程序。
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        105
#define _APS_NEXT_COMMAND_VALUE         40015
#define _APS_NEXT_CONTROL_VALUE         1000
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif
