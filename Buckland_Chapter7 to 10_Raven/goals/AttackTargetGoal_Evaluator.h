//==============================================================================================
//【文件说明】AttackTargetGoal_Evaluator.h —— "攻击当前目标"的评估器
//
//【这个文件是干什么的?】
//  继承自 Goal_Evaluator。它回答:"现在机器人该不该开打?"
//  规则(见 .cpp):只要当前锁定了敌人,就按"血量 × 总火力"打分——
//  血多、武器强时最敢主动进攻。
//
//【谁在使用这个文件?】—— Goal_Think.cpp 初始化时 new 本评估器挂进决策大脑。
//【本文件包含了谁?】—— Goal_Evaluator.h(父类)、../Raven_Bot.h(机器人类)。
//==============================================================================================
#ifndef RAVEN_ATTACK_GOAL_EVALUATOR
#define RAVEN_ATTACK_GOAL_EVALUATOR
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   AttackTargetGoal_Evaluator.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:  class to calculate how desirable the goal of attacking the bot's
//         current target is
//-----------------------------------------------------------------------------

#include "Goal_Evaluator.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// 继承:AttackTargetGoal_Evaluator : public Goal_Evaluator。
class AttackTargetGoal_Evaluator : public Goal_Evaluator
{ 
public:

// 构造函数:bias 转发给父类。
  AttackTargetGoal_Evaluator(double bias):Goal_Evaluator(bias){}
  
// 重写:算"攻击目标"的渴望分。
  double CalculateDesirability(Raven_Bot* pBot);

// 重写:选中后加"攻击目标"目标。
  void  SetGoal(Raven_Bot* pEnt);

// 重写:屏幕画调试分。
  void RenderInfo(Vector2D Position, Raven_Bot* pBot);
};



#endif