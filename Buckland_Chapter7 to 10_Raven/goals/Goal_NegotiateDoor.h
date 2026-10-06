//==============================================================================================
//【文件说明】Goal_NegotiateDoor.h —— "开门通过"组合目标
//
//【这个文件是干什么的?】
//  路径上有一扇门时,机器人要先跑到门边的开关旁按开关开门,再穿过这道门。
//  本组合目标把这件事拆成三个子目标(按相反顺序压栈):先走到开关位置,
//  再走到门边,最后穿过边(见 .cpp 的 Activate)。继承自 Goal_Composite<Raven_Bot>。
//
//【它包含谁?】—— Goals/Goal_Composite.h、Raven_Goal_Types.h、../Raven_Bot.h、
//   ../navigation/PathEdge.h(路径边,一扇门对应一条边)。
//==============================================================================================
#ifndef GOAL_NEGOTIATE_DOOR_H
#define GOAL_NEGOTIATE_DOOR_H
#pragma warning (disable:4786)

#include "Goals/Goal_Composite.h"
#include "Raven_Goal_Types.h"
#include "../Raven_Bot.h"
#include "../navigation/PathEdge.h"


//--------------------------------------------------------------------------------
// class Goal_NegotiateDoor : public Goal_Composite<Raven_Bot>:开门是一种组合目标。
class Goal_NegotiateDoor : public Goal_Composite<Raven_Bot>
{
private:

// m_PathEdge:要通过的那条路径边(带门信息)。
  PathEdge m_PathEdge;

// m_bLastEdgeInPath:这条边是不是整条路径的最后一条(布尔)。
  bool     m_bLastEdgeInPath;

// public:对外接口。
public:

// 构造:pBot=机器人,edge=要过的边,LastEdge=是否最后一条边。
  Goal_NegotiateDoor(Raven_Bot* pBot, PathEdge edge, bool LastEdge);

 //the usual suspects
//(原文注释:照例的函数)—— Activate/Process;Terminate 空实现 {}。
  void Activate();
  int  Process();
  void Terminate(){}
};



#endif
