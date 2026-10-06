//==============================================================================================
//【文件说明】Goal_Explore.cpp —— "到处探索"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:选一个随机地图点当目的地、请求路径,先挂"直奔该点"子目标;
//  Process:跑子目标;HandleMessage:路好了换成"沿路径走",无路则失败。
//==============================================================================================
#include "Goal_Explore.h"
#include "../Raven_Bot.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "Messaging/Telegram.h"
#include "..\Raven_Messages.h"

#include "Goal_SeekToPosition.h"
#include "Goal_FollowPath.h"



//------------------------------ Activate -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_Explore::Activate()
{
// 状态=进行中。
  m_iStatus = active;

  //if this goal is reactivated then there may be some existing subgoals that
  //must be removed
//(原文注释:重新激活时,旧子目标要先清掉)
// 清掉旧子目标。
  RemoveAllSubgoals();

// ! :取反——目的地还没选过,就选一个随机点。
  if (!m_bDestinationIsSet)
  {
    //grab a random location
// GetMap()->GetRandomNodeLocation():从地图节点里随机取一个点。
//(原文注释:随机挑一个位置)
    m_CurrentDestination = m_pOwner->GetWorld()->GetMap()->GetRandomNodeLocation();

// 标记目的地已确定(下次重激活不重复选)。
    m_bDestinationIsSet = true;
  }

  //and request a path to that position
// 发起寻路请求。
//(原文注释:并向路径规划器请求到该点的路径)
  m_pOwner->GetPathPlanner()->RequestPathToPosition(m_CurrentDestination);

  //the bot may have to wait a few update cycles before a path is calculated
  //so for appearances sake it simple ARRIVES at the destination until a path
  //has been found
// 先挂"直奔目的地"子目标顶着。
//(原文注释:算路要等几帧,这期间先让机器人直奔目的地,等路算好再换)
  AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_CurrentDestination));
}

//------------------------------ Process -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_Explore::Process()
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


//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// HandleMessage:处理路径规划器消息。
bool Goal_Explore::HandleMessage(const Telegram& msg)
{
  //first, pass the message down the goal hierarchy
// 看子目标能否处理。
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

      return true; //msg handled


    case Msg_NoPathAvailable:

// 无路可达 → 失败。
      m_iStatus = failed;

      return true; //msg handled

// 不认识的消息 → 没处理。
    default: return false;
    }
  }

  //handled by subgoals
  return true;
}




