//==============================================================================================
//【文件说明】Goal_SeekToPosition.cpp —— "直奔某坐标点"目标的实现
//
//【这个文件是干什么的?】
//  构造→记录目标点;Activate→记下开始时间、估算到达时间、打开 seek 转向;
//  Process→每帧检查是否卡住/是否到达;Terminate→关掉转向;Render→画目标点。
//==============================================================================================
#include "Goal_SeekToPosition.h"
#include "..\Raven_Bot.h"
#include "..\Raven_SteeringBehaviors.h"
#include "time/CrudeTimer.h"
#include "../navigation/Raven_PathPlanner.h"
#include "misc/cgdi.h"



#include "debug/DebugConsole.h"



//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// 构造函数:初始化列表把 pBot、编号 goal_seek_to_position 交给父类,
// 并记下目标点 target;m_dTimeToReachPos 先置 0(Activate 时再算)。
Goal_SeekToPosition::Goal_SeekToPosition(Raven_Bot* pBot,
                                         Vector2D   target):

                                Goal<Raven_Bot>(pBot,
                                                goal_seek_to_position),
                                 m_vPosition(target),
                                 m_dTimeToReachPos(0.0)
{}

                                             
//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_SeekToPosition::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //record the time the bot starts this goal
// Clock 是全局计时器单例;GetCurrentTime()=当前游戏时间,存为开始时刻。
//(原文注释:记录机器人开始本目标的时刻)
  m_dStartTime = Clock->GetCurrentTime();    
  
  //This value is used to determine if the bot becomes stuck 
// 问机器人:按它的速度跑到目标点该花多少秒。
//(原文注释:这个值用来判断机器人是否卡住)
  m_dTimeToReachPos = m_pOwner->CalculateTimeToReachPosition(m_vPosition);
  
  //factor in a margin of error for any reactive behavior
// 误差余量 1 秒,加在预计时间上,免得机器人被误判卡住。
//(原文注释:为"反应性行为"预留一点误差余量)
  const double MarginOfError = 1.0;

// += :加等——把余量叠到预计时间上。
  m_dTimeToReachPos += MarginOfError;

  
// 告诉转向行为器:目标点是 m_vPosition。
  m_pOwner->GetSteering()->SetTarget(m_vPosition);

// 打开"追逐"转向(直线朝目标跑)。
  m_pOwner->GetSteering()->SeekOn();
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_SeekToPosition::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();
    
  //test to see if the bot has become stuck
// 卡住了 → 状态=失败。
//(原文注释:检查机器人是否卡住)
  if (isStuck())
  {
    m_iStatus = failed;
  }
  
  //test to see if the bot has reached the waypoint. If so terminate the goal
//(原文注释:检查是否已到达目标点,到了就结束本目标)
  else
  { 
// 已经站在目标点附近 → 状态=完成。
    if (m_pOwner->isAtPosition(m_vPosition))
    {
      m_iStatus = completed;
    }
  }

  return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//(原文注释:若机器人到达当前路径点花的时间超过预期,则返回 true)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// isStuck:判断是否卡住。const 表示本函数不改任何成员。
bool Goal_SeekToPosition::isStuck()const
{  
// 已用时间 = 当前时间 - 开始时间(减法)。
  double TimeTaken = Clock->GetCurrentTime() - m_dStartTime;

// 已用时间 > 预计时间 = 超时 = 卡住。
  if (TimeTaken > m_dTimeToReachPos)
  {
// debug_con:调试控制台对象;<< 把字符串流式输出(打印"某机器人卡住了")。
    debug_con << "BOT " << m_pOwner->ID() << " IS STUCK!!" << "";

    return true;
  }

  return false;
}


//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Terminate:终止——关掉追逐和到达(arrive)转向。
void Goal_SeekToPosition::Terminate()
{
  m_pOwner->GetSteering()->SeekOff();
  m_pOwner->GetSteering()->ArriveOff();
// 状态=完成。

  m_iStatus = completed;
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画目标点。进行中画绿圈,未激活画红圈。
void Goal_SeekToPosition::Render()
{
  if (m_iStatus == active)
  {
// gdi 绘图单例:绿画刷/黑笔/画半径3的圆。
    gdi->GreenBrush();
    gdi->BlackPen();
    gdi->Circle(m_vPosition, 3);
  }

  else if (m_iStatus == inactive)
  {

   gdi->RedBrush();
   gdi->BlackPen();
   gdi->Circle(m_vPosition, 3);
  }
}

