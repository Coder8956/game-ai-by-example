//------------------------------------------------------------------------------
//  afxres.h  —  最小替代头（工程级修复，非源码改动）
//
//  原工程 Script1.rc 通过 #include "afxres.h" 引入资源编译符号，该头由 MFC
//  (atlmfc\include\afxres.h) 提供。本机 VS 2026 未安装 "适用于最新 v143 生成
//  工具的 C++ MFC" 组件，故于工程目录放置本最小桩头，内容对齐 MFC 版本中
//  资源脚本实际用到的部分：
//    - 引入 Windows SDK 的 winres.h（资源编译器默认包含路径已含 SDK um 目录）
//    - 提供 IDC_STATIC 等 MFC 常用符号
//  不影响任何 C/C++ 源码的编译。
//------------------------------------------------------------------------------
#ifndef __AFXRES_H__
#define __AFXRES_H__

#ifdef _WIN32
#include <winres.h>
#endif

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

#endif // __AFXRES_H__
