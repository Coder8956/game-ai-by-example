//==============================================================================================
//【文件说明】Goal_TraverseEdge.h —— "走过一条路径边"目标
//
//【这个文件是干什么的?】
//  路径由很多段"边"连成,本目标负责让机器人走完其中一条边(从这头走到那头)。
//  它会按边的类型(游泳/爬行)调整最大速度;超时没到判定卡住(failed);到点 completed。
//  继承自 Goal<Raven_Bot>。
//
//【它包含谁?】—— Goals/Goal.h、2d/Vector2D.h、../navigation/Raven_PathPlanner.h、
//   ../navigation/PathEdge.h(路径边类)。
//==============================================================================================
#ifndef GOAL_TRAVERSE_EDGE_H
#define GOAL_TRAVERSE_EDGE_H
#pragma warning (disable:4786)

#include "Goals/Goal.h"
#include "2d/Vector2D.h"
#include "../navigation/Raven_PathPlanner.h"
#include "../navigation/PathEdge.h"


//--------------------------------------------------------------------------------
// class Goal_TraverseEdge : public Goal<Raven_Bot>:走过一条边是一种目标。
class Goal_TraverseEdge : public Goal<Raven_Bot>
{
private:

  //the edge the bot will follow
// m_Edge:要走的边。
//(原文注释:机器人要走的那条边)
  PathEdge  m_Edge;

  //true if m_Edge is the last in the path.
// 是否最后一条边(最后一条要精准停下 arrive,否则直接 seek 冲过去)。
//(原文注释:若 m_Edge 是整条路径的最后一条边,则为 true)
  bool      m_bLastEdgeInPath;

  //the estimated time the bot should take to traverse the edge
// 预计时间,超时即卡住。
//(原文注释:走完这条边预计该花的时间)
  double     m_dTimeExpected;
  
  //this records the time this goal was activated
// 激活时刻。
//(原文注释:记录本目标被激活的时刻)
  double     m_dStartTime;

  //returns true if the bot gets stuck
// isStuck:是否卡住;const=只读函数。
//(原文注释:机器人卡住则返回 true)
  bool      isStuck()const;

// public:对外接口。
public:

// 构造:pBot=机器人,edge=要走的边,LastEdge=是否最后一条边。
  Goal_TraverseEdge(Raven_Bot* pBot,
                    PathEdge   edge,
                    bool       LastEdge); 

  //the usual suspects
//(原文注释:照例的函数)—— Activate/Process/Terminate/Render。
  void Activate();
  int  Process();
  void Terminate();
  void Render();
};




#endif

