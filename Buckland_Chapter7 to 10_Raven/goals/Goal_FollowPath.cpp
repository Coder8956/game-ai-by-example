//==============================================================================================
//【文件说明】Goal_FollowPath.cpp —— "沿整条路径走"组合目标的实现
//
//【这个文件是干什么的?】
//  Activate:取出链表最前面那条边,按它的类型挂子目标(普通边走 TraverseEdge,
//  过门边挂 NegotiateDoor);Process:子目标跑完且路径还有边就再 Activate 取下一条;
//  Render:把剩余路径画成带箭头的线。
#include "Goal_FollowPath.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"

#include "Goal_TraverseEdge.h"
#include "Goal_NegotiateDoor.h"
#include "misc/cgdi.h"



//--------------------------------------------------------------------------------
// 构造函数:把 pBot、编号 goal_follow_path 交给父类,并把路径 path 拷进 m_Path。
//------------------------------ ctor -----------------------------------------
//-----------------------------------------------------------------------------
Goal_FollowPath::
Goal_FollowPath(Raven_Bot*          pBot,
                std::list<PathEdge> path):Goal_Composite<Raven_Bot>(pBot, goal_follow_path),
                                                  m_Path(path)
{
}


//------------------------------ Activate -------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_FollowPath::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //get a reference to the next edge
// front():取链表最前面的元素。
//(原文注释:取出下一条边)
  PathEdge edge = m_Path.front();

  //remove the edge from the path
// pop_front():删除链表最前面的元素。
//(原文注释:把这条边从路径里删掉)
  m_Path.pop_front(); 

  //some edges specify that the bot should use a specific behavior when
  //following them. This switch statement queries the edge behavior flag and
  //adds the appropriate goals/s to the subgoal list.
// 按边的类型分支。
//(原文注释:按边的行为标志,把对应的子目标加进子目标列表)
  switch(edge.Behavior())
  {
// normal=普通边:挂"走过这条边"子目标;m_Path.empty()=是不是最后一条。
  case NavGraphEdge::normal:
    {
      AddSubgoal(new Goal_TraverseEdge(m_pOwner, edge, m_Path.empty()));
    }

    break;

// goes_through_door=过门边:挂"开门通过"子目标。
  case NavGraphEdge::goes_through_door:
    {

      //also add a goal that is able to handle opening the door
      AddSubgoal(new Goal_NegotiateDoor(m_pOwner, edge, m_Path.empty()));
    }

    break;

// jump=跳跃边(此处原作者未写具体子目标,留空)。
  case NavGraphEdge::jump:
    {
      //add subgoal to jump along the edge
    }

    break;

// grapple=抓钩边(同样留空未实现)。
  case NavGraphEdge::grapple:
    {
      //add subgoal to grapple along the edge
    }

    break;

// default:不认识的边类型 → 抛异常报错。
  default:

// throw:抛异常中止。
    throw std::runtime_error("<Goal_FollowPath::Activate>: Unrecognized edge type");
  }
}


//-------------------------- Process ------------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_FollowPath::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();

  m_iStatus = ProcessSubgoals();

  //if there are no subgoals present check to see if the path still has edges.
  //remaining. If it does then call activate to grab the next edge.
// && :逻辑与——本边已完成 且 路径还有边。
//(原文注释:若没有子目标了,检查路径里是否还剩边;有就再 Activate 取下一条)
  if (m_iStatus == completed && !m_Path.empty())
  {
// 再激活一次,取下一条边。
    Activate(); 
  }

  return m_iStatus;
}
 
//---------------------------- Render -----------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:画剩余路径。
void Goal_FollowPath::Render()
{ 
  //render all the path waypoints remaining on the path list
// iterator=迭代器(遍历链表用的指针式对象);for 循环逐边画出箭头线和终点圆点。
//(原文注释:画出路径链表上剩余的所有路径点)
  std::list<PathEdge>::iterator it;
  for (it = m_Path.begin(); it != m_Path.end(); ++it)
  {  
    gdi->BlackPen();
    gdi->LineWithArrow(it->Source(), it->Destination(), 5);
    
    gdi->RedBrush();
    gdi->BlackPen();
    gdi->Circle(it->Destination(), 3);
  }

  //forward the request to the subgoals
// 再让父类画子目标的渲染。
  Goal_Composite<Raven_Bot>::Render();
}
  




