//==============================================================================================
//【文件说明】Goal_AttackTarget.cpp —— "攻击当前目标"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:清旧子目标;没目标就完成退出;能射击(LOS)时——
//  能左右挪就挂横移躲避子目标,不能挪就直冲敌人位置;看不见敌人就挂搜寻子目标。
//==============================================================================================
#include "Goal_AttackTarget.h"
#include "Goal_SeekToPosition.h"
#include "Goal_HuntTarget.h"
#include "Goal_DodgeSideToSide.h"
#include "../Raven_Bot.h"






//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_AttackTarget::Activate()
{
// 状态=进行中。
  m_iStatus = active;

  //if this goal is reactivated then there may be some existing subgoals that
  //must be removed
// 清掉旧子目标。
//(原文注释:重新激活时,旧子目标要先清掉)
  RemoveAllSubgoals();

  //it is possible for a bot's target to die whilst this goal is active so we
  //must test to make sure the bot always has an active target
// ! :取反——没有活动目标了。
//(原文注释:目标可能在本目标激活期间死掉,要确认仍有活动目标)
  if (!m_pOwner->GetTargetSys()->isTargetPresent())
  {
// 状态=完成。
     m_iStatus = completed;

// 直接返回,不再安排战斗子目标。
     return;
  }

  //if the bot is able to shoot the target (there is LOS between bot and
  //target), then select a tactic to follow while shooting
// isTargetShootable():敌人可射击(有视线)。
//(原文注释:若机器人能射到目标(双方之间有视线 LOS),则选一个边打边用的战术)
  if (m_pOwner->GetTargetSys()->isTargetShootable())
  {
    //if the bot has space to strafe then do so
// dummy:占位向量(只用来调 canStepLeft/Right,不关心结果坐标)。
//(原文注释:若机器人有空间左右横移,就横移)
    Vector2D dummy;
// || :逻辑或——左边或右边能挪一步,就可以横移躲子弹。
    if (m_pOwner->canStepLeft(dummy) || m_pOwner->canStepRight(dummy))
    {
// 挂"左右横移躲避"子目标。
      AddSubgoal(new Goal_DodgeSideToSide(m_pOwner));
    }

    //if not able to strafe, head directly at the target's position 
//(原文注释:不能横移就直奔敌人位置)
    else
    {
// 挂"直奔敌人位置"子目标。
      AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_pOwner->GetTargetBot()->Pos()));
    }
  }

  //if the target is not visible, go hunt it.
//(原文注释:若目标看不见了,就去搜寻它)
  else
  {
// 挂"搜寻敌人"子目标。
    AddSubgoal(new Goal_HuntTarget(m_pOwner));
  }
}

//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_AttackTarget::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();
    
  //process the subgoals
// 跑子目标,取回状态。
//(原文注释:处理子目标)
  m_iStatus = ProcessSubgoals();

// 子目标失败就重新激活(重选战术)。
  ReactivateIfFailed();

  return m_iStatus;
}




