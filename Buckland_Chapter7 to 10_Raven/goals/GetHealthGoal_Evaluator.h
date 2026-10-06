//==============================================================================================
//【文件说明】GetHealthGoal_Evaluator.h —— "去吃血包"目标的评估器
//
//【这个文件是干什么的?】
//  继承自 Goal_Evaluator。它回答:"现在机器人该不该去捡血包(health)?"
//  打分规则(见 .cpp):血量越低、离血包越近 → 越想去吃血包。
//
//【谁在使用这个文件?】—— Goal_Think.cpp 初始化时 new 本评估器挂进决策大脑。
//【本文件包含了谁?】—— Goal_Evaluator.h(父类)、../Raven_Bot.h(机器人类)。
//==============================================================================================
#ifndef RAVEN_HEALTH_EVALUATOR
#define RAVEN_HEALTH_EVALUATOR
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   GetHealthGoal_Evaluator.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to calculate how desirable the goal of fetching a health item
//          is
//-----------------------------------------------------------------------------

#include "Goal_Evaluator.h"
#include "../Raven_Bot.h"

//--------------------------------------------------------------------------------
// 继承:GetHealthGoal_Evaluator : public Goal_Evaluator("吃血评估器"是一种评估器)。
class GetHealthGoal_Evaluator : public Goal_Evaluator
{
public:

// 构造函数:bias 转发给父类。
  GetHealthGoal_Evaluator(double bias):Goal_Evaluator(bias){}
  
// 重写:算"吃血包"的渴望分(实现见 .cpp)。
  double CalculateDesirability(Raven_Bot* pBot);

// 重写:选中后给机器人脑子加"去捡血包"目标。
  void  SetGoal(Raven_Bot* pEnt);

// 重写:屏幕上画调试分。
  void RenderInfo(Vector2D Position, Raven_Bot* pBot);
};

#endif
    
