//==============================================================================================
//【文件说明】Goal_DodgeSideToSide.cpp —— "左右横移躲避"目标的实现
//
//【这个文件是干什么的?】
//  Activate:开 seek 转向,按当前方向试着往右/左迈一步;走不动就掉头换边;
//  Process:敌人出视野就完成,走到横移点就暂停等下一帧再换方向;
//  Terminate:关 seek;Render:调试画横移点连线。
//==============================================================================================
#include "Goal_DodgeSideToSide.h"
#include "Goal_SeekToPosition.h"
#include "../Raven_Bot.h"
#include "../Raven_SteeringBehaviors.h"
#include "../Raven_Game.h"

#include "Messaging/Telegram.h"
#include "../Raven_Messages.h"

#include "debug/DebugConsole.h"
#include "misc/cgdi.H"


//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_DodgeSideToSide::Activate()
{
// 状态=进行中。
  m_iStatus = active;

// 开 seek(朝目标点走)转向。
  m_pOwner->GetSteering()->SeekOn();

  
// 按当前方向分支:顺时针边(向右)或逆时针边(向左)。
    if (m_bClockwise)
    {
// canStepRight:判断往右迈一步会不会撞墙/出界;能走就把转向目标设为横移点。
      if (m_pOwner->canStepRight(m_vStrafeTarget))
      {
        m_pOwner->GetSteering()->SetTarget(m_vStrafeTarget);
      }
      else
      {
        //debug_con << "changing" << "";
// ! :逻辑取反——方向掉头(右↔左)。
// 置为未激活,下一帧 Process 会重新激活(从而重新算横移点)。
        m_bClockwise = !m_bClockwise;
        m_iStatus = inactive;
      }
    }

    else
    {
// 向左那一侧:同理,能走就设目标,不能走就掉头。
      if (m_pOwner->canStepLeft(m_vStrafeTarget))
      {
        m_pOwner->GetSteering()->SetTarget(m_vStrafeTarget);
      }
      else
      {
       // debug_con << "changing" << "";
        m_bClockwise = !m_bClockwise;
        m_iStatus = inactive;
      }
    }

   
}



//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_DodgeSideToSide::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive(); 

  //if target goes out of view terminate
// ! :取反——敌人不在视野锥内。FOV=视场角(Field Of View)。
//(原文注释:若目标离开视野,则结束本目标)
  if (!m_pOwner->GetTargetSys()->isTargetWithinFOV())
  {
    m_iStatus = completed;
  }

  //else if bot reaches the target position set status to inactive so the goal 
  //is reactivated on the next update-step
// 已走到横移点 → 暂停(inactive),等下一帧换方向。
//(原文注释:若机器人到达横移点,则置为未激活,下一帧再重新激活)
  else if (m_pOwner->isAtPosition(m_vStrafeTarget))
  {
    m_iStatus = inactive;
  }

  return m_iStatus;
}

//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Terminate:终止——关 seek 转向。
void Goal_DodgeSideToSide::Terminate()
{
  m_pOwner->GetSteering()->SeekOff();
}

//---------------------------- Render -----------------------------------------

//--------------------------------------------------------------------------------
// Render:调试画横移点(用 #ifdef SHOW_TARGET 包着,默认不画)。
void Goal_DodgeSideToSide::Render()
{
//#define SHOW_TARGET
// #ifdef:条件编译——只有定义了 SHOW_TARGET 宏,下面画线条/画圆的代码才参与编译。
#ifdef SHOW_TARGET
  gdi->OrangePen();
  gdi->HollowBrush();

  gdi->Line(m_pOwner->Pos(), m_vStrafeTarget);
  gdi->Circle(m_vStrafeTarget, 3);
#endif
  
}



