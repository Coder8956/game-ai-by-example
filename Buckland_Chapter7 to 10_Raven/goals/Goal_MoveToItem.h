//==============================================================================================
//【文件说明】Goal_MoveToItem.h —— (注意:这是另一套早期框架的遗留文件)
//
//【这个文件是干什么的?】
//  它继承自 Raven_Goal(不是本目录主流的 Goal<Raven_Bot>),引用的是
//  Raven_NavModule / Raven_GoalQ 等早期框架类,属于原工程里保留的旧版本代码。
//  作用仍是"让机器人朝某物品走",但只在 Initialize 里发起一次寻路请求就算完成。
//  本文件只加注释说明,不改任何代码。
//==============================================================================================
#ifndef GOAL_MOVE_TO_ITEM_H
#define GOAL_MOVE_TO_ITEM_H
#pragma warning (disable:4786)

#include "Raven_Goal.h"


// 前置声明:Raven_Bot(机器人类)在别处定义。
class Raven_Bot;


//--------------------------------------------------------------------------------
// class Goal_MoveToItem : public Raven_Goal:朝物品走是一种(旧框架)目标。
class Goal_MoveToItem : public Raven_Goal
{
private:

// m_iItemType:要去的物品类型编号。
  int  m_iItemType;

// public:对外接口。
public:

// 构造:把 pBot、物品类型 type 存下;注意这里编号传的是 goal_explore(旧写法)。
  Goal_MoveToItem(Raven_Bot* pBot,
               int        type):Raven_Goal(pBot, goal_explore),
                                m_iItemType(type)
  {}

// Initialize:初始化(发起寻路);Process/Terminate 此处空实现 {}。
  void Initialize();

  void Process(){}

  void Terminate(){}
};



#endif
