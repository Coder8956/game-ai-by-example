//==============================================================================================
//【文件说明】Goal_HuntTarget.h —— "搜寻敌人"组合目标
//
//【这个文件是干什么的?】
//  机器人锁定了敌人但敌人跑出了视野,本目标负责把敌人找出来:
//  先奔往"最后一次看到敌人的位置(LRP)";到了还没看到就到处探索随机点;
//  等敌人重新进入视野就结束。继承自 Goal_Composite<Raven_Bot>。
//
//【完成】敌人进入视野;【失败/退出】目标死亡(没有活动目标)。
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h、../Raven_Bot.h。
//  (注意:本文件包含保护宏名是 GOAL_FIND_TARGET_H,原工程如此,未改。)
#ifndef GOAL_FIND_TARGET_H
#define GOAL_FIND_TARGET_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_HuntTarget.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   Causes a bot to search for its current target. Exits when target
//          is in view
//-----------------------------------------------------------------------------
#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"


//--------------------------------------------------------------------------------
// class Goal_HuntTarget : public Goal_Composite<Raven_Bot>:搜寻敌人是一种组合目标。
class Goal_HuntTarget : public Goal_Composite<Raven_Bot>
{
private:

  //this value is set to true if the last visible position of the target
  //bot has been searched without success
// m_bLVPTried:是否已搜过"最后可见位置"(Last Visible Position)。
//(原文注释:若已搜过敌人最后出现位置仍没找到,则置为 true)
  bool  m_bLVPTried;

// public:对外接口。
public:

// 构造:m_bLVPTried 先置 false。
  Goal_HuntTarget(Raven_Bot* pBot):Goal_Composite<Raven_Bot>(pBot, goal_hunt_target),
                                   m_bLVPTried(false)
  {}

   //the usual suspects
//(原文注释:照例的函数)—— Activate/Process/Terminate/Render;Terminate 空实现。
  void Activate();
  int  Process();
  void Terminate(){}

  void Render();


};





#endif
