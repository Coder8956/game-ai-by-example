//==============================================================================================
//【文件说明】Regulator.h —— "节流器":按指定频率放行代码的小开关
//
//【这个文件是干什么的?】
//  游戏里有些事不需要每帧都做:例如机器人"喊话"或"评估战术",每帧算一遍太浪费。
//  Regulator(节流器/调节器)的用法是:创建它时告诉它"我想每秒做 N 次",之后每帧
//  问一句 isReady();只有真正到点了它才回答 true,其余时间一律 false —— 于是那段
//  代码就被"限速"成每秒 N 次。
//
//【谁在使用这个文件?】(= 哪些文件用 #include 包含了它)
//  FieldPlayer.h / FieldPlayer.cpp / FieldPlayerStates.cpp —— 第 4 章足球球员,
//      用它限制"传球决策、支援点评估"等逻辑的刷新频率(不是每帧都算);
//  Raven_Bot.cpp —— 第 7~10 章 Raven 机器人,用它给各类 AI 动作"限速"。
//
//【本文件包含了谁?】
//  mmsystem.h    —— Windows 多媒体库头文件(提供 timeGetTime() 毫秒计时);
//  misc/utils.h  —— 通用工具库(RandFloat / RandInRange / isEqual,均在 Common\misc\)。
//
//【C++ 小课堂:包含保护(include guard)】
//  #ifndef REGULATOR / #define REGULATOR / #endif 是头文件标准三件套:
//  "#include"的本质是把文件内容原样复制进来;若本文件沿多条包含链被复制两遍,
//  同样的定义出现两次,编译器会报"重复定义"。这三行保证第二次复制被自动跳过。
//  (同样的三件套也见 Goal.h、Locations.h;之后各头文件不再重复详解。)
//==============================================================================================
#ifndef REGULATOR
#define REGULATOR
//------------------------------------------------------------------------
//
//  Name:   Regulator.h
//
//  Desc:   Use this class to regulate code flow (for an update function say)
//          Instantiate the class with the frequency you would like your code
//          section to flow (like 10 times per second) and then only allow 
//          the program flow to continue if Ready() returns true
//
//  Author: Mat Buckland 2003 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
// ↓↓↓ 原文翻译【Regulator —— 节流器】:
//   用这个类来调节代码流(比如某个 Update 函数)。创建它时告诉它你希望这段代码
//   每秒执行多少次(比如每秒 10 次),之后只有当 isReady() 返回 true 时才放行。
//------------------------------------------------------------------------
// 本文件开头两条系统相关指令(都为了能用 timeGetTime 这个 Windows 计时函数):
//   第 1 条 #pragma comment(lib,"winmm.lib") —— #pragma 是给编译器下命令的专用指令;
//     comment(lib,...) 的意思是"编译时自动把 winmm.lib 库链接进程序"(timeGetTime
//     的实现就在这个库里),省得手动去工程设置里加。行尾英文注释是原作者提醒:
//     不用 MSVC 的话,记得自己在工程里把 winmm.lib 加进去。
//   第 2 条 #include "mmsystem.h" —— 把多媒体库的头文件拿进来,这样编译器才认得
//     timeGetTime()、DWORD 这些名字。
#pragma comment(lib,"winmm.lib") //if you don't use MSVC make sure this library is included in your project
#include "mmsystem.h" 

// 通用工具库头文件:提供本文件用到的 RandFloat()(0~1 随机)、
// RandInRange(a,b)(区间随机)、isEqual(x,y)(浮点数近似相等)等小函数。
#include "misc/utils.h"




//--------------------------------------------------------------------------------
// class Regulator —— "节流器"类。
//   使用三步:① 创建时传入"期望每秒次数" → ② 每帧调一次 isReady() →
//   ③ 它返回 true 才执行那段被限速的代码。
//--------------------------------------------------------------------------------
class Regulator
{
private:

  //the time period between updates 
//(原文注释:两次更新之间的时间间隔)
//  即"每隔多少毫秒放行一次",单位毫秒。m_d = 成员变量中的 double 浮点类型。
  double m_dUpdatePeriod;

  //the next time the regulator allows code flow
//(原文注释:节流器下一次允许代码放行的时刻)
//  记录"下一次放行时刻"的 Windows 毫秒时间戳。m_dw = 成员变量中的 DWORD 类型;
//  DWORD 是 Windows 自己定义的"32 位无符号整数"。
  DWORD m_dwNextUpdateTime;


public:

  
  //--------------------------------------------------------------------------------
  // 构造函数:创建节流器时自动调用。
  //   NumUpdatesPerSecondRqd = 你期望这段代码每秒执行多少次(Required 的缩写)。
  //--------------------------------------------------------------------------------
  Regulator(double NumUpdatesPerSecondRqd)
  {
    m_dwNextUpdateTime = (DWORD)(timeGetTime()+RandFloat()*1000);

    // 先随机定一个"下一次放行时刻":timeGetTime() 返回开机至今的毫秒数;
    // RandFloat()*1000 = 0~1000 毫秒的随机偏移,给个"错峰"初值,免得所有节流器
    // 都在同一毫秒一起触发。前面的 (DWORD) 是强制类型转换:把浮点结果截成 DWORD。
    if (NumUpdatesPerSecondRqd > 0)
    {
      m_dUpdatePeriod = 1000.0 / NumUpdatesPerSecondRqd; 
      // 正数频率:每秒 N 次 → 两次放行间隔 = 1000 毫秒 ÷ N。
    }

    else if (isEqual(0.0, NumUpdatesPerSecondRqd))
    {
      m_dUpdatePeriod = 0.0;
      // 频率正好等于 0:间隔设成 0 → 后面 isReady 永远放行(见下面"隐身模式")。
    }

    else if (NumUpdatesPerSecondRqd < 0)
    {
      m_dUpdatePeriod = -1;
      // 频率为负数:间隔设成 -1 → 后面 isReady 永远不放行(见下面"永不放行模式")。
    }
  }


  //returns true if the current time exceeds m_dwNextUpdateTime
  //--------------------------------------------------------------------------------
  // isReady:每帧调用一次。返回 true = 现在到点了,可以执行被限速的代码;
  // 返回 false = 还没到点,这一帧跳过。
  //--------------------------------------------------------------------------------
  bool isReady()
  {
    //if a regulator is instantiated with a zero freq then it goes into
    // ↓↓↓ 这两行英文注释的翻译:如果节流器创建时频率为 0,它就进入"隐身模式"
    //   (stealth mode)——不做任何限速,永远放行。
    //stealth mode (doesn't regulate)
    if (isEqual(0.0, m_dUpdatePeriod)) return true;
    // 隐身模式:间隔≈0,直接放行。isEqual 是浮点数近似相等判断(避免 == 的误差)。

    //if the regulator is instantiated with a negative freq then it will
    // ↓↓↓ 这两行英文注释的翻译:如果节流器创建时频率为负数,它将永远不放行。
    //never allow the code to flow
    if (m_dUpdatePeriod < 0) return false;
    // 永不放行模式:间隔为负,直接拒绝。

    DWORD CurrentTime = timeGetTime();

    //the number of milliseconds the update period can vary per required
    //update-step. This is here to make sure any multiple clients of this class
    //have their updates spread evenly
    // ↓↓↓ 上面这几行英文注释的翻译:每次放行,间隔可以有最多 10 毫秒的随机浮动;
    //   这样做是为了让同时使用本类的多个"客户"(比如场上 11 个球员)的更新时刻
    //   被均匀摊开,不会挤在同一毫秒一起触发。
    // static const:静态常量 —— 全类所有对象共用这一份,程序运行期间值不变。
    static const double UpdatePeriodVariator = 10.0;

    if (CurrentTime >= m_dwNextUpdateTime)
    {
      m_dwNextUpdateTime = (DWORD)(CurrentTime + m_dUpdatePeriod + RandInRange(-UpdatePeriodVariator, UpdatePeriodVariator));
      // 到点了:先把"下一次放行时刻"往后推一个间隔,再加一点随机浮动
      // (RandInRange(-10,10) = -10~10 毫秒之间的随机数,实现上面说的"错峰")。
      // 更新完时刻表后,下面那行 return true 通知调用者"现在可以执行了"。

      return true;
    }

    return false;
    // 还没到点:走到这里说明 CurrentTime 仍小于下次放行时刻,返回 false,这一帧跳过。
  }
};



#endif