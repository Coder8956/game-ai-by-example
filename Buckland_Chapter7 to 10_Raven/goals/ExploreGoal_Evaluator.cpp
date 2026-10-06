//==============================================================================================
//【文件说明】ExploreGoal_Evaluator.cpp —— "去探索"评估器的实现
//
//【这个文件是干什么的?】
//  实现三个重写函数:CalculateDesirability(给探索打个固定低分)、
//  SetGoal(选中就给机器人脑子加探索目标)、RenderInfo(屏幕上画调试分 EX: x.xx)。
//==============================================================================================
#include "ExploreGoal_Evaluator.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../Raven_ObjectEnumerations.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/Stream_Utility_Functions.h"
#include "Raven_Feature.h"

#include "Goal_Think.h"
#include "Raven_Goal_Types.h"




//---------------- CalculateDesirability -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// CalculateDesirability:返回"探索"的渴望分。
double ExploreGoal_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
// 固定基础分 0.05:不高不低,意味着"没事干时才去探索"。
  double Desirability = 0.05;

// *= :乘等——Desirability = Desirability × 性格偏置 m_dCharacterBias(父类成员)。
  Desirability *= m_dCharacterBias;

  return Desirability;
}

//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// SetGoal:把"探索"目标加进机器人的决策大脑。GetBrain()=取大脑指针;AddGoal_Explore()=加探索。
void ExploreGoal_Evaluator::SetGoal(Raven_Bot* pBot)
{
  pBot->GetBrain()->AddGoal_Explore();
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// RenderInfo:在 Position 处画 "EX: 分数"。ttos(数字, 小数位)=数字转字符串工具函数;
void ExploreGoal_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
// gdi 是全局绘图单例;TextAtPos(位置, 文本)=在屏幕写字。
  gdi->TextAtPos(Position, "EX: " + ttos(CalculateDesirability(pBot), 2));
}