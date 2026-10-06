//==============================================================================================
//【文件说明】Goal_GetItem.cpp —— "去捡物品"组合目标的实现
//
//【这个文件是干什么的?】
//  ItemTypeToGoalType:物品编号→目标编号;Activate:请求路径+先闲逛;
//  Process:检查物品是否被抢,并跑子目标;HandleMessage:路径好了换成沿路径走;
//  hasItemBeenStolen:物品触发器已失效(被捡走)且我们能看到那里 → 判定被抢。
//==============================================================================================
#include "Goal_GetItem.h"
#include "../Raven_ObjectEnumerations.h"
#include "../Raven_Bot.h"
#include "../navigation/Raven_PathPlanner.h"

#include "Messaging/Telegram.h"
#include "..\Raven_Messages.h"

#include "Goal_Wander.h"
#include "Goal_FollowPath.h"


//--------------------------------------------------------------------------------
// ItemTypeToGoalType:按物品编号 switch 翻成对应目标编号;不认识就抛异常。
int ItemTypeToGoalType(int gt)
{
  switch(gt)
  {
  case type_health:

    return goal_get_health;

  case type_shotgun:

    return goal_get_shotgun;

  case type_rail_gun:

    return goal_get_railgun;

  case type_rocket_launcher:

    return goal_get_rocket_launcher;

// throw:抛异常中止;std::runtime_error=标准运行时错误(不认识的物品类型)。
  default: throw std::runtime_error("Goal_GetItem cannot determine item type");

  }//end switch
}

//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_GetItem::Activate()
{
// 状态=进行中;m_pGiverTrigger 先清零(还没拿到触发器指针)。
  m_iStatus = active;
  
  m_pGiverTrigger = 0;
  
  //request a path to the item
// 发起"到某类物品"的算路请求。
//(原文注释:向路径规划器请求一条到物品的路径)
  m_pOwner->GetPathPlanner()->RequestPathToItem(m_iItemToGet);

  //the bot may have to wait a few update cycles before a path is calculated
  //so for appearances sake it just wanders
// 先挂一个闲逛子目标顶着。
//(原文注释:算路要等几帧,这期间让机器人先闲逛,别傻站着)
  AddSubgoal(new Goal_Wander(m_pOwner));

}

//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_GetItem::Process()
{
// 未激活就先激活。
  ActivateIfInactive();

// 物品被抢了 → 终止本目标。
  if (hasItemBeenStolen())
  {
    Terminate();
  }

  else
  {
    //process the subgoals
// 跑子目标,取回状态。
//(原文注释:处理子目标)
    m_iStatus = ProcessSubgoals();
  }

  return m_iStatus;
}
//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// HandleMessage:处理路径规划器消息。
bool Goal_GetItem::HandleMessage(const Telegram& msg)
{
  //first, pass the message down the goal hierarchy
// 看子目标能否处理该消息。
//(原文注释:先把消息往子目标链转发)
  bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

  //if the msg was not handled, test to see if this goal can handle it
  if (bHandled == false)
  {
// 按消息编号分支。
    switch(msg.Msg)
    {
    case Msg_PathReady:

      //clear any existing goals
//(原文注释:清掉现有子目标)
      RemoveAllSubgoals();

// 路算好了:挂"沿路径走"子目标。
      AddSubgoal(new Goal_FollowPath(m_pOwner,
                                     m_pOwner->GetPathPlanner()->GetPath()));

      //get the pointer to the item
// static_cast<类型*>(...):显式类型转换,把消息附带信息转成触发器指针。
//(原文注释:取出指向该物品触发器的指针)
      m_pGiverTrigger = static_cast<Raven_Map::TriggerType*>(msg.ExtraInfo);

      return true; //msg handled


    case Msg_NoPathAvailable:

// 无路可达 → 本目标失败。
      m_iStatus = failed;

      return true; //msg handled

// 不认识的消息 → 没处理。
    default: return false;
    }
  }

  //handled by subgoals
  return true;
}

//---------------------------- hasItemBeenStolen ------------------------------
//
//  returns true if the bot sees that the item it is heading for has been
//  picked up by an opponent
//(原文注释:若机器人看到要捡的物品已被对手捡走,返回 true)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// hasItemBeenStolen:物品是否被抢。三个条件同时满足:
// ①拿到了触发器指针;②触发器已不活跃(被人捡走失效);③我们能看到那个位置(LOS=视线)。
bool Goal_GetItem::hasItemBeenStolen()const
{
// && :逻辑与——三个条件都为真才算被抢。
  if (m_pGiverTrigger &&
      !m_pGiverTrigger->isActive() &&
      m_pOwner->hasLOSto(m_pGiverTrigger->Pos()) )
  {
    return true;
  }

  return false;
}