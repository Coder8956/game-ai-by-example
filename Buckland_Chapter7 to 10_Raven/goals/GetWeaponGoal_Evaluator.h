//==============================================================================================
//【文件说明】GetWeaponGoal_Evaluator.h —— "去捡某种武器"目标的评估器
//
//【这个文件是干什么的?】
//  继承自 Goal_Evaluator。和"吃血评估器"很像,但它评估的是"去捡某件武器"。
//  与吃血评估器不同:它记住"是哪种武器"(m_iWeaponType),可以给轨道炮/火箭筒/霰弹枪
//  各 new 一个评估器,分别打分。
//
//【谁在使用这个文件?】—— Goal_Think.cpp 初始化时按武器型号 new 若干个本评估器。
//【本文件包含了谁?】—— Goal_Evaluator.h(父类)、../Raven_Bot.h(机器人类)。
//==============================================================================================
#ifndef RAVEN_WEAPON_EVALUATOR
#define RAVEN_WEAPON_EVALUATOR
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   GetWeaponGoal_Evaluator.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:  class to calculate how desirable the goal of fetching a weapon item
//         is 
//-----------------------------------------------------------------------------

#include "Goal_Evaluator.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// 继承:GetWeaponGoal_Evaluator : public Goal_Evaluator。
class GetWeaponGoal_Evaluator : public Goal_Evaluator
{ 
// 私有成员:本评估器盯的是哪种武器(m_i=整数成员;如 type_rail_gun)。
  int   m_iWeaponType;

// public:对外接口。
public:

// 构造函数:bias 转发给父类,WeaponType 存进 m_iWeaponType(两个初始化项)。
  GetWeaponGoal_Evaluator(double bias,
                          int   WeaponType):Goal_Evaluator(bias),
                                            m_iWeaponType(WeaponType)
  {}
  
// 重写:算"捡这件武器"的渴望分。
  double CalculateDesirability(Raven_Bot* pBot);

// 重写:选中后加"去捡武器"目标。
  void  SetGoal(Raven_Bot* pEnt);

// 重写:屏幕画调试分。
  void  RenderInfo(Vector2D Position, Raven_Bot* pBot);
};

#endif