//==============================================================================================
//【文件说明】Goal_AdjustRange.h —— "调整与敌人的距离"目标
//
//【这个文件是干什么的?】
//  战斗时让机器人和敌人保持在"理想射程"附近:太近就后退、太远就逼近。
//  本文件里实际的距离判断代码被原作者注释掉了(见 .cpp 的 /* */ 块),
//  是个未完成的占位实现。继承自 Goal<Raven_Bot>。
//
//【它包含谁?】—— goals/Goal.h、Raven_Goal_Types.h、../Raven_Bot.h。
//==============================================================================================
#ifndef GOAL_ADJUST_RANGE_H
#define GOAL_ADJUST_RANGE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_AdjustRange.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   
//
//-----------------------------------------------------------------------------
#include "goals/Goal.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"





//--------------------------------------------------------------------------------
// class Goal_AdjustRange : public Goal<Raven_Bot>:调整距离是一种目标。
class Goal_AdjustRange : public Goal<Raven_Bot>
{
private:

// m_pTarget:指向当前敌人的指针(m_p=指针成员)。
  Raven_Bot*  m_pTarget;

// m_dIdealRange:理想射程距离(希望和敌人保持的间距)。
  double       m_dIdealRange;

// public:对外接口。
public:

// 声明:构造/Activate/Process/Terminate 标准动作(实现在 .cpp)。
  Goal_AdjustRange(Raven_Bot* pBot);

  void Activate();

  int  Process();

  void Terminate();
 
};






#endif
