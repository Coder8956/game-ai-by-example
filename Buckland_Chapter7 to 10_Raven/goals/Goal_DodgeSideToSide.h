//==============================================================================================
//【文件说明】Goal_DodgeSideToSide.h —— "左右横移躲子弹"目标(strafe 横移)
//
//【这个文件是干什么的?】
//  战斗中让机器人左右来回横移(strafe),躲开敌人射来的子弹。
//  随机决定先往左还是先往右(m_bClockwise);走到横移点就停下换方向;
//  敌人脱离视野就结束。继承自 Goal<Raven_Bot>。
//
//【它包含谁?】—— Goals/Goal.h、Raven_Goal_Types.h、../Raven_Bot.h。
//==============================================================================================
#ifndef GOAL_DODGE_SIDE_H
#define GOAL_DODGE_SIDE_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_DodgeSideToSide.h
//
//  Author: Mat Buckland (ai-junkie.com)
//
//  Desc:   this goal makes the bot dodge from side to side
//
//-----------------------------------------------------------------------------
#include "Goals/Goal.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"





//--------------------------------------------------------------------------------
// class Goal_DodgeSideToSide : public Goal<Raven_Bot>:横移躲避是一种目标。
class Goal_DodgeSideToSide : public Goal<Raven_Bot>
{
private:

// m_vStrafeTarget:当前横移目标点(往左/右走一小步的目的地)。
  Vector2D    m_vStrafeTarget;

// m_bClockwise:当前方向标志(布尔型;true=向右那一侧,反了就取反)。
  bool        m_bClockwise;

// GetStrafeTarget:算下一个横移点;const=只读函数。
  Vector2D  GetStrafeTarget()const;


// public:对外接口。
public:

// 构造:RandBool() 随机一个真假,决定先往左还是右走。
  Goal_DodgeSideToSide(Raven_Bot* pBot):Goal<Raven_Bot>(pBot, goal_strafe),
                                        m_bClockwise(RandBool())
  {}


// 标准动作:Activate/Process/Render/Terminate。
  void Activate();

  int  Process();

  void Render();

  void Terminate();
 
};






#endif
