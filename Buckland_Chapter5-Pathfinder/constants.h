//==============================================================================================
//【文件说明】constants.h —— Pathfinder 工程的"全局尺寸常数表"
//
//【这个文件是干什么的?】
//  本文件只定义一堆 const int 常量,集中描述窗口大小、网格格子数等"硬编码数字"。
//  把数字集中写在这里(而不是散落在代码里),以后想改窗口尺寸、改格子数,
//  只改这一个文件即可,不用满工程找数字。
//
//【谁在使用这个文件?】
//  main.cpp        —— 创建窗口时用 WindowWidth/WindowHeight,排布客户区时用
//                     ClientWidth/ClientHeight/InfoWindowHeight,建新图时用 NumCellsX/Y;
//  Pathfinder.cpp  —— CreateGraph 里用 NumCellsX/Y 建网格,并在算客户区高度时
//                     减去 InfoWindowHeight 给底部计时信息条留位置。
//
//【C++ 小课堂:const 常量】
//  const int WindowWidth = 600; —— const 是"只读常量":这个整数一旦赋值就不许再改,
//  编译器会帮你守住。用它代替"魔法数字"(裸写的 600),代码更易懂、更好维护。
//==============================================================================================
//--------------------------------------------------------------------------------
//【包含保护 / include guard】#ifndef ... #define ... #endif 三行是头文件标准开场白,
// 防止本文件被同一条包含链复制两次而报"重复定义"。(详解见 WestWorld1/Locations.h)
//--------------------------------------------------------------------------------
#ifndef CONSTANTS_H
#define CONSTANTS_H


// ---- 窗口整体尺寸(单位:像素)----
// WindowWidth/Height:整个程序窗口(含标题栏、边框)的宽与高,固定 600x600。
const int WindowWidth  = 600;
const int WindowHeight = 600;

// ClientWidth/Height:窗口里"客户区"(真正画图的那块白色区域)的宽与高,500x500。
const int ClientWidth  = 500;
const int ClientHeight = 500;


// ---- 寻路网格划分 ----
// 把 500x500 的客户区切成一张格子网:横向 NumCellsX 格、纵向 NumCellsY 格,
// 都是 19。于是每个格子约 500/19 ≈ 26 像素见方;寻路算法就在这 19x19=361 个格子上跑。
const int NumCellsX    = 19;
const int NumCellsY    = 19;


//blank space required to print timing info
// InfoWindowHeight:客户区底部额外留的一条高度(20 像素),用来显示
// "本次寻路花了多少毫秒"这类文字信息,不占格子网的位置。
//(原文注释:打印计时信息所需的空白高度)
const int InfoWindowHeight = 20;


#endif