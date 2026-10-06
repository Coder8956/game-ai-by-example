//==============================================================================================
//【文件说明】Goal_TraverseEdge.cpp —— "走过一条路径边"目标的实现
//
//【这个文件是干什么的?】
//  Activate:按边的类型(游泳/爬行)调速,记下开始时间,算预计到达时间,
//  然后开 seek 或 arrive 转向朝边的终点走;Process:卡住则失败,到点则完成;
//  Terminate:关转向、速度复原;Render:画当前这条边。
//==============================================================================================
#include "Goal_TraverseEdge.h"
#include "..\Raven_Bot.h"
#include "Raven_Goal_Types.h"
#include "..\Raven_SteeringBehaviors.h"
#include "time/CrudeTimer.h"
#include "..\constants.h"
#include "../navigation/Raven_PathPlanner.h"
#include "misc/cgdi.h"
#include "../lua/Raven_Scriptor.h"


#include "debug/DebugConsole.h"



//--------------------------------------------------------------------------------
// 构造函数:把 pBot、编号 goal_traverse_edge 交给父类,存下边 edge;
// m_dTimeExpected 先置 0(Activate 时再算)。
//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_TraverseEdge::Goal_TraverseEdge(Raven_Bot* pBot,
                                     PathEdge   edge,
                                     bool       LastEdge):

                                Goal<Raven_Bot>(pBot, goal_traverse_edge),
                                m_Edge(edge),
                                m_dTimeExpected(0.0),
                                m_bLastEdgeInPath(LastEdge)
                                
{}

                            
                                             
//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
//--------------------------------------------------------------------------------
// Activate:激活。
void Goal_TraverseEdge::Activate()
{
// 状态=进行中。
  m_iStatus = active;
  
  //the edge behavior flag may specify a type of movement that necessitates a 
  //change in the bot's max possible speed as it follows this edge
// 按边的行为类型分支:swim=游泳、crawl=爬行(分别调慢速度)。
//(原文注释:这条边的行为标志可能要求机器人走这条边时改变最大速度)
  switch(m_Edge.Behavior())
  {
// NavGraphEdge::swim:枚举值,表示这条边是"游泳区"。
    case NavGraphEdge::swim:
    {
// 把最大速度设成游泳速度(从 lua 脚本配置读)。
      m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSwimmingSpeed"));
    }
   
    break;
   
// 爬行区同理,调成爬行速度。
    case NavGraphEdge::crawl:
    {
       m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxCrawlingSpeed"));
    }
   
    break;
  }
  

  //record the time the bot starts this goal
// Clock 单例取当前游戏时间,存为开始时刻。
//(原文注释:记录开始本目标的时刻)
  m_dStartTime = Clock->GetCurrentTime();   
  
  //calculate the expected time required to reach the this waypoint. This value
  //is used to determine if the bot becomes stuck 
// 问机器人按它的速度走到边的终点要多久。
//(原文注释:算出到该路径点预计所需时间,用来判断机器人是否卡住)
  m_dTimeExpected = m_pOwner->CalculateTimeToReachPosition(m_Edge.Destination());
  
  //factor in a margin of error for any reactive behavior
// static const:静态常量(全类共享,只初始化一次);余量 2 秒。
//(原文注释:为反应性行为预留误差余量)
  static const double MarginOfError = 2.0;

// 把余量叠到预计时间上。
  m_dTimeExpected += MarginOfError;


  //set the steering target
// 转向目标设为边的终点 Destination。
//(原文注释:设置转向目标点)
  m_pOwner->GetSteering()->SetTarget(m_Edge.Destination());

  //Set the appropriate steering behavior. If this is the last edge in the path
  //the bot should arrive at the position it points to, else it should seek
// 最后一条边 → arrive(到站减速停下)。
//(原文注释:若这是最后一条边,机器人应精准到站(arrive);否则直接 seek 冲过去)
  if (m_bLastEdgeInPath)
  {
// 开 arrive 转向。
     m_pOwner->GetSteering()->ArriveOn();
  }

  else
  {
// 否则开 seek(直线朝点跑,不减速)。
    m_pOwner->GetSteering()->SeekOn();
  }
}



//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Process:每帧调用,返回状态码。
int Goal_TraverseEdge::Process()
{
  //if status is inactive, call Activate()
// 父类便捷函数。
//(原文注释:未激活就先激活)
  ActivateIfInactive();
  
  //if the bot has become stuck return failure
// 卡住 → 状态=失败。
//(原文注释:机器人若卡住,则返回失败)
  if (isStuck())
  {
    m_iStatus = failed;
  }
  
  //if the bot has reached the end of the edge return completed
//(原文注释:若机器人已到边的终点,则返回完成)
  else
  { 
// 已到边的终点 → 状态=完成。
    if (m_pOwner->isAtPosition(m_Edge.Destination()))
    {
      m_iStatus = completed;
    }
  }

  return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//(原文注释:若到达当前路径点花的时间超过预期,返回 true)
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// isStuck:判断是否卡住;const=只读函数。
bool Goal_TraverseEdge::isStuck()const
{  
// 已用时间 = 当前时间 - 开始时间。
  double TimeTaken = Clock->GetCurrentTime() - m_dStartTime;

// 超过预计时间 → 卡住。
  if (TimeTaken > m_dTimeExpected)
  {
// 调试控制台打印"某机器人卡住了"。
    debug_con << "BOT " << m_pOwner->ID() << " IS STUCK!!" << "";

    return true;
  }

  return false;
}


//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Terminate:终止。
void Goal_TraverseEdge::Terminate()
{
  //turn off steering behaviors.
// 关 seek/arrive。
//(原文注释:关掉各转向行为)
  m_pOwner->GetSteering()->SeekOff();
  m_pOwner->GetSteering()->ArriveOff();

  //return max speed back to normal
// 速度复原为默认最大速度。
//(原文注释:把最大速度恢复成正常值)
  m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSpeed"));
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// Render:激活中画一条蓝线连到边终点,绿圈标出终点。
void Goal_TraverseEdge::Render()
{
// 只在进行中画。
  if (m_iStatus == active)
  {
    gdi->BluePen();
    gdi->Line(m_pOwner->Pos(), m_Edge.Destination());
    gdi->GreenBrush();
    gdi->BlackPen();
    gdi->Circle(m_Edge.Destination(), 3);
  }
}

