//==============================================================================================
//【文件说明】Goal_FollowPath.h —— "沿整条路径走"组合目标
//
//【这个文件是干什么的?】
//  路径规划器算出的是一串"边"(std::list<PathEdge>)。本目标把这条路径存下来,
//  然后一条边一条边地走完:每取最前面的一条边,按它的类型(普通/过门/跳/抓钩)
//  挂上对应的子目标(走边 Goal_TraverseEdge 或过门 Goal_NegotiateDoor)。
//
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h、../Raven_Bot.h、
//   ../navigation/Raven_PathPlanner.h、../navigation/PathEdge.h。
//==============================================================================================
#ifndef GOAL_FOLLOWPATH_H
#define GOAL_FOLLOWPATH_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Goal_FollowPath.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:
//-----------------------------------------------------------------------------
#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../navigation/PathEdge.h"



//--------------------------------------------------------------------------------
// class Goal_FollowPath : public Goal_Composite<Raven_Bot>:沿路走是一种组合目标。
class Goal_FollowPath : public Goal_Composite<Raven_Bot>
{
private:

  //a local copy of the path returned by the path planner
// m_Path:一条路径=一串边的链表(std::list 是 C++ 标准链表容器)。
//(原文注释:路径规划器返回的路径的本地副本)
  std::list<PathEdge>  m_Path;

// public:对外接口。
public:

// 构造:pBot=机器人,path=整条路径(拷一份存进 m_Path)。
  Goal_FollowPath(Raven_Bot* pBot, std::list<PathEdge> path);

  //the usual suspects
//(原文注释:照例的函数)—— Activate/Process/Render;Terminate 空实现 {}。
  void Activate();
  int Process();
  void Render();
  void Terminate(){}
};

#endif

