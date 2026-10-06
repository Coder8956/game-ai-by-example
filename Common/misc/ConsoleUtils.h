//==============================================================================================
//【文件说明】ConsoleUtils.h —— Windows 控制台的两个小工具(改颜色/等按键)
//
//【谁在使用这个文件?】
//  第 2 章 WestWorld1、第 5 章 Pathfinder 等命令行演示程序(暂停按任意键)。
//
//【本文件包含了谁?】
//  <windows.h>   —— Windows API(GetStdHandle/SetConsoleTextAttribute);
//  <conio.h>     —— _kbhit()(检测键盘是否按下);
//  <iostream>    —— cout 打印。
//==============================================================================================
#ifndef CONSOLE_UTILS_H
#define CONSOLE_UTILS_H
//--------------------------------------------------------------------------------
// 包含保护原理详见 Buckland_Chapter4-SimpleSoccer/Goal.h。
//--------------------------------------------------------------------------------
//------------------------------------------------------------------------
//
//  Name:   ConsoleUtils.h
//
//  Desc:   Just a few handy utilities for dealing with consoles
//
//  Author: Mat Buckland (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <windows.h>
#include <conio.h>
#include <iostream>

//default text colors can be found in wincon.h
//--------------------------------------------------------------------------------
// SetTextColor:拿到控制台输出句柄 HANDLE,用 SetConsoleTextAttribute 改文字颜色。
//--------------------------------------------------------------------------------
inline void SetTextColor(WORD colors)
{
  HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
  
  SetConsoleTextAttribute(hConsole, colors);
}

//--------------------------------------------------------------------------------
// PressAnyKeyToContinue:把颜色设成白色,打印"按任意键继续",然后空转等 _kbhit() 检测到按键。
//--------------------------------------------------------------------------------
inline void PressAnyKeyToContinue()
{
  //change text color to white
  SetTextColor(FOREGROUND_BLUE| FOREGROUND_RED | FOREGROUND_GREEN);

  std::cout << "\n\nPress any key to continue" << std::endl; 

  while (!_kbhit()){}

  return;
}


#endif