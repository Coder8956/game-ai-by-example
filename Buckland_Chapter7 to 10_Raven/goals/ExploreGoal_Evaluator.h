//==============================================================================================
//【文件说明】ExploreGoal_Evaluator.h —— "去探索"目标的评估器(给"探索"打分)
//
//【这个文件是干什么的?】
//  继承自 Goal_Evaluator(基类规矩见 Goal_Evaluator.h)。它负责回答:"现在机器人该不该去探索?"
//  本实现很简单:固定给探索一个很小的基础分 0.05,再乘性格偏置——
//  意思是"只要没有更诱人的目标,机器人就去到处转转",避免它站着发呆。
//
//【谁在使用这个文件?】—— Goal_Think.cpp 在初始化时 new 出本评估器,挂到决策大脑里。
//【本文件包含了谁?】—— Goal_Evaluator.h(父类)、../Raven_Bot.h(机器人类)。
//==============================================================================================
#ifndef RAVEN_EXPLORE_GOAL_EVALUATOR
#define RAVEN_EXPLORE_GOAL_EVALUATOR
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   ExploreGoal_Evaluator.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:  class to calculate how desirable the goal of exploring is
//-----------------------------------------------------------------------------

#include "Goal_Evaluator.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// 继承关系:ExploreGoal_Evaluator : public Goal_Evaluator
//  = "探索评估器"是一种"目标评估器"(public 继承,公开接口全部沿用)。
class ExploreGoal_Evaluator : public Goal_Evaluator
{ 
public:

// 构造函数:把 bias 转发给父类 Goal_Evaluator 存进性格偏置(初始化列表写法)。
  ExploreGoal_Evaluator(double bias):Goal_Evaluator(bias){}
  
// 重写父类纯虚函数:计算"探索"的渴望分(实现见 .cpp)。
  double CalculateDesirability(Raven_Bot* pBot);

// 重写:一旦"探索"被选中,就把探索目标加进机器人脑子。pEnt 是参数名(实体指针)。
  void  SetGoal(Raven_Bot* pEnt);

// 重写:在屏幕 Position 处画出探索项的调试分。
  void RenderInfo(Vector2D Position, Raven_Bot* pBot);
};

#endif