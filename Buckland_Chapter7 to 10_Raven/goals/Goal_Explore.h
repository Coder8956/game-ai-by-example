//==============================================================================================
//【文件说明】Goal_Explore.h —— "到处探索"组合目标
//
//【这个文件是干什么的?】
//  让机器人随机挑一个地图上的点作为目的地,走过去;到了再挑下一个,四处转。
//  继承自组合目标 Goal_Composite<Raven_Bot>。内部:先选随机点、请求路径;
//  路算好前先直奔该点;路算好后换成"沿路径走"。
//
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h;前置声明 Raven_Bot。
//==============================================================================================
#ifndef GOAL_EXPLORE_H
#define GOAL_EXPLORE_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"


class Raven_Bot;


//--------------------------------------------------------------------------------
// class Goal_Explore : public Goal_Composite<Raven_Bot>:探索是一种组合目标。
class Goal_Explore : public Goal_Composite<Raven_Bot>
{
private:
  
// m_CurrentDestination:当前探索目的地坐标。
  Vector2D  m_CurrentDestination;

  //set to true when the destination for the exploration has been established
// m_bDestinationIsSet:目的地是否已确定(布尔)。
//(原文注释:探索目的地一旦确定,就置为 true)
  bool      m_bDestinationIsSet;

// public:对外接口。
public:

// 构造:pOwner=所属机器人;m_bDestinationIsSet 先置 false(还没选目的地)。
  Goal_Explore(Raven_Bot* pOwner):Goal_Composite<Raven_Bot>(pOwner,
                                                            goal_explore),
                                  m_bDestinationIsSet(false)
  {}


  void Activate();

  int Process();

// Activate/Process 标准动作;Terminate 空实现 {};HandleMessage 处理路径消息。
  void Terminate(){}

  bool HandleMessage(const Telegram& msg);
};





#endif
