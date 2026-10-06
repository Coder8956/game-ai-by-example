//==============================================================================================
//【文件说明】Goal_HuntTarget.cpp —— "搜寻敌人"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:若还有活动目标,取"最后记录位置 LRP";若已到 LRP 或 LRP 为空,
//  就挂"探索"子目标随机找;否则挂"奔往 LRP"子目标。Process:敌人进视野就完成。
//==============================================================================================
#include "Goal_HuntTarget.h"
#include "Goal_Explore.h"
#include "Goal_MoveToPosition.h"
#include "..\Raven_Bot.h"
#include "..\Raven_SteeringBehaviors.h"



#include "debug/DebugConsole.h"
#include "misc/cgdi.h"

//---------------------------- Initialize -------------------------------------
//-----------------------------------------------------------------------------  
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_HuntTarget::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //if this goal is reactivated then there may be some existing subgoals that
  //must be removed
// 清掉旧子目标。
//(原文注释:重新激活时,旧子目标要先清掉)
  RemoveAllSubgoals();
  
  //it is possible for the target to die whilst this goal is active so we
  //must test to make sure the bot always has an active target
//(原文注释:目标可能在本目标激活期间死掉,所以要确认机器人仍有活动目标)
// 仍有活动目标才继续搜寻。
  if (m_pOwner->GetTargetSys()->isTargetPresent())
  {
    //grab a local copy of the last recorded position (LRP) of the target
// lrp=最后记录位置;const=只读。
//(原文注释:取出敌人"最后记录位置 LRP"的本地副本)
    const Vector2D lrp = m_pOwner->GetTargetSys()->GetLastRecordedPosition();

    //if the bot has reached the LRP and it still hasn't found the target
    //it starts to search by using the explore goal to move to random
    //map locations
// || :逻辑或——LRP 是空点(没记录过)或已站到 LRP 上。
//(原文注释:若已到 LRP 仍没找到敌人,就改用"探索"目标随机去地图各处搜)
    if (lrp.isZero() || m_pOwner->isAtPosition(lrp))
    {
// 挂"探索"子目标,随机点乱找。
      AddSubgoal(new Goal_Explore(m_pOwner));
    }

    //else move to the LRP
//(原文注释:否则就奔往 LRP)
    else
    {
// 挂"移动到 LRP"子目标。
      AddSubgoal(new Goal_MoveToPosition(m_pOwner, lrp));
    }
  }

  //if their is no active target then this goal can be removed from the queue
//(原文注释:若没有活动目标了,本目标即可从队列移除)
// 没有活动目标 → 本目标完成(结束搜寻)。
  else
  {
    m_iStatus = completed;
  }
    
}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_HuntTarget::Process()
{
  //if status is inactive, call Activate()
// 未激活就先激活。
  ActivateIfInactive();

// 跑子目标,取回状态。
  m_iStatus = ProcessSubgoals();

  //if target is in view this goal is satisfied
// 敌人进入视野锥 → 完成。
//(原文注释:若目标进入视野,本目标即满足)
  if (m_pOwner->GetTargetSys()->isTargetWithinFOV())
  {
     m_iStatus = completed;
  }

  return m_iStatus;
}


//------------------------------- Render --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:调试画 LRP(用 #ifdef 包着,默认不画)。
void Goal_HuntTarget::Render()
{  
//#define SHOW_LAST_RECORDED_POSITION
// #ifdef:条件编译——定义了该宏才画 LRP。
#ifdef SHOW_LAST_RECORDED_POSITION
  //render last recorded position as a green circle
  if (m_pOwner->GetTargetSys()->isTargetPresent())
  {
    gdi->GreenPen();
    gdi->RedBrush();
    gdi->Circle(m_pOwner->GetTargetSys()->GetLastRecordedPosition(), 3);
  }
#endif

 //forward the request to the subgoals
  Goal_Composite<Raven_Bot>::Render();
  
}