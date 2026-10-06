//==============================================================================================
//【文件说明】AttackTargetGoal_Evaluator.cpp —— "攻击当前目标"评估器的实现
//
//【这个文件是干什么的?】
//  只有当机器人当前锁定了敌人(isTargetPresent)时才打分:渴望分 = 血量 × 总火力。
//  没锁定目标时渴望分为 0(先去找敌人,见 Goal_HuntTarget)。
//==============================================================================================
#include "AttackTargetGoal_Evaluator.h"
#include "Goal_Think.h"
#include "Raven_Goal_Types.h"
#include "../Raven_WeaponSystem.h"
#include "../Raven_ObjectEnumerations.h"
#include "misc/cgdi.h"
#include "misc/Stream_Utility_Functions.h"
#include "Raven_Feature.h"


#include "debug/DebugConsole.h"

//------------------ CalculateDesirability ------------------------------------
//
//  returns a value between 0 and 1 that indicates the Rating of a bot (the
//  higher the score, the stronger the bot).
//(原文注释:返回 0~1,表示机器人的"实力评分";分数越高机器人越强)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// CalculateDesirability:算"攻击"的渴望分。
double AttackTargetGoal_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
// 先把渴望分初始化为 0(没目标时就保持 0)。
  double Desirability = 0.0;

  //only do the calculation if there is a target present
// GetTargetSys()=目标系统;isTargetPresent()=是否已锁定敌人。&&/|| 之外的 && 在此不用;
//(原文注释:只有当前存在目标时才做计算)
  if (pBot->GetTargetSys()->isTargetPresent()) 
  {
// Tweaker=1.0:此处缩放系数为 1(不缩放)。
     const double Tweaker = 1.0;

// 渴望分 = 缩放 × 血量占比 × 总武器火力。血多、武器强 → 越敢打。
     Desirability = Tweaker *
                    Raven_Feature::Health(pBot) * 
                    Raven_Feature::TotalWeaponStrength(pBot);

     //bias the value according to the personality of the bot
// 乘性格偏置。
//(原文注释:按性格偏置调整)
     Desirability *= m_dCharacterBias;
  }
    
  return Desirability;
}

//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// SetGoal:选中后给大脑加"攻击目标"目标。
void AttackTargetGoal_Evaluator::SetGoal(Raven_Bot* pBot)
{
// AddGoal_AttackTarget():让大脑生成攻击动作序列。
  pBot->GetBrain()->AddGoal_AttackTarget(); 
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RenderInfo:屏幕画 "AT: 分数";return 之后是原作者雪藏的调试代码。
void AttackTargetGoal_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
// 写字:AT = Attack(攻击)。
  gdi->TextAtPos(Position, "AT: " + ttos(CalculateDesirability(pBot), 2));
  return;
    
  std::string s = ttos(Raven_Feature::Health(pBot)) + ", " + ttos(Raven_Feature::TotalWeaponStrength(pBot));
  gdi->TextAtPos(Position+Vector2D(0,12), s);
}