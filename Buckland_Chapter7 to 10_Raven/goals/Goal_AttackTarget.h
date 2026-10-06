//==============================================================================================
//【文件说明】Goal_AttackTarget.h —— "攻击当前目标"组合目标(战斗大脑)
//
//【这个文件是干什么的?】
//  这是战斗时的总调度:机器人锁定敌人后,按情况拆出战斗子目标——
//  能看见且能打到 → 左右横移躲子弹(DodgeSideToSide)或直冲敌人位置;
//  看不见敌人 → 先去搜寻(HuntTarget)。继承自 Goal_Composite<Raven_Bot>。
//
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h、../Raven_Bot.h。
//==============================================================================================
#ifndef GOAL_ATTACKTARGET_H
#define GOAL_ATTACKTARGET_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_AttackTarget.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   
//
//-----------------------------------------------------------------------------
#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"





//--------------------------------------------------------------------------------
// class Goal_AttackTarget : public Goal_Composite<Raven_Bot>:攻击是一种组合目标。
class Goal_AttackTarget : public Goal_Composite<Raven_Bot>
{
// public:对外接口。
public:

// 构造:pOwner=所属机器人。
  Goal_AttackTarget(Raven_Bot* pOwner):Goal_Composite<Raven_Bot>(pOwner, goal_attack_target)
  {}

  void Activate();

  int  Process();

// Activate/Process 标准动作;Terminate 内联在类里:状态=完成。
  void Terminate(){m_iStatus = completed;}

};






#endif
