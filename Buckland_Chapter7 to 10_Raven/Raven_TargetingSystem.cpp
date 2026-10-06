//==============================================================================================
//【文件说明】Raven_TargetingSystem.cpp —— 选目标子系统的实现
//==============================================================================================
#include "Raven_TargetingSystem.h"
#include "Raven_Bot.h"
#include "Raven_SensoryMemory.h"



//-------------------------------- ctor ---------------------------------------
//-----------------------------------------------------------------------------
// 构造函数:记下 owner,当前目标先置空。
Raven_TargetingSystem::Raven_TargetingSystem(Raven_Bot* owner):m_pOwner(owner),
                                                               m_pCurrentTarget(0)
{}



//----------------------------- Update ----------------------------------------

//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Update:遍历感官记忆里「最近见过的敌人」,挑最近的活的那个当当前目标。
void Raven_TargetingSystem::Update()
{
  double ClosestDistSoFar = MaxDouble;
  m_pCurrentTarget       = 0;

  //grab a list of all the opponents the owner can sense
// SensedBots:最近感知到的敌人列表。
//(原文注释:取出拥有者能感知到的所有对手列表)
  std::list<Raven_Bot*> SensedBots;
  SensedBots = m_pOwner->GetSensoryMem()->GetListOfRecentlySensedOpponents();
  
  std::list<Raven_Bot*>::const_iterator curBot = SensedBots.begin();
  for (curBot; curBot != SensedBots.end(); ++curBot)
  {
    //make sure the bot is alive and that it is not the owner
//(原文注释:确保这个 bot 活着,而且不是拥有者自己)
    if ((*curBot)->isAlive() && (*curBot != m_pOwner) )
    {
// 距离平方比较(省开方);更近就更新当前目标。
      double dist = Vec2DDistanceSq((*curBot)->Pos(), m_pOwner->Pos());

      if (dist < ClosestDistSoFar)
      {
        ClosestDistSoFar = dist;
        m_pCurrentTarget = *curBot;
      }
    }
  }
}




// 下面 5 个查询全部转发给感官记忆 SensoryMemory 去问。
bool Raven_TargetingSystem::isTargetWithinFOV()const
{
  return m_pOwner->GetSensoryMem()->isOpponentWithinFOV(m_pCurrentTarget);
}

bool Raven_TargetingSystem::isTargetShootable()const
{
  return m_pOwner->GetSensoryMem()->isOpponentShootable(m_pCurrentTarget);
}

Vector2D Raven_TargetingSystem::GetLastRecordedPosition()const
{
  return m_pOwner->GetSensoryMem()->GetLastRecordedPositionOfOpponent(m_pCurrentTarget);
}

double Raven_TargetingSystem::GetTimeTargetHasBeenVisible()const
{
  return m_pOwner->GetSensoryMem()->GetTimeOpponentHasBeenVisible(m_pCurrentTarget);
}

double Raven_TargetingSystem::GetTimeTargetHasBeenOutOfView()const
{
  return m_pOwner->GetSensoryMem()->GetTimeOpponentHasBeenOutOfView(m_pCurrentTarget);
}
