//==============================================================================================
//【文件说明】CrudeTimer.h —— "粗略计时器":程序从启动到现在过了多少秒
//
//【这个文件是干什么的?】
//  提供一个全局可用的"秒表":程序启动那一刻记为 0,之后任何地方调 Clock.GetCurrentTime()
//  就能拿到"到现在为止经过了多少秒"。精度约毫秒级(粗略),足够游戏里做"过了 3 秒
//  就爆炸"这类倒计时/超时逻辑。它用"单例模式"实现:全程序只有一个计时器对象,
//  通过宏 Clock 在任何文件里直接使用。
//
//【谁在使用这个文件?】
//  goals/Goal_TraverseEdge.cpp、goals/Goal_SeekToPosition.cpp —— 第 7~10 章 Raven 的
//      "走导航路 / 追目标"复合目标,用它判断某步行动花了多久、是否超时。
//
//【本文件包含了谁?】
//  <windows.h> —— Windows 系统头文件(timeGetTime() 毫秒计时函数来自这里)。
//
//【C++ 小课堂:单例模式(Singleton)】
//  一个类在全程序中只允许存在一个对象(本例:计时器一个就够,多了会对不上时间)。
//  三招实现:① 构造函数放进 private —— 外面不能 new;② 提供 static 函数 Instance()
//    帮你取出那唯一的对象;③ 把"拷贝构造函数"和"赋值运算符"也声明成 private 但不
//    实现,防止有人偷偷复制出第二个。
//==============================================================================================
#ifndef CRUDETIMER_H
#define CRUDETIMER_H
//------------------------------------------------------------------------
//
//  Name:   CrudeTimer.h
//
//  Desc:   timer to measure time in seconds
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------

// ↓↓↓ 原文翻译【CrudeTimer —— 粗略计时器】:
//   用途:以秒为单位计时的计时器。作者:Mat Buckland,2002 年。
//------------------------------------------------------------------------
//this library must be included
//(原文注释:下面这个库必须被包含进来)
//  指 winmm.lib:#pragma comment(lib,"winmm.lib") 让 MSVC 自动链接多媒体库
// (本文件用的 timeGetTime() 就在这个库里)。
#pragma comment(lib, "winmm.lib")

#include <windows.h>



//========================================================================
// #define Clock CrudeTimer::Instance() —— 定义一个"宏":预处理阶段把代码里出现的
//   Clock 这个词,原样替换成 CrudeTimer::Instance()(取得那个唯一的计时器对象)。
//   于是全程序任何地方只要写 Clock.GetCurrentTime(),就等于调了单例的成员函数,
//   用起来像一个全局变量。(宏是预处理指令,不进编译,纯文本替换。)
//========================================================================
#define Clock CrudeTimer::Instance()

//------------------------------------------------------------------------
// class CrudeTimer —— 粗略计时器类(单例)。
//------------------------------------------------------------------------
class CrudeTimer
{
private:
  

  //set to the time (in seconds) when class is instantiated
//(原文注释:类被创建那一刻的时间,以秒为单位)
//  记下"程序启动时刻"。m_d = 成员变量中的 double 浮点类型。
  double m_dStartTime;

  //set the start time
  //(原文注释:设置起始时间)
  // 私有构造函数:外面不能 new,只有类自己能创建。timeGetTime() 返回开机至今的
  // 毫秒数;×0.001 把毫秒换成秒。对象诞生那一刻记下起始秒数。
  CrudeTimer(){m_dStartTime = timeGetTime() * 0.001;}

  //copy ctor and assignment should be private
  //(原文注释:拷贝构造函数和赋值运算符应当设为私有)
  // 下面两行是单例模式第③招:声明(不实现)私有的拷贝构造函数 CrudeTimer(const
  // CrudeTimer&) 和赋值运算符 operator=。一旦有人想"复制一个计时器",编译器就报错
  // 或链接失败 —— 从根上杜绝第二个实例出现。
  CrudeTimer(const CrudeTimer&);
  CrudeTimer& operator=(const CrudeTimer&);
  
public:

  // Instance():单例的"取对象入口"。static = 静态函数,不靠对象就能调用:
  // 写法 CrudeTimer::Instance()。真正的实现写在 CrudeTimer.cpp 里(返回那唯一对象)。
  static CrudeTimer* Instance();

  //returns how much time has elapsed since the timer was started
  //(原文注释:返回自计时器启动以来经过了多少时间)
  //  当前秒数(timeGetTime()*0.001) - 启动秒数 = 已经过了多少秒。函数体直接写在声明里,
  // 这是 C++ 的"内联"写法(简短函数直接在类定义内给函数体)。
  double GetCurrentTime(){return timeGetTime() * 0.001 - m_dStartTime;}

};







#endif