//==============================================================================================
//【文件说明】PrecisionTimer.h —— "高精度计时器":游戏的帧节奏总指挥
//
//【这个文件是干什么的?】
//  比 CrudeTimer 更精准的秒表,用 Windows 的"高性能计数器"(QueryPerformanceCounter)
//  计时。游戏主循环靠它回答两个问题:① 现在该渲染下一帧了吗?(ReadyForNextFrame);
//  ② 从上一帧到现在过了多少秒?(TimeElapsed,物体移动距离 = 速度 × 这段时间)。
//
//【谁在使用这个文件?】
//  main.cpp(第 3、4、5、7~10 章)—— 各游戏主程序在游戏循环里驱动它;
//  GameWorld.cpp、Raven_Game.cpp、Pathfinder.cpp —— 具体游戏世界类用它推进逻辑。
//
//【本文件包含了谁?】
//  <windows.h> —— Windows 系统头文件(QueryPerformanceCounter 等计时 API);
//  <cassert>   —— 断言宏 assert()(ReadyForNextFrame 里检查是否设过 FPS)。
//
//【C++ 小课堂:高性能计时器 vs 普通计时】
//  timeGetTime() 精度只有毫秒;高精度计数器精度可达微秒甚至更高,但它读出的不是"秒",
//  而是"滴答数(tick)"。要先 QueryPerformanceFrequency 问出"每秒滴答多少次",再用
//  滴答数 ×(1/每秒滴答数)换算回秒。LONGLONG 是 Windows 的 64 位整数(滴答数很大)。
//==============================================================================================
#ifndef PRECISION_TIMER_H
#define PRECISION_TIMER_H
//-----------------------------------------------------------------------
//
//  Name: PrecisionTimer.h
//
//  Author: Mat Buckland 2002
//
//  Desc: Windows timer class.
//
//        nb. this only uses the high performance timer. There is no
//        support for ancient computers. I know, I know, I should add
//        support, but hey, I have shares in AMD and Intel... Go upgrade ;o)
//
//-----------------------------------------------------------------------
// ↓↓↓ 原文翻译【PrecisionTimer —— 高精度计时器】:
//   Windows 计时器类。注意:它只用"高性能计时器",不支持古老的电脑。
//   作者调侃:我知道该加兼容,但我持有 AMD 和 Intel 的股票……快去升级电脑吧 ;o)
//------------------------------------------------------------------------
#include <windows.h>
#include <cassert>


//------------------------------------------------------------------------
// class PrecisionTimer —— 高精度计时器类(普通类,不是单例,需要用时自己定义一个对象)。
//------------------------------------------------------------------------
class PrecisionTimer
{

private:

  // 下面这一组都是 LONGLONG(64 位整数)的"滴答数"记录,分别记:
  //   m_CurrentTime          当前读到的计数器值;
  //   m_LastTime             上一帧的计数器值;
  //   m_LastTimeInTimeElapsed 上次调用 TimeElapsed() 时的值;
  //   m_NextTime             下一帧该到的计数器值(到点就渲染);
  //   m_StartTime            Start() 启动那一刻的值;
  //   m_FrameTime            一帧应占多少个滴答;
  //   m_PerfCountFreq        每秒滴答数(频率)。
  LONGLONG  m_CurrentTime,
            m_LastTime,
            m_LastTimeInTimeElapsed,
            m_NextTime,
            m_StartTime,
            m_FrameTime,
            m_PerfCountFreq;

  // 这一组是 double 秒数:
  //   m_TimeElapsed        本帧流逝的秒数;
  //   m_LastTimeElapsed    上一次 TimeElapsed 算出的秒数;
  //   m_TimeScale          "滴答→秒"的换算系数 = 1/每秒滴答数。
  double    m_TimeElapsed,
            m_LastTimeElapsed,
            m_TimeScale;

  // 目标帧率:m_NormalFPS = 正常每秒帧数;m_SlowFPS = 慢速每秒帧数(演示用)。
  double    m_NormalFPS;
  double    m_SlowFPS;

  bool      m_bStarted;

  //if true a call to TimeElapsed() will return 0 if the current
  //time elapsed is much smaller than the previous. Used to counter
  //the problems associated with the user using menus/resizing/moving 
  //a window etc
  // ↓↓↓ 上面这几行英文注释的翻译:若 m_bSmoothUpdates 为 true,当本次流逝时间
  //   比上次小很多时,TimeElapsed() 返回 0。这是为了对付用户拉开菜单、拖拽/缩放
  //   窗口等造成的"时间突变"问题。m_b = 成员变量中的 bool 布尔类型。
  bool      m_bSmoothUpdates;


public:

  //(原文注释:ctors = constructors,构造函数们)
  // 两个构造函数:一个不带参数(默认),一个可传入目标帧率 fps。
  //ctors
  PrecisionTimer();
  PrecisionTimer(double fps);


  //whatdayaknow, this starts the timer
  //(原文注释:如名字所示,这个函数启动计时器)
  // 在进入游戏循环之前立刻调用 Start()。
  void    Start();

  //determines if enough time has passed to move onto next frame
  //(原文注释:判断是否已经过了足够时间、该进入下一帧了)
  // 只在设置了 FPS 时使用。inline = 内联函数(函数体写在类外但声明带 inline,
  // 详见本文件下方 ReadyForNextFrame 的实现)。
  inline bool    ReadyForNextFrame();

  //only use this after a call to the above.
  //double  GetTimeElapsed(){return m_TimeElapsed;}

  // 流逝时间:返回"距离上次调用它过了多少秒"。游戏里物体就按这个值移动。
  inline double  TimeElapsed();

  // CurrentTime:返回"自 Start() 以来过了多少秒"(当前时刻)。
  // 下面 QueryPerformanceCounter 是 Windows API,读出当前高性能计数器的滴答值;
  // 它要求传入 LARGE_INTEGER* 指针,这里把 m_CurrentTime 的地址 &m_CurrentTime
  // 强制转成 (LARGE_INTEGER*) 塞进去(二者都是 64 位整数,内存布局兼容);& 是取地址符。
  // (本文件后面 ReadyForNextFrame、TimeElapsed 里还各有一行同样的 QPC 调用,原理相同,
  //  不再重复解释。)
  double  CurrentTime()
  { 
    QueryPerformanceCounter( (LARGE_INTEGER*) &m_CurrentTime);

    return (m_CurrentTime - m_StartTime) * m_TimeScale;
  }

  // Started:只读查询计时器是否已启动;末尾 const 表示本函数不修改任何成员。
  // SmoothUpdatesOn/Off:打开/关闭上面说的"时间突变平滑"开关。
  bool    Started()const{return m_bStarted;}

  void    SmoothUpdatesOn(){m_bSmoothUpdates = true;}
  void    SmoothUpdatesOff(){m_bSmoothUpdates = false;}

};


//========================================================================
// 下面两个 inline 成员函数的实现写在类外(因为稍长,放类里太挤):
// 用 PrecisionTimer:: 前缀指明它们属于这个类。inline 允许跨文件重复定义而不报错。
//========================================================================
//-------------------------ReadyForNextFrame()-------------------------------
//
//  returns true if it is time to move on to the next frame step. To be used if
//  FPS is set.
//(原文注释的翻译:如果设置了 FPS,就用它来判断是否该进下一帧)
//
//----------------------------------------------------------------------------
inline bool PrecisionTimer::ReadyForNextFrame()
{
  // assert:断言——条件为假时程序立刻报错停住。这里要求必须设置过正常帧率,
  // 否则下面按帧计时毫无意义。引号里的字符串会在报错时显示出来提示原因。
  assert(m_NormalFPS && "PrecisionTimer::ReadyForNextFrame<No FPS set in timer>");
  
  QueryPerformanceCounter( (LARGE_INTEGER*) &m_CurrentTime);

  // QPC 读数后,判断:当前滴答是否已超过"下一帧该到的时刻"。超过 = 该渲染下一帧了。
  if (m_CurrentTime > m_NextTime)
  {

    m_TimeElapsed = (m_CurrentTime - m_LastTime) * m_TimeScale;
    m_LastTime    = m_CurrentTime;
    // 算出本帧流逝秒数 =(本次滴答 - 上次滴答)× 换算系数 m_TimeScale;
    // 然后把"上次滴答"更新为本次,供下一帧再算。

    //update time to render next frame
    m_NextTime = m_CurrentTime + m_FrameTime;
    // 推算"下一帧该到的滴答时刻" = 本次 + 每帧应占的滴答数。

    return true;
  }

  return false;
}

//--------------------------- TimeElapsed --------------------------------
//
//  returns time elapsed since last call to this function.
//(原文注释:返回自上次调用本函数以来流逝的时间)
//-------------------------------------------------------------------------
inline double PrecisionTimer::TimeElapsed()
{
  // 先把上次算出的流逝秒数存起来(供后面平滑判断用)。
  m_LastTimeElapsed = m_TimeElapsed;

  QueryPerformanceCounter( (LARGE_INTEGER*) &m_CurrentTime);
  
  m_TimeElapsed = (m_CurrentTime - m_LastTimeInTimeElapsed) * m_TimeScale;
  // 再算新的流逝秒数 =(本次滴答 - 上次调本函数时的滴答)× 换算系数。
  
  m_LastTimeInTimeElapsed    = m_CurrentTime;
  // 更新"上次调用时刻",为下一次做准备。

  // 平滑阈值:本次流逝若不到上次的 5 倍,就认为是正常的小波动,照实返回;
  // 若本次比上次大太多(比如刚拖完窗口),说明是系统卡顿造成的假时间,返回 0,
  // 避免游戏物体那一瞬间"瞬移"。
  const double Smoothness = 5.0;

  // 若开启了平滑:本次时间 < 上次的 5 倍才照实返回;否则返回 0(见上面说明)。
  if (m_bSmoothUpdates)
  {
    if (m_TimeElapsed < (m_LastTimeElapsed * Smoothness))
    {
      return m_TimeElapsed;
    }

    else
    {
      return 0.0;
    }
  }
  
  else
  {
    return m_TimeElapsed;
  }
    
}



#endif

  
