//==============================================================================================
//【文件说明】Goal_MoveToPosition.cpp —— "移动到某坐标"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:清空子目标、向路径规划器请求路径,路没算好前先加"直奔点"子目标;
//  Process:逐个子目标处理,子目标失败就重规划;
//  HandleMessage:收到路径规划器"路好了/无路"消息后切换子目标;
//  Render:画一个同心圆靶心表示目的地。
//==============================================================================================
#include "Goal_MoveToPosition.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "../navigation/Raven_PathPlanner.h"
#include "Messaging/Telegram.h"
#include "../Raven_Messages.h"
#include "misc/cgdi.h"

#include "Goal_SeekToPosition.h"
#include "Goal_FollowPath.h"



//------------------------------- Activate ------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_MoveToPosition::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //make sure the subgoal list is clear.
// 先把旧子目标全部清掉。
  RemoveAllSubgoals();

  //requests a path to the target position from the path planner. Because, for
  //demonstration purposes, the Raven path planner uses time-slicing when 
  //processing the path requests the bot may have to wait a few update cycles
  //before a path is calculated. Consequently, for appearances sake, it just
  //seeks directly to the target position whilst it's awaiting notification
  //that the path planning request has succeeded/failed
// 向路径规划器发起算路请求;返回 true 表示请求已受理。
//(原文注释:向路径规划器请求到目标点的路径。为演示,路径规划器用"时间片"
//           方式算路,机器人要等几帧才收到结果;等待期间它先直奔目标点)
  if (m_pOwner->GetPathPlanner()->RequestPathToPosition(m_vDestination))
  {
// new:在堆上创建对象(指针返回)。先加一个"直奔目标点"子目标顶着,等路算好再换。
    AddSubgoal(new Goal_SeekToPosition(m_pOwner, m_vDestination));
  }
}

//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_MoveToPosition::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();
    
  //process the subgoals
// 让父类跑一遍子目标,把组合后的状态写回 m_iStatus。
//(原文注释:依次处理各个子目标)
  m_iStatus = ProcessSubgoals();

  //if any of the subgoals have failed then this goal re-plans
// 父类便捷函数:失败就重新激活(重规划)。
//(原文注释:若任一子目标失败,本目标就重新规划)
  ReactivateIfFailed();

  return m_iStatus;
}

//---------------------------- HandleMessage ----------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// HandleMessage:处理路径规划器发来的消息。
bool Goal_MoveToPosition::HandleMessage(const Telegram& msg)
{
  //first, pass the message down the goal hierarchy
// bHandled=是否已被某个子目标处理(布尔型局部变量)。
//(原文注释:先把消息往子目标链最前端转发,看子目标能不能处理)
  bool bHandled = ForwardMessageToFrontMostSubgoal(msg);

  //if the msg was not handled, test to see if this goal can handle it
// == false:等于假(=没被处理过)。
//(原文注释:若子目标都没处理,则本目标自己判断能不能处理)
  if (bHandled == false)
  {
// msg.Msg=消息编号;按编号分支。
    switch(msg.Msg)
    {
    case Msg_PathReady:

      //clear any existing goals
//(原文注释:清掉现有子目标)
      RemoveAllSubgoals();

// 路径算好了:加"沿路径走"子目标,路径取自路径规划器的 GetPath()。
      AddSubgoal(new Goal_FollowPath(m_pOwner,
                                     m_pOwner->GetPathPlanner()->GetPath()));

      return true; //msg handled


    case Msg_NoPathAvailable:

// 路径规划器说无路可达 → 本目标失败。
      m_iStatus = failed;

      return true; //msg handled

// 不认识的消息 → 返回 false(没处理)。
    default: return false;
    }
  }

  //handled by subgoals
  return true;
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画目的地靶心(三层同心圆:蓝6→红4→黄2)。
void Goal_MoveToPosition::Render()
{
  //forward the request to the subgoals
// 先让父类画子目标的渲染。
  Goal_Composite<Raven_Bot>::Render();
  
  //draw a bullseye
  gdi->BlackPen();
  gdi->BlueBrush();
  gdi->Circle(m_vDestination, 6);
  gdi->RedBrush();
  gdi->RedPen();
  gdi->Circle(m_vDestination, 4);
  gdi->YellowBrush();
  gdi->YellowPen();
  gdi->Circle(m_vDestination, 2);
}

