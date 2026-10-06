//==============================================================================================
//【文件说明】Goal_NegotiateDoor.cpp —— "开门通过"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:清掉旧子目标,找到门边最近的开关位置,然后按"相反顺序"压入三个子目标:
//   ①穿过门(Goal_TraverseEdge) → ②走到门边 → ③走到开关旁。
//  (子目标是压栈执行,后加的先做,所以要反着加。)
#include "Goal_NegotiateDoor.h"
#include "..\Raven_Bot.h"
#include "..\Raven_Game.h"
#include "../navigation/Raven_PathPlanner.h"


#include "Goal_MoveToPosition.h"
#include "Goal_TraverseEdge.h"


#include "debug/DebugConsole.h"



//--------------------------------------------------------------------------------
// 构造函数:Goal_NegotiateDoor:: 表示这是该类的构造函数(跨两行书写)。
// 初始化列表把 pBot、编号 goal_negotiate_door 交给父类,
// 并存下边 edge 与是否最后一条边 LastEdge。
//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_NegotiateDoor::
Goal_NegotiateDoor(Raven_Bot*   pBot,
                   PathEdge     edge,
                   bool         LastEdge):Goal_Composite<Raven_Bot>(pBot, goal_negotiate_door),
                                        m_PathEdge(edge),
                                        m_bLastEdgeInPath(LastEdge)

{
}

//---------------------------- Activate ---------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_NegotiateDoor::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //if this goal is reactivated then there may be some existing subgoals that
  //must be removed
// 清掉旧子目标。
//(原文注释:本目标被重新激活时,可能有旧子目标要先清掉)
  RemoveAllSubgoals();
  
  //get the position of the closest navigable switch
// posSw=开关位置;DoorID()=这条边对应的门编号。
//(原文注释:找到最近的可通行开关的位置)
  Vector2D posSw = m_pOwner->GetWorld()->GetPosOfClosestSwitch(m_pOwner->Pos(),
                                                          m_PathEdge.DoorID());

  //because goals are *pushed* onto the front of the subgoal list they must
  //be added in reverse order.
//(原文注释:因为目标是"压"到子目标链表最前面执行的,所以要按相反顺序添加)
  
  //first the goal to traverse the edge that passes through the door
// ①穿过门。
//(原文注释:先加"穿过门那条边"的目标)
  AddSubgoal(new Goal_TraverseEdge(m_pOwner, m_PathEdge, m_bLastEdgeInPath));

  //next, the goal that will move the bot to the beginning of the edge that
  //passes through the door
// ②走到门边(边的起点 Source)。
//(原文注释:再加"走到门那条边的起点"的目标)
  AddSubgoal(new Goal_MoveToPosition(m_pOwner, m_PathEdge.Source()));
  
  //finally, the Goal that will direct the bot to the location of the switch
// ③走到开关旁。
//(原文注释:最后加"走到开关位置"的目标)
  AddSubgoal(new Goal_MoveToPosition(m_pOwner, posSw));
}


//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_NegotiateDoor::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();

  //process the subgoals
// 跑子目标,取回状态。
//(原文注释:处理子目标)
  m_iStatus = ProcessSubgoals();

  return m_iStatus;
}




