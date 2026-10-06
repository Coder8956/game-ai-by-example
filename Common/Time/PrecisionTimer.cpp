//==============================================================================================
//【文件说明】PrecisionTimer.cpp —— PrecisionTimer 高精度计时器的实现
//
//【这个文件是干什么的?】
//  实现三个函数:两个构造函数(初始化各成员、问出每秒滴答数)和 Start()
// (进入游戏循环前调用,记下起始时刻)。头文件里 ReadyForNextFrame / TimeElapsed
// 已写成 inline,不在本文件。
//
//【谁在使用本文件?】谁包含 PrecisionTimer.h,谁就用到这里的构造/Start(详见头文件)。
//
//【本文件包含了谁?】
//  Time/PrecisionTimer.h —— 自己的类声明(注意路径带 Time/ 前缀)。
//==============================================================================================
#include "Time/PrecisionTimer.h"


//---------------------- default constructor ------------------------------
//
//-------------------------------------------------------------------------
// ↓↓↓ 原文翻译【默认构造函数】:创建一个"不锁帧率"的计时器。
//------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:冒号后是"初始化列表",逐项给成员设初值(比进函数体再赋值高效)。
//   PrecisionTimer:: = 作用域解析符,说明这是 PrecisionTimer 类的构造函数。
//--------------------------------------------------------------------------------
PrecisionTimer::PrecisionTimer(): m_NormalFPS(0.0),
                  m_SlowFPS(1.0),
                  m_TimeElapsed(0.0),
                  m_FrameTime(0),
                  m_LastTime(0),
                  m_LastTimeInTimeElapsed(0),
                  m_PerfCountFreq(0),
                  m_bStarted(false),
                  m_StartTime(0),
                  m_LastTimeElapsed(0.0),
                  m_bSmoothUpdates(false)
{
  //how many ticks per sec do we get
  QueryPerformanceFrequency( (LARGE_INTEGER*) &m_PerfCountFreq);
  
  m_TimeScale = 1.0/m_PerfCountFreq;
}

//---------------------- constructor -------------------------------------
//
//  use to specify FPS
//
//-------------------------------------------------------------------------
// ↓↓↓ 原文翻译【带帧率的构造函数】:用它来指定目标 FPS(每秒帧数)。
//------------------------------------------------------------------------
// 第二个构造函数:多做一步——按帧率算出"每帧占多少个滴答"。
PrecisionTimer::PrecisionTimer(double fps): m_NormalFPS(fps),
                  m_SlowFPS(1.0),
                  m_TimeElapsed(0.0),
                  m_FrameTime(0),
                  m_LastTime(0),
                  m_LastTimeInTimeElapsed(0),
                  m_PerfCountFreq(0),
                  m_bStarted(false),
                  m_StartTime(0),
                  m_LastTimeElapsed(0.0),
                  m_bSmoothUpdates(false)
{

  //how many ticks per sec do we get
  QueryPerformanceFrequency( (LARGE_INTEGER*) &m_PerfCountFreq);

  m_TimeScale = 1.0/m_PerfCountFreq;

  //calculate ticks per frame
  // 每帧滴答数 = 每秒滴答数 ÷ 每秒帧数 = 一帧应占多长时间(以滴答计)。
  // (LONGLONG) 是强制类型转换,把除法结果截成 64 位整数。
  m_FrameTime = (LONGLONG)(m_PerfCountFreq / m_NormalFPS);
}




//------------------------Start()-----------------------------------------
//
//  call this immediately prior to game loop. Starts the timer (obviously!)
//
//--------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Start()】:在游戏循环开始前立刻调用它,启动计时器。
//------------------------------------------------------------------------
// Start():进入游戏循环前调用。作用:记下"现在"作为零点,并排好下一帧的时刻表。
void PrecisionTimer::Start()
{
  m_bStarted = true;
  
  m_TimeElapsed = 0.0;

  //get the time
  QueryPerformanceCounter( (LARGE_INTEGER*) &m_LastTime);
  // 记下此刻的滴答值(作为本帧/起始时刻)。

  //keep a record of when the timer was started
  m_StartTime = m_LastTimeInTimeElapsed = m_LastTime;
  // 一次连等:把三个记录都设成同一时刻。= 从右往左赋值,m_LastTime 先传入,
  // 再赋给 m_LastTimeInTimeElapsed,再赋给 m_StartTime。

  //update time to render next frame
  m_NextTime = m_LastTime + m_FrameTime;
  // 排好"下一帧该到的时刻" = 此刻 + 一帧的滴答数。

  return;
}

